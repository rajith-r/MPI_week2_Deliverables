#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <numeric>
#include <mpi.h>
//#include "scatter.h"
//#include "serial_gaussian.h"
//#include "mpi_gaussian.h"
#include "cyclic_gaussian.h"
#include "benchmarks.h"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	//int scatter_result = scatter(argc, argv);
	//int gaussian_result = serial_gaussian_elim();
	//int mpi_gaussian_result = mpi_gaussian_elim(argc, argv);
	//int cyclic_gaussian_result = cyclic_gaussian_elim(argc, argv);

	//task0 does the merge sequentially using std::merge
	//mergesort_task0(argc, argv);

	//task0 gathers tsorted data and merges it using mergepath as well as using using omp for
	//mergesort_mergepath_omp(argc, argv);

	//task0 gathers sorted data and merges it using mergepath without using omp for
	//mergesort_mergepath(argc, argv);

	//distributed mergesort using cyclic distribution of data
	benchmark_merge_sort(argc, argv);
	MPI_Finalize();
    return 0;
}