#include <vector>
#include <algorithm>

#include "../engine/MoveUtils.h"
#include "../engine/Board.h"
#include "../engine/Evaluate.h"
#include "../engine/Enums.h"

const int M = 64;
const int NUM_FEATURES = 40960;
const int K = 32;

bool needs_refresh[2] = { true, true };

struct NNUEAccumulator {
    double v[2][M];

    double* operator[](Color perspective) {
        return v[perspective];
    }
};

std::vector<NNUEAccumulator> accumulatorStk;
NNUEAccumulator currAccumulator;

struct LinearLayer {
    int num_inputs;
    int num_outputs;
    double bias[M];
    double weight[M][M];
} ;
LinearLayer L_0 = {NUM_FEATURES, M}, L_1 = {2 * M, K}, L_2 = {K, 1};

void refresh_accumulator(
    const LinearLayer& layer,
    NNUEAccumulator& new_acc,
    const std::vector<int>& active_features,
    Color perspective
) {
    for (int i = 0; i < M; ++i) {
        new_acc[perspective][i] = layer.bias[i];
    }

    for (int a : active_features) {
        for (int i = 0; i < M; ++i) {
            new_acc[perspective][i] += layer.weight[a][i];
        }
    }
}

void update_accumulator(
    const LinearLayer& layer,
    NNUEAccumulator& new_acc,
    NNUEAccumulator&  prev_acc,
    const std::vector<int>& removed_features,
    const std::vector<int>& added_features,
    Color perspective
) {
    for (int i = 0; i < M; ++i) {
        new_acc[perspective][i] = prev_acc[perspective][i];
    }

    for (int r : removed_features) {
        for (int i = 0; i < M; ++i) {
            new_acc[perspective][i] -= layer.weight[r][i];
        }
    }

    for (int a : added_features) {
        for (int i = 0; i < M; ++i) {
            new_acc[perspective][i] += layer.weight[a][i];
        }
    }
}

double* linear(
    const LinearLayer& layer,
    double* output,
    const double* input
) {
    for (int i = 0; i < layer.num_outputs; ++i) {
        output[i] = layer.bias[i];
    }

    for (int i = 0; i < layer.num_inputs; ++i) {
        for (int j = 0; j < layer.num_outputs; ++j) {
            output[j] += input[i] * layer.weight[i][j];
        }
    }

    return output + layer.num_outputs;
}

double* crelu(
    int size,
    double* output,
    const double* input
) {
    for (int i = 0; i < size; ++i) {
        output[i] = std::min(std::max(input[i], 0.0), 1.0);
    }

    return output + size;
}

std::vector<int> getActiveFeatures(Color perspective) {
    std::vector<int> result;

    const int kingSquare = (perspective == White ? board->whiteKingSquare : board->blackKingSquare);

    for (int sq = 0; sq < 64; ++sq) {
        const int piece = board->squares[sq];
        const int color = (piece & 8) >> 3;
        const int pieceType = piece & 7;

        if (pieceType == Empty) {
            continue;
        }

        result.push_back(get_half_kp_index(perspective, kingSquare, sq, pieceType, (Color)color));
    }

    return result;

}

std::vector<int> getAddedFeatures(Color perspective, int move) {
    std::vector<int> result;

    const int from = MoveUtils::getFromSq(move);
    const int to = MoveUtils::getToSq(move);
    const int piece = MoveUtils::getPiece(move);
    const int promPiece = MoveUtils::getPromotionPiece(move);
    const int kingSquare = (perspective == White ? board->whiteKingSquare : board->blackKingSquare);
    const int color = MoveUtils::getColor(move);

    if (!MoveUtils::isPromotion(move)) {
        result.push_back(get_half_kp_index(perspective, kingSquare, from, promPiece, (Color)color));
    }

    else {
        result.push_back(get_half_kp_index(perspective, kingSquare, to, promPiece, (Color)color));
    }

    if(MoveUtils::isCastle(move)) {
        const int moveFile = (to % 8);
        const int moveRank = (to / 8);
        const int rookFile = (moveFile == 6 ? 5 : 3);
        const int rookSquare = (moveRank * 8) + rookFile;

        result.push_back(get_half_kp_index(perspective, kingSquare, rookSquare, Rook, (Color)color));
    }

    return result;
}

std::vector<int> getRemovedFeatures(Color perspective, int move) {
    std::vector<int> result;

    const int from = MoveUtils::getFromSq(move);
    const int to = MoveUtils::getToSq(move);
    const int piece = MoveUtils::getPiece(move);
    const int promPiece = MoveUtils::getPromotionPiece(move);
    const int kingSquare = (perspective == White ? board->whiteKingSquare : board->blackKingSquare);
    const int color = MoveUtils::getColor(move);

    result.push_back(get_half_kp_index(perspective, kingSquare, from, piece, (Color)color));

    if (MoveUtils::isCapture(move)) {
        const int capturedPieceSquare = (MoveUtils::isEP(move) ? (to + (color == White ? -8 : 8)) : to);
        result.push_back(get_half_kp_index(perspective, kingSquare, capturedPieceSquare, MoveUtils::getCapturedPiece(move), (color == White ? Black : White)));
    }

    if (MoveUtils::isPromotion(move)) {
        result.push_back(get_half_kp_index(perspective, kingSquare, to, Pawn, (Color)color));
    }

    return result;
}

void makeMove(const int& move) {
    NNUEAccumulator& lastAccumulator = accumulatorStk.back();
    accumulatorStk.push_back(currAccumulator);

    for (Color perspective : { White, Black }) {
        if (needs_refresh[(bool)perspective]) { // when king moves (because basically all active features change)
            refresh_accumulator(
                L_0,
                currAccumulator,
                getActiveFeatures(perspective),
                perspective
            );
        } else {
            update_accumulator(
                L_0,
                currAccumulator,
                lastAccumulator,
                getRemovedFeatures(perspective, move),
                getAddedFeatures(perspective, move),
                perspective
            );
        }
    }
}

double nnue_evaluate() {
    double buffer[2*M];

    double input[2*M];
    Color stm = (Color)board->turn;
    for (int i = 0; i < M; ++i) {
        input[i] = currAccumulator[stm][i];
        input[M+i] = currAccumulator[(stm == White ? Black : White)][i];
    }

    double* curr_output = buffer;
    double* curr_input = input;
    double* next_output;

    next_output = crelu(2 * L_0.num_outputs, curr_output, curr_input);
    curr_input = curr_output;
    curr_output = next_output;

    next_output = linear(L_1, curr_output, curr_input);
    curr_input = curr_output;
    curr_output = next_output;

    next_output = crelu(L_1.num_outputs, curr_output, curr_input);
    curr_input = curr_output;
    curr_output = next_output;

    next_output = linear(L_2, curr_output, curr_input);

    return *curr_output;
}
