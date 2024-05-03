// ExampleDLL.h
#pragma once
#ifdef TRAIN_EXPORTS
#define TRAIN_API __declspec(dllexport)
#else
#define TRAIN_API __declspec(dllimport)
#endif

#include "../engine/Enums.h"
#include <string>
#include <vector>

struct TrainingDataEntry {
    std::string board_string;
    int stm;
    int eval_cp;
};

#ifdef __cplusplus
extern "C" {
#endif

struct TRAIN_API SparseBatch {
    int size;
    int num_active_white_features;
    int num_active_black_features;

    float* stm;
    float* score;
    int* white_features_indices;
    int* black_features_indices;

    void fill(const std::vector<TrainingDataEntry>& entries);
};

TRAIN_API struct SparseBatch* CreateSparseBatch(const char* file_name);
TRAIN_API void DeleteSparseBatch(struct SparseBatch* batch);

#ifdef __cplusplus
}
#endif