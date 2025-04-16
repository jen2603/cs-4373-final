/*
 * File: MatrixDet.c
 *
 * Description: Computes the determinant of a matrix both serially
 * and in parallel (using OpenMP) using Gaussian elimination.
 * It outputs the filename, matrix size, actual determinant and
 * log10(|determinant|). It also measures the time for each method
 * so as to compute the speedup and efficiency.
 *
 * How to compile (with OpenMP):
 *   gcc -O2 -fopenmp MatrixDet.c -o determinant -lm
 *
 * Usage:
 *   ./MatrixDet <matrix_filename> <matrix_size> <number_of_threads>
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 #include <omp.h>
 
 #define EPSILON 1e-12
 
 // Function to read a matrix from binary file.
 void readMatrix(const char *fname, double *A, int n) {
     FILE *fp = fopen(fname, "rb");
     if (fp == NULL) {
         fprintf(stderr, "Error opening file %s\n", fname);
         exit(EXIT_FAILURE);
     }
     for (int i = 0; i < n * n; i++) {
         if(fread(&A[i], sizeof(double), 1, fp) != 1) {
             fprintf(stderr, "Error reading matrix from %s\n", fname);
             fclose(fp);
             exit(EXIT_FAILURE);
         }
     }
     fclose(fp);
 }
 
 // Serial Gaussian elimination to compute determinant.
 // Returns the determinant. Also computes log10(|determinant|) using the pivot multiplications.
 double computeDeterminantSerial(double *A, int n, double *logDet) {
     int sign = 1;
     *logDet = 0.0;
 
     // Create a working copy of A so as not to modify original.
     double *B = (double *)malloc(n * n * sizeof(double));
     if (B == NULL) {
         fprintf(stderr, "Memory allocation error!\n");
         exit(EXIT_FAILURE);
     }
     for (int i = 0; i < n * n; i++) {
         B[i] = A[i];
     }
 
     for (int i = 0; i < n; i++) {
         // Pivoting: find nonzero pivot
         int pivotIndex = i;
         while (pivotIndex < n && fabs(B[pivotIndex * n + i]) < EPSILON)
             pivotIndex++;
 
         // If pivot not found, the determinant is zero.
         if (pivotIndex == n) {
             free(B);
             *logDet = -INFINITY;  // log(|0|) is -infinity.
             return 0.0;
         }
 
         // Swap rows if needed.
         if (pivotIndex != i) {
             for (int k = 0; k < n; k++) {
                 double tmp = B[i * n + k];
                 B[i * n + k] = B[pivotIndex * n + k];
                 B[pivotIndex * n + k] = tmp;
             }
             sign = -sign;
         }
 
         // Accumulate log10 of the absolute pivot.
         double pivotVal = B[i * n + i];
         *logDet += log10(fabs(pivotVal));
 
         // Eliminate column i from rows i+1...n-1.
         for (int j = i + 1; j < n; j++) {
             double factor = B[j * n + i] / pivotVal;
             // Perform row j subtraction.
             for (int k = i; k < n; k++) {
                 B[j * n + k] -= factor * B[i * n + k];
             }
         }
     }
 
     // The determinant is the product of the diagonal entries times the sign.
     double det = sign;
     for (int i = 0; i < n; i++) {
         det *= B[i * n + i];
     }
 
     free(B);
     return det;
 }
 
 // Parallel version using OpenMP to parallelize the elimination step.
 double computeDeterminantParallel(double *A, int n, double *logDet, int numThreads) {
     int sign = 1;
     *logDet = 0.0;
 
     double *B = (double *)malloc(n * n * sizeof(double));
     if (B == NULL) {
         fprintf(stderr, "Memory allocation error!\n");
         exit(EXIT_FAILURE);
     }
     for (int i = 0; i < n * n; i++) {
         B[i] = A[i];
     }
 
     omp_set_num_threads(numThreads);
 
     for (int i = 0; i < n; i++) {
         // Pivot selection: find pivot row.
         int pivotIndex = i;
         while (pivotIndex < n && fabs(B[pivotIndex * n + i]) < EPSILON)
             pivotIndex++;
 
         if (pivotIndex == n) {
             free(B);
             *logDet = -INFINITY;  // log(|0|) is -infinity.
             return 0.0;
         }
 
         // Swap rows if needed.
         if (pivotIndex != i) {
             for (int k = 0; k < n; k++) {
                 double tmp = B[i * n + k];
                 B[i * n + k] = B[pivotIndex * n + k];
                 B[pivotIndex * n + k] = tmp;
             }
             sign = -sign;
         }
 
         double pivotVal = B[i * n + i];
         *logDet += log10(fabs(pivotVal));
 
         // Parallel elimination: for each row j = i+1 to n-1 update row j.
         #pragma omp parallel for schedule(static)
         for (int j = i + 1; j < n; j++) {
             double factor = B[j * n + i] / pivotVal;
             for (int k = i; k < n; k++) {
                 B[j * n + k] -= factor * B[i * n + k];
             }
         }
     }
 
     double det = sign;
     for (int i = 0; i < n; i++) {
         det *= B[i * n + i];
     }
 
     free(B);
     return det;
 }
 
 int main(int argc, char *argv[]) {
     if (argc < 4) {
         fprintf(stderr, "Usage: %s <matrix_filename> <matrix_size> <number_of_threads>\n", argv[0]);
         exit(EXIT_FAILURE);
     }
 
     const char *filename = argv[1];
     int n = atoi(argv[2]);
     int numThreads = atoi(argv[3]);
 
     // Allocate memory for the matrix.
     double *A = (double *)malloc(n * n * sizeof(double));
     if (A == NULL) {
         fprintf(stderr, "Memory allocation error!\n");
         exit(EXIT_FAILURE);
     }
 
     // Reading the matrix file.
     printf("1: %s\n", filename);
     printf("2: %dx%d\n", n, n);
     readMatrix(filename, A, n);
 
     double logDetSerial, logDetParallel;
     double detSerial, detParallel;
     double start, end;
     double tSerial, tParallel;
 
     // Serial run.
     start = omp_get_wtime();
     detSerial = computeDeterminantSerial(A, n, &logDetSerial);
     end = omp_get_wtime();
     tSerial = end - start;
 
     // Parallel run.
     start = omp_get_wtime();
     detParallel = computeDeterminantParallel(A, n, &logDetParallel, numThreads);
     end = omp_get_wtime();
     tParallel = end - start;
 
     // Reporting results: note that the sample output shows scientific notation.
     printf("3: det = %.6e\n", detSerial);
     printf("4: log(abs(det)) = %.6e\n\n", logDetSerial);
 
     // Optionally report timing, speedup, and efficiency.
     double speedup = tSerial / tParallel;
     double efficiency = speedup / numThreads;
     printf("Serial execution time   : %f seconds\n", tSerial);
     printf("Parallel execution time : %f seconds with %d threads\n", tParallel, numThreads);
     printf("Speedup                 : %f\n", speedup);
     printf("Efficiency              : %f\n", efficiency);
 
     // Clean up.
     free(A);
 
     return 0;
 }