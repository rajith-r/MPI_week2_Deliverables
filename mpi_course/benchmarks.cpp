#include "merge_sort.h"
#include <mpi.h>
#include "benchmarks.h"
#include <random>
#include <iostream>
#include <algorithm>

// helper function to fill random data
void generate_random_data(float* input_ptr, int dim, std::mt19937& gen, std::uniform_int_distribution<>& dis) {
    for (int i = 0; i < dim; ++i) {
        input_ptr[i] = dis(gen);
    }
}

void benchmark_merge_sort(int argc, char* argv[]) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    int rank, num_tasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    for (int dim = 1 << 2; dim <= 1 << 15; dim *= 2) {

        std::unique_ptr<float[]> input_ptr = std::make_unique<float[]>(dim);
        //// ---------- mergesort_task0 ----------
        generate_random_data(input_ptr.get(), dim, gen, dis);

        double start_time = MPI_Wtime();
        if (dim < num_tasks) {
            if (rank == 0) std::sort(input_ptr.get(), input_ptr.get() + dim);
        }
        else {
            mergesort_task0(argc, argv, dim, input_ptr.get());
        }
        double end_time = MPI_Wtime();
        double elapsed_ms = (end_time - start_time) * 1000.0;

        if (rank == 0) {
            printf("Dimension: %d, Time taken (task0): %.3f ms\n", dim, elapsed_ms);
            std::cout << "task0 sorted: " << (std::is_sorted(input_ptr.get(), input_ptr.get() + dim) ? "YES" : "NO") << "\n";
        }

        //// ---------- mergesort_mergepath ----------
        generate_random_data(input_ptr.get(), dim, gen, dis);

        start_time = MPI_Wtime();
        if (dim < num_tasks) {
            if (rank == 0) std::sort(input_ptr.get(), input_ptr.get() + dim);
        }
        else {
            mergesort_mergepath(argc, argv, dim, input_ptr.get());
        }
        end_time = MPI_Wtime();
        elapsed_ms = (end_time - start_time) * 1000.0;

        if (rank == 0) {
            printf("Dimension: %d, Time taken (mergepath): %.3f ms\n", dim, elapsed_ms);
            std::cout << "mergepath sorted: " << (std::is_sorted(input_ptr.get(), input_ptr.get() + dim) ? "YES" : "NO") << "\n";
        }

        //// ---------- mergesort_mergepath_omp ----------
        generate_random_data(input_ptr.get(), dim, gen, dis);

        start_time = MPI_Wtime();
        if (dim < num_tasks) {
            if (rank == 0) std::sort(input_ptr.get(), input_ptr.get() + dim);
        }
        else {
            mergesort_mergepath_omp(argc, argv, dim, input_ptr.get());
        }
        end_time = MPI_Wtime();
        elapsed_ms = (end_time - start_time) * 1000.0;

        if (rank == 0) {
            printf("Dimension: %d, Time taken (OMP): %.3f ms\n", dim, elapsed_ms);
            std::cout << "OMP sorted: " << (std::is_sorted(input_ptr.get(), input_ptr.get() + dim) ? "YES" : "NO") << "\n";
        }

        //// ---------- merge_sort_distributed ----------
        generate_random_data(input_ptr.get(), dim, gen, dis);

        if (dim < num_tasks) {
            if (rank == 0) {
                start_time = MPI_Wtime();
                std::sort(input_ptr.get(), input_ptr.get() + dim);
                end_time = MPI_Wtime();
                elapsed_ms = (end_time - start_time) * 1000.0;

                printf("Dimension: %d, Time taken (distributed fallback): %.3f ms\n", dim, elapsed_ms);
                std::cout << "distributed fallback sorted: " << (std::is_sorted(input_ptr.get(), input_ptr.get() + dim) ? "YES" : "NO") << "\n";
            }
        }
        else if (dim % num_tasks == 0 && (num_tasks & (num_tasks - 1)) == 0) {
            start_time = MPI_Wtime();
            merge_sort_distributed(argc, argv, dim, input_ptr.get());
            end_time = MPI_Wtime();
            elapsed_ms = (end_time - start_time) * 1000.0;

            if (rank == 0) {
                printf("Dimension: %d, Time taken (distributed): %.3f ms\n", dim, elapsed_ms);
                std::cout << "distributed sorted: " << (std::is_sorted(input_ptr.get(), input_ptr.get() + dim) ? "YES" : "NO") << "\n";
            }
        }
        else {
            if (rank == 0) {
                printf("Dimension: %d skipped for distributed: incompatible dim/num_tasks\n", dim);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }
}
