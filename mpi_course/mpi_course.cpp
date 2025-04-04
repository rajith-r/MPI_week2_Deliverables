// An example of sum reduction using MPI
// By: Nick from CoffeeBeforeArch

#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <numeric>
#include <mpi.h>
#include "scatter.h"
#include "serial_gaussian.h"
#include "mpi_gaussian.h"

int main(int argc, char* argv[]) {
	//int scatter_result = scatter(argc, argv);
	//int gaussian_result = serial_gaussian_elim();
	int mpi_gaussian_result = mpi_gaussian_elim(argc, argv);
    return 0;
}