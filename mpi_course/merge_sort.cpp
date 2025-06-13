#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <numeric>
#include "mpi.h"
#include <omp.h>
#include "merge_sort.h"
void mergesort_task0(int argc, char* argv[], int dim, float* input_ptr) {

	int num_tasks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

	int task_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	int chunk_size = dim / num_tasks;

	// Allocate memory for receiving data
	std::unique_ptr<float[]> recv_ptr(new float[chunk_size]);

	//scatter the input data to all tasks
	MPI_Scatter(input_ptr, chunk_size, MPI_FLOAT, recv_ptr.get(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	std::sort(recv_ptr.get(), recv_ptr.get() + chunk_size);
	// Gather sorted chunks back to the root task
	MPI_Gather(recv_ptr.get(), chunk_size, MPI_FLOAT, input_ptr, chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

	//merge the sorted chunks
	if (task_id == 0) {
		std::vector<float> merged(input_ptr, input_ptr + dim);
		int current_size = chunk_size;

		for (int step = 1; step < num_tasks; step *= 2) {
			for (int i = 0; i + step < num_tasks; i += 2 * step) {
				int left = i * chunk_size;
				int mid = (i + step) * chunk_size;
				int right = std::min((i + 2 * step) * chunk_size, dim);

				std::inplace_merge(merged.begin() + left,
					merged.begin() + mid,
					merged.begin() + right);
			}
			current_size *= 2;
		}
		// Copy the sorted data back to the input pointer
		std::copy(merged.begin(), merged.end(), input_ptr);
	}
}

void merge_path(int argc, char* argv[], std::vector<float>& arr, int left, int mid, int right) {
	std::vector<float> temp(right - left);
	int i = left, j = mid, k = 0;
	while (i < mid && j < right) {
		if (arr[i] <= arr[j]) {
			temp[k++] = arr[i++];
		}
		else {
			temp[k++] = arr[j++];
		}
	}
	while (i < mid) {
		temp[k++] = arr[i++];
	}
	while (j < right) {
		temp[k++] = arr[j++];
	}
	for (i = left, k = 0; i < right; ++i, ++k) {
		arr[i] = temp[k];
	}
}

void mergesort_mergepath(int argc, char* argv[], int dim, float* input_ptr) {
	int num_tasks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

	int task_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	int chunk_size = dim / num_tasks;

	// Allocate memory for receiving data
	std::unique_ptr<float[]> recv_ptr(new float[chunk_size]);

	//scatter the input data to all tasks
	MPI_Scatter(input_ptr, chunk_size, MPI_FLOAT, recv_ptr.get(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	std::sort(recv_ptr.get(), recv_ptr.get() + chunk_size);


	// Gather sorted chunks back to the root task
	MPI_Gather(recv_ptr.get(), chunk_size, MPI_FLOAT, input_ptr, chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

	//merge the sorted chunks
	if (task_id == 0) {
		std::vector<float> merged(input_ptr, input_ptr + dim);
		int total_tasks = chunk_size;

		for (int step = 1; step < num_tasks; step *= 2) {
			// Each task will merge pairs of chunks
			int task_no = 0;
			for (int i = 0; i + step < num_tasks; i += 2 * step) {
				int left = i * chunk_size;
				int mid = (i + step) * chunk_size;
				int right = std::min((i + 2 * step) * chunk_size, dim);
				merge_path(argc, argv, merged, left, mid, right);
			}
		}
		// Copy the sorted data back to the input pointer
		std::copy(merged.begin(), merged.end(), input_ptr);
	}
}

void mergesort_mergepath_omp(int argc, char* argv[], int dim, float* input_ptr) {

	int num_tasks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

	int task_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	int chunk_size = dim / num_tasks;

	// Allocate memory for receiving data
	std::unique_ptr<float[]> recv_ptr(new float[chunk_size]);

	//scatter the input data to all tasks
	MPI_Scatter(input_ptr, chunk_size, MPI_FLOAT, recv_ptr.get(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	std::sort(recv_ptr.get(), recv_ptr.get() + chunk_size);

	// Gather sorted chunks back to the root task
	MPI_Gather(recv_ptr.get(), chunk_size, MPI_FLOAT, input_ptr, chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);

	//merge the sorted chunks
	if (task_id == 0) {
		std::vector<float> merged(input_ptr, input_ptr + dim);
		int total_tasks = chunk_size;

		for (int step = 1; step < num_tasks; step *= 2) {
			// Each task will merge pairs of chunks
			int task_no = 0;
#pragma omp parallel for
			for (int i = 0; i + step < num_tasks; i += 2 * step) {
				int left = i * chunk_size;
				int mid = (i + step) * chunk_size;
				int right = std::min((i + 2 * step) * chunk_size, dim);
				merge_path(argc, argv, merged, left, mid, right);
			}
		}
		// Copy the sorted data back to the input pointer
		std::copy(merged.begin(), merged.end(), input_ptr);
	}
}


//distributed merge sort using MPI

//normal merge

//sort individula chunks

//merge chunks
//taskid:    0       2     4       6
//step 1: (0, 1), (2, 3), (4, 5), (6, 7)
//step 2: (0, 3), (4, 7)
//step 3: (0, 7)
void merge_sort_distributed(int argc, char* argv[], int dim, float* input_ptr) {
	//MPI_Init(&argc, &argv);
	int num_tasks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
	int task_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
	int chunk_size = dim / num_tasks;

	if (dim % num_tasks != 0 || (num_tasks & (num_tasks - 1)) != 0) {
		std::cerr << "Error: Dimension must be divisible by number of tasks, and number of tasks must be power of 2." << std::endl;
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	// Allocate memory for receiving data
	std::unique_ptr<float[]> recv_ptr(new float[chunk_size]);

	MPI_Scatter(input_ptr, chunk_size, MPI_FLOAT, recv_ptr.get(), chunk_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
	std::sort(recv_ptr.get(), recv_ptr.get() + chunk_size);

	// Lets distrbute merge using MPI
	//merge chunks
	//taskid:    0       2     4       6
	//step 1: (0, 1), (2, 3), (4, 5), (6, 7)
	//step 2: (0, 2), (4, 6)
	//step 3: (0, 7)
	//std::cout << "disrtibuted merge using MPI" << std::endl;
	int current_size = chunk_size;
	int step = 1;
	while (step < num_tasks) {
		if (task_id % (2 * step) == 0) {
			std::unique_ptr<float[]> partner(new float[current_size]);
			MPI_Recv(partner.get(), current_size, MPI_FLOAT, task_id + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::unique_ptr<float[]> merged(new float[current_size * 2]);

			int i = 0, j = 0, k = 0;
			while (i < current_size && j < current_size) {
				if (recv_ptr[i] <= partner[j]) merged[k++] = recv_ptr[i++];
				else merged[k++] = partner[j++];
			}
			while (i < current_size) merged[k++] = recv_ptr[i++];
			while (j < current_size) merged[k++] = partner[j++];
			recv_ptr = std::move(merged);
			current_size *= 2;
		}
		else {
			if (task_id - step >= 0) {
				MPI_Send(recv_ptr.get(), current_size, MPI_FLOAT, task_id - step, 0, MPI_COMM_WORLD);
			}
		}
		step *= 2;
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//print sorted data only on root task
	if (task_id == 0) {
		std::copy(recv_ptr.get(), recv_ptr.get() + current_size, input_ptr);
	}
	//MPI_Finalize();
}

