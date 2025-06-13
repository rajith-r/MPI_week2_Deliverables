#include <algorithm>
#include <iostream>
#include <memory>
#include <random>
#include <numeric>
#include "mpi.h"

void print_matrix(const float* matrix, int dim) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            std::cout << matrix[i * dim + j] << ' ';
        }
        std::cout << '\n';
    }
}

int  cyclic_gaussian_elim(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

	int num_tasks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

	int task_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
	
	int dim = 1 << 4;
	int nrows = dim / num_tasks;

	std::unique_ptr<float[]> input_ptr;

	// Create buffer for receive
	int m_chunk_size = nrows * dim;
	std::unique_ptr<float[]> m_chunk = std::make_unique<float[]>(m_chunk_size);

	// Pivot row receive after each iteration
	std::unique_ptr<float[]> pivot_row = std::make_unique<float[]>(dim);

	if (task_id == 0) {
		input_ptr = std::make_unique<float[]>(dim * dim);
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(1.0f, 2.0f);
		// Create random data
		std::generate(input_ptr.get(), input_ptr.get() + dim * dim,
			[&] { return dist(mt); });
		std::cout << "Initial Matrix:\n";
		print_matrix(input_ptr.get(), dim);

		// Send the matrix to all processes
		for (int i = 0; i < nrows; i++)
		{
			MPI_Scatter(input_ptr.get() + i * dim * num_tasks, dim, MPI_FLOAT,
				m_chunk.get() + i * dim, dim, MPI_FLOAT, 0, MPI_COMM_WORLD);
		}
	}

	//perform Gaussian elimination
	for (int row = 0; row < nrows; row++) {
		for (int rank = 0; rank < num_tasks; rank++) {
			int global_col = row * num_tasks + rank;
			if (task_id == rank) {
				// pivot column
				float pivot_value = m_chunk[row * dim + global_col];

				// Normalize pivot row
				for (int col = global_col; col < dim; col++) {
					m_chunk[row*dim+col] /= pivot_value;
				}

				// Send pivot row to all processes
				MPI_Bcast(m_chunk.get() + row * dim, dim, MPI_FLOAT, rank, MPI_COMM_WORLD);

				// eliminate rows in the current process
				for (int elim_row = row + 1; elim_row < nrows; elim_row++) {
					float scale = m_chunk[elim_row * dim + global_col];
					for (int col = global_col; col < dim; col++) {
						m_chunk[elim_row * dim + col] -= scale * m_chunk[row * dim + col];
					}
				}
				
			}
			else {
				MPI_Bcast(pivot_row.get(), dim, MPI_FLOAT, rank, MPI_COMM_WORLD);
				int global_pivot_row = row * num_tasks + rank;

				for (int elim_row = 0; elim_row < nrows; elim_row++) {
					int global_elim_row = elim_row * num_tasks + task_id;
					
					if (global_elim_row > global_pivot_row) {
						float scale = m_chunk[elim_row * dim + global_col];
						for (int col = global_col; col < dim; col++) {
							m_chunk[elim_row * dim + col] -= scale * pivot_row[col];
						}
					}
				}
			}
	   }
	}

	// Gather the results
	for (int i = 0; i < nrows; i++) {
		MPI_Gather(m_chunk.get() + i * dim, dim, MPI_FLOAT,
			input_ptr.get() + i * dim * num_tasks, dim, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}
	if (task_id == 0) {
		// Print the final matrix
		std::cout << "Final Matrix:\n";
		print_matrix(input_ptr.get(), dim);
	}

    MPI_Finalize();
    return 0;
}