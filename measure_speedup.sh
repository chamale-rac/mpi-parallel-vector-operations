#!/bin/bash

# Number of tests to run
num_tests=10

# Vector size
vector_size=300000000
mpi_vector_size=300000000

# Files for sequential and parallel programs
seq_program="./vector_add"
par_program="mpiexec -n 4 ./mpi_vector_add"

# Arrays to store times
seq_times=()
par_times=()

# Run the sequential program
echo "Running sequential program ($seq_program)..."
for ((i = 1; i <= num_tests; i++)); do
    output=$($seq_program $vector_size)
    time=$(echo "$output" | grep "Vector addition took" | awk '{print $4}')
    seq_times+=($time)
    echo "Sequential run $i: $time seconds"
done

# Run the parallel program
echo "Running parallel program ($par_program)..."
for ((i = 1; i <= num_tests; i++)); do
    output=$($par_program $mpi_vector_size)
    time=$(echo "$output" | grep "Vector addition took" | awk '{print $4}')
    par_times+=($time)
    echo "Parallel run $i: $time seconds"
done

# Calculate the average time for both programs
sum_seq=0
sum_par=0

for time in "${seq_times[@]}"; do
    sum_seq=$(echo "$sum_seq + $time" | bc)
done
avg_seq=$(echo "scale=6; $sum_seq / $num_tests" | bc)

for time in "${par_times[@]}"; do
    sum_par=$(echo "$sum_par + $time" | bc)
done
avg_par=$(echo "scale=6; $sum_par / $num_tests" | bc)

# Calculate speedup
speedup=$(echo "scale=6; $avg_seq / $avg_par" | bc)

# Print results in a table
echo ""
echo "---------------------------------------------"
echo "|  Program      | Average Time (seconds)    |"
echo "---------------------------------------------"
printf "| Sequential    | %-25s |\n" $avg_seq
printf "| Parallel (4)  | %-25s |\n" $avg_par
echo "---------------------------------------------"
echo ""
echo "Speedup: $speedup"
