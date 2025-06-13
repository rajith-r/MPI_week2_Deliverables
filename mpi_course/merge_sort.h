#pragma once

#include <vector>

// Distributed merge sort variants
void mergesort_task0(int argc, char* argv[], int dim, float* input_ptr);
void mergesort_mergepath(int argc, char* argv[], int dim, float* input_ptr);
void mergesort_mergepath_omp(int argc, char* argv[], int dim, float* input_ptr);
void merge_sort_distributed(int argc, char* argv[], int dim, float* input_ptr);

// Merge functions
void merge_path(int argc, char* argv[], std::vector<float>& arr, int left, int mid, int right);
