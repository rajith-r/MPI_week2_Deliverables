#include <iostream>
#include <mpi.h>
#include <random>
   
void print_matrix(const float* A, int dim) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            std::cout << A[i * dim + j] << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "---------------------------\n";
}

int mpi_gaussian_elim(int argc, char* argv[]) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get the total number of tasks
    int num_tasks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
	std::cout << "Number of tasks: " << num_tasks << std::endl;
    // Calculate chunk size
    // Assume this divides evenly
    const int dim = 1 << 4;
    //const int dim = static_cast<int>(std::sqrt(num_elements)); // 32
    int rows_per_proc = dim / num_tasks;
	std::cout << "Rows per process: " << rows_per_proc << std::endl;

    // Get the task ID
    int task_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	int start_row = task_id * rows_per_proc;
	int end_row = start_row + rows_per_proc - 1;
	std::cout << "Rank " << task_id << " processing rows " << start_row << " to " << end_row << "\n";
    // Create buffer for send (only initialized in rank 0)
    std::unique_ptr<float[]> input_ptr;


	// Create buffer for receive
	int m_chunk_size = rows_per_proc*dim;

	// Receive chunk
    std::unique_ptr<float[]> m_chunk = std::make_unique<float[]>(m_chunk_size);

    //Pivot row receive after each iteration
	std::unique_ptr<float[]> pivot_row = std::make_unique<float[]>(dim);

	//Request for non-blocking send
    std::vector<MPI_Request> requests(num_tasks);
    // Generate random numbers from rank 0
    if (task_id == 0) {
        // Allocate memory for send buffer
        input_ptr = std::make_unique<float[]>(dim*dim);

        // Create random number generator
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution dist(1.0f, 2.0f);

        // Create random data
        std::generate(input_ptr.get(), input_ptr.get() + dim*dim,
            [&] { return dist(mt); });
		//std::cout << "Initial Matrix:\n";
        //print_matrix(input_ptr.get(), dim);
    }

    // Before doing anything, send parts of the matrix to each process
    MPI_Scatter(input_ptr.get(), dim* rows_per_proc, MPI_FLOAT, m_chunk.get(),
        dim * rows_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

    for (int row = 0; row <= end_row; row++) {
		int mapped_rank = row / rows_per_proc;
		int local_row = row % rows_per_proc;
		if (task_id == mapped_rank) {
			// Normalize pivot row
			float pivot_value = m_chunk[local_row * dim + row];
			for (int col = 0; col < dim; col++) {
				m_chunk[local_row * dim + col] /= pivot_value;
			}
			// Send pivot row to all processes
			for (int i = mapped_rank + 1; i < num_tasks; i++) {
                //std::cout << "[Rank " << task_id << "] Sending pivot row for row " << row << "\n";
				MPI_Isend(m_chunk.get() + local_row * dim, dim, MPI_FLOAT, i, 0,
					MPI_COMM_WORLD, &requests[i]);
			}
			//eliminate rows in the current process
			for (int elim_row = local_row + 1; elim_row < rows_per_proc; elim_row++) {
				float scale = m_chunk[elim_row * dim + row];
				for (int col = row; col < dim; col++) {
					m_chunk[elim_row * dim + col] -= scale * m_chunk[local_row * dim + col];
				}
			}
            
			// Wait for all requests to finish
			for (int i = mapped_rank + 1; i < num_tasks; i++) {
				MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
			}

        }
        else {
            // Receive pivot row
            //std::cout << "[Rank " << task_id << "] Waiting to receive pivot row for row " << row << "\n";
            MPI_Recv(pivot_row.get(), dim, MPI_FLOAT, mapped_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //eliminate rows in the current process
            for (int elim_row = 0; elim_row < rows_per_proc; elim_row++) {
                float scale = m_chunk[elim_row * dim + row];
                for (int col = row; col < dim; col++) {
                    m_chunk[elim_row * dim + col] -= scale * pivot_row[col];
                }
            }
        }
    }
	// Gather the results
	MPI_Gather(m_chunk.get(), rows_per_proc *dim, MPI_FLOAT, input_ptr.get(),
        rows_per_proc * dim, MPI_FLOAT, 0, MPI_COMM_WORLD);

	if (task_id == 0) {
		// Print the final matrix
		std::cout << "Final Matrix:\n";
		print_matrix(input_ptr.get(), dim);
	}
    // Finish our MPI work
    MPI_Finalize();
    return 0;
}