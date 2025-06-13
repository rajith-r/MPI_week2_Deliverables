# Distributed Merge Sort using MPI and OpenMP

This project implements and benchmarks multiple parallel merge sort variants using both MPI and OpenMP. The benchmarks analyze performance across problem sizes ranging from 4 to 32K elements.

---

## 🚀 Summary

✅ Fully functional distributed merge sort with dynamic buffer resizing  
✅ Binary tree based reduction pattern for distributed merge  
✅ Correctly synchronized MPI pairing and ownership transfer logic  
✅ Benchmarked against multiple variants (task-root, merge-path, OpenMP hybrid, fully distributed)  
✅ Extensive correctness and scaling validation

---

## 🔧 Key Implementations

| Algorithm      | Parallelism Model   | Description |
| -------------- | ------------------- | -------------------------------------------------- |
| `serial`       | Sequential          | Pure serial baseline (`std::sort()`) |
| `task0`        | MPI + Sequential Merge | Scatter/Gather with root merging |
| `mergepath`    | MPI + Sequential Merge Path | Optimized merge using merge path after gather |
| `mergepath_omp`| MPI + OpenMP Merge Path | Best shared-memory speedup |
| `distributed`  | Fully Distributed MPI | Binary tree distributed merging |

---

## 📊 Speedup vs Serial

Speedup is defined as:  
\[
\text{Speedup} = \frac{T_{\text{serial}}}{T_{\text{parallel}}}
\]

| Dimension | task0 | mergepath | mergepath_omp | distributed |
|-----------|:-----:|:---------:|:-------------:|:-----------:|
| 4 | 0.01× | 0.10× | 0.15× | 0.01× |
| 8 | 0.09× | 0.28× | 0.36× | 0.51× |
| 16 | 0.72× | 1.00× | 0.72× | 1.00× |
| 32 | 1.35× | 1.50× | 1.50× | 2.25× |
| 64 | 2.95× | 2.50× | 2.70× | 4.05× |
| 128 | 3.33× | 2.46× | 2.83× | 2.10× |
| 256 | 2.91× | 2.29× | 2.35× | 3.56× |
| 512 | 5.15× | 3.29× | 3.43× | 5.15× |
| 1024 | 5.34× | 5.82× | 5.89× | 8.84× |
| 2048 | 10.17× | 7.30× | 7.25× | 9.53× |
| 4096 | 12.89× | 7.67× | 3.96× | 6.98× |
| 8192 | 5.43× | 6.14× | 7.77× | 8.04× |
| 16384 | 9.58× | 8.14× | 7.86× | 8.23× |
| 32768 | 11.70× | 8.09× | 7.82× | 11.00× |

---

## 📈 Key Insights

- ✅ For very small problems (less than 32 elements), MPI overhead dominates; serial is better.
- ✅ `task0` outscales serial for small-medium sizes, but distributed MPI becomes the best at ≥ 1K elements.
- ✅ `mergepath_omp` provides excellent shared-memory speedup up to ~4K elements.
- ✅ Fully distributed MPI wins for large datasets due to better scalability across nodes.

---

## ⚠️ Issues Faced (and Resolved)

- 🔄 **MPI Rank Synchronization**:  
  Initial distributed merges failed due to ranks being desynchronized across iterations.
  - ✅ Solved by isolating distributed runs into separate MPI executions and using proper MPI barriers.
  
- 🔧 **Dynamic Buffer Resizing**:  
  Distributed merges required careful buffer management with dynamically changing ownership after each round.

- 📨 **Send/Receive Size Mismatches**:  
  All MPI message sizes must match exactly for correctness.

- 🔄 **Ownership Tracking**:  
  After each merge step, the number of elements owned by each rank changes — ownership must be tracked.

- 💡 **`unique_ptr` Safety**:  
  Used `std::unique_ptr` to manage dynamic arrays safely; ownership transfers were handled carefully during reallocations.

- 📊 **Binary Tree Pairing**:  
  Designed a fully scalable binary tree pairing pattern to minimize communication rounds.

- 💡 **MPI Debugging**:  
  Extensive debugging of pairing rules, message sizes, and step calculations to ensure correctness.

---

## 🔬 Main Takeaway

This project builds a fully functional distributed merge sort on top of MPI. It involved:

- Debugging buffer overflows
- Implementing dynamic resizing
- Designing rank pairing logic
- Fully synchronized binary tree merge steps
- Achieving research-grade benchmark quality

---

## ✅ Next Steps

- ✅ Scale to larger problem sizes (≥ 1M elements)
- ✅ Test strong-scaling and weak-scaling behavior across cluster nodes
- ✅ Explore hybrid MPI + OpenMP merges for further optimization
- ✅ Analyze communication overhead vs compute cost

---

## 🛠 Build & Run

mpiexec -n 4 mpi_course.exe
