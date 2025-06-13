import subprocess
import pandas as pd
import re
import matplotlib.pyplot as plt

# CONFIGURABLE: paths and settings
mpi_binary = r"C:\Users\rajit\source\repos\mpi_course\x64\Debug\mpi_course.exe"
process_counts = [1, 2, 4, 8, 16]  # You can modify this easily
dimensions = [1 << i for i in range(2, 16)]  # 4 -> 32768

# Regex patterns to extract output lines
pattern = re.compile(r"Dimension: (\d+), Time taken \((.*?)\): ([\d\.]+) ms")

# Master dataframe to collect all results
results = []

# Main loop: for each process count
for nprocs in process_counts:
    print(f"\n--- Running with {nprocs} processes ---")

    # Launch MPI program
    completed = subprocess.run(
        ["mpiexec", "-n", str(nprocs), mpi_binary],
        capture_output=True,
        text=True
    )

    output = completed.stdout

    # Parse output
    for line in output.splitlines():
        match = pattern.search(line)
        if match:
            dim = int(match.group(1))
            algo = match.group(2)
            time_ms = float(match.group(3))
            results.append({
                "Processes": nprocs,
                "Dimension": dim,
                "Algorithm": algo,
                "Time_ms": time_ms
            })

# Convert to dataframe
df = pd.DataFrame(results)

# Save to Excel
df.to_excel("mpi_benchmark_results.xlsx", index=False)
print("\n✅ Benchmark results saved to mpi_benchmark_results.xlsx")

# Plotting
for algo in df["Algorithm"].unique():
    plt.figure(figsize=(8,5))
    for nprocs in process_counts:
        subdf = df[(df["Algorithm"] == algo) & (df["Processes"] == nprocs)]
        plt.plot(subdf["Dimension"], subdf["Time_ms"], label=f"{nprocs} procs", marker="o")

    plt.title(f"{algo} Runtime Scaling")
    plt.xlabel("Dimension")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid()
    plt.xscale("log", base=2)
    plt.yscale("log", base=10)
    plt.savefig(f"{algo}_scaling.png")
    print(f"✅ Plot saved: {algo}_scaling.png")
