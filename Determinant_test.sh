#!/bin/bash

# Redirect all output to a results file
exec > d_results.txt 2>&1

# Compile the C program
echo "Compiling MatrixDet.c..."
gcc -O2 -fopenmp MatrixDet.c -o determinant -lm
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# List of matrix files to test
MATRIX_FILES=("Matrices/m0016x0016.bin" "Matrices/m0032x0032.bin" "Matrices/m0064x0064.bin" "Matrices/m0128x0128.bin" "Matrices/m0256x0256.bin" "Matrices/m0496x0496.bin" "Matrices/m0512x0512.bin" "Matrices/m1000x1000.bin" "Matrices/m1024x1024.bin" "Matrices/m2000x2000.bin" "Matrices/m2048x2048.bin" "Matrices/m3000x3000.bin" "Matrices/m4000x4000.bin" "Matrices/m4096x4096.bin")
THREAD_COUNTS=(1 2 4 6 8 12 16)

# Loop over each matrix file
for FILE in "${MATRIX_FILES[@]}"; do
    # Extract matrix size from filename (expects format mXXXXxXXXX.bin)
    BASENAME=$(basename "$FILE")
    SIZE=$(echo "$BASENAME" | sed -E 's/^m0*([0-9]+)x0*\1\.bin$/\1/')
    if [ -z "$SIZE" ]; then
        echo "Could not parse matrix size from filename: $FILE"
        continue
    fi

    echo "======================================="
    echo "Testing $FILE (size $SIZE x $SIZE)"
    echo "======================================="

    for THREADS in "${THREAD_COUNTS[@]}"; do
        echo "--- Running with $THREADS thread(s) ---"
        ./determinant "$FILE" "$SIZE" "$THREADS"
        echo
    done
done