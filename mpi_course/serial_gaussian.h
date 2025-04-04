#include <iostream>

void print_matrix2(const float* A, int dim) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            std::cout << A[i * dim + j] << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "---------------------------\n";
}

int serial_gaussian_elim() {
    const int dim = 3;
    float A[dim * dim] = {
        2, 1, -1,
        -3, -1, 2,
        -2, 1, 2
    };

    std::cout << "Initial Matrix:\n";
    print_matrix2(A, dim);

    for (int pivot = 0; pivot < dim; pivot++) {
        float pivot_value = A[pivot * dim + pivot];

        // Normalize pivot row
        for (int col = pivot; col < dim; col++) {
            A[pivot * dim + col] /= pivot_value;
        }

        // Eliminate rows below
        for (int row = pivot + 1; row < dim; row++) {
            float scale = A[row * dim + pivot];
            for (int col = pivot; col < dim; col++) {
                A[row * dim + col] -= scale * A[pivot * dim + col];
            }
        }

        std::cout << "After step " << pivot + 1 << ":\n";
        print_matrix2(A, dim);
    }

    return 0;
}
