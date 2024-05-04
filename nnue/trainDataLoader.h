#pragma once
#ifdef TRAINDATALOADER_EXPORTS
#define TRAINDATALOADER_API __declspec(dllexport)
#else
#define TRAINDATALOADER_API __declspec(dllimport)
#endif

#include "../engine/Enums.h"
#include <string>
#include <vector>

struct TrainingDataEntry {
    std::string board_string;
    int stm;
    int eval_cp;
    float result;
};

#ifdef __cplusplus
extern "C" {
#endif

struct TRAINDATALOADER_API SparseBatch {
    int size;
    int num_active_white_features;
    int num_active_black_features;

    float* stm;
    float* score;
    float* result;
    int* white_features_indices;
    int* black_features_indices;

    void fill(const std::vector<TrainingDataEntry>& entries);
};

TRAINDATALOADER_API struct SparseBatch** CreateSparseBatchArr(const char* file_name, int batch_size, int* arraySize);
TRAINDATALOADER_API void DeleteSparseBatchArr(struct SparseBatch** arr, int arrSize);

#ifdef __cplusplus
}
#endif