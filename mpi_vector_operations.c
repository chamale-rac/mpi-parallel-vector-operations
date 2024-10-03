/* File:     mpi_vector_operations.c
 *
 * Purpose:  Implement parallel vector operations:
 *           1) Compute the dot product of two vectors.
 *           2) Multiply each vector by a scalar (the same scalar for both).
 *
 * Compile:  mpicc -g -Wall -o mpi_vector_operations mpi_vector_operations.c
 * Run:      mpiexec -n <comm_sz> ./mpi_vector_operations <order of the vectors> <scalar>
 *
 * Input:    The order of the vectors, n, and the scalar s
 * Output:   The dot product of the two vectors and the vectors after scalar multiplication
 *
 * Notes:
 * 1.  The order of the vectors, n, should be evenly divisible by comm_sz
 * 2.  This program uses MPI_Scatter and MPI_Gather for distributing and collecting vectors
 * 3.  It also uses MPI_Reduce to compute the global dot product
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void Check_for_error(int local_ok, char fname[], char message[],
      MPI_Comm comm);
void Allocate_vectors(double** local_x_pp, double** local_y_pp,
      int local_n, MPI_Comm comm);
void Print_vector(double local_b[], int local_n, int n, char title[],
      int my_rank, MPI_Comm comm);
void Generate_vector(double local_a[], int local_n, int my_rank, int i_seed);
double Parallel_dot_product(double local_x[], double local_y[], int local_n);
void Parallel_scalar_multiplication(double local_a[], int local_n, double scalar);

int main(int argc, char* argv[]) {
    int n, local_n;
    int comm_sz, my_rank;
    double *local_x, *local_y;
    double s;
    MPI_Comm comm;
    double start, end;
    double local_dot, global_dot;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &comm_sz);
    MPI_Comm_rank(comm, &my_rank);

    // Check if the user provided the vector size and scalar as arguments
    if (argc != 3) {
        if (my_rank == 0) {
            fprintf(stderr, "Usage: %s <order of the vectors> <scalar>\n", argv[0]);
        }
        MPI_Finalize();
        exit(-1);
    }

    // Receive n and scalar s as execution parameters
    n = atoi(argv[1]);
    s = atof(argv[2]);
    if (n <= 0 || n % comm_sz != 0) {
        if (my_rank == 0) {
            fprintf(stderr, "Order of the vectors should be a positive integer and evenly divisible by the number of processes\n");
        }
        MPI_Finalize();
        exit(-1);
    }

    local_n = n / comm_sz;

    // Allocate memory for vectors
    Allocate_vectors(&local_x, &local_y, local_n, comm);

    // Generate random vectors
    Generate_vector(local_x, local_n, my_rank, 1);
    Generate_vector(local_y, local_n, my_rank, 2);

    Print_vector(local_x, local_n, n, "=> The first vector is", my_rank, comm);
    Print_vector(local_y, local_n, n, "=> The second vector is", my_rank, comm);

    // Perform parallel scalar multiplication
    Parallel_scalar_multiplication(local_x, local_n, s);
    Parallel_scalar_multiplication(local_y, local_n, s);

    // Measure the time taken for dot product computation
    MPI_Barrier(comm);  // Synchronize before starting the timer
    start = MPI_Wtime();
    local_dot = Parallel_dot_product(local_x, local_y, local_n);
    MPI_Reduce(&local_dot, &global_dot, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
    end = MPI_Wtime();

    // Print the vectors after scalar multiplication
    Print_vector(local_x, local_n, n, "=> The first vector after scalar multiplication is", my_rank, comm);
    Print_vector(local_y, local_n, n, "=> The second vector after scalar multiplication is", my_rank, comm);

    // Print the dot product
    if (my_rank == 0) {
        printf("The dot product is %f\n", global_dot);
        printf("Dot product computation took %f seconds\n", end - start);
    }

    // Free allocated memory
    free(local_x);
    free(local_y);

    MPI_Finalize();
    return 0;
}  /* main */

/*-------------------------------------------------------------------
 * Function:  Check_for_error
 * Purpose:   Check whether any process has found an error.  If so,
 *            print message and terminate all processes.  Otherwise,
 *            continue execution.
 */
void Check_for_error(
      int       local_ok   /* in */,
      char      fname[]    /* in */,
      char      message[]  /* in */,
      MPI_Comm  comm       /* in */) {
   int ok;

   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) {
         fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname,
               message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */

/*-------------------------------------------------------------------
 * Function:  Allocate_vectors
 * Purpose:   Allocate storage for x and y
 */
void Allocate_vectors(
      double**   local_x_pp  /* out */,
      double**   local_y_pp  /* out */,
      int        local_n     /* in  */,
      MPI_Comm   comm        /* in  */) {
   int local_ok = 1;
   char* fname = "Allocate_vectors";

   *local_x_pp = malloc(local_n*sizeof(double));
   *local_y_pp = malloc(local_n*sizeof(double));

   if (*local_x_pp == NULL || *local_y_pp == NULL) local_ok = 0;
   Check_for_error(local_ok, fname, "Can't allocate local vector(s)",
         comm);
}  /* Allocate_vectors */

/*-------------------------------------------------------------------
 * Function:  Print_vector
 * Purpose:   Print a vector that has a block distribution to stdout
 */
void Print_vector(
      double    local_b[]  /* in */,
      int       local_n    /* in */,
      int       n          /* in */,
      char      title[]    /* in */,
      int       my_rank    /* in */,
      MPI_Comm  comm       /* in */) {

    double* b = NULL;
    int i;
    if (my_rank == 0) {
        b = malloc(n * sizeof(double));
        if (b == NULL) {
            fprintf(stderr, "Can't allocate temporary vector for printing\n");
            MPI_Finalize();
            exit(-1);
        }
    }

    MPI_Gather(local_b, local_n, MPI_DOUBLE, b, local_n, MPI_DOUBLE, 0, comm);

    if (my_rank == 0) {
        printf("%s\n", title);
        printf("\t");
        int elements_to_print = (n < 20) ? n : 10;
        for (i = 0; i < elements_to_print; i++)
            printf("%f ", b[i]);
        printf("\t");
        if (n >= 20) {
            printf("\n\t...\n");
            printf("\t");
            for (i = n - 10; i < n; i++)
                printf("%f ", b[i]);
        }
        printf("\n");
        free(b);
    }
} /* Print_vector */

/*---------------------------------------------------------------------
 * Function:  Generate_vector
 * Purpose:   Generate a vector with random numbers
 */
void Generate_vector(double local_a[], int local_n, int my_rank, int i_seed) {
    unsigned int seed = (unsigned int)time(NULL) + my_rank + i_seed;
    for (int i = 0; i < local_n; i++) {
        local_a[i] = (double)rand_r(&seed) / RAND_MAX;  // Generate a random number between 0 and 1
    }
}

/*---------------------------------------------------------------------
 * Function:  Parallel_dot_product
 * Purpose:   Compute the dot product of two vectors in parallel
 */
double Parallel_dot_product(double local_x[], double local_y[], int local_n) {
    double local_dot = 0.0;
    for (int i = 0; i < local_n; i++) {
        local_dot += local_x[i] * local_y[i];
    }
    return local_dot;
}

/*---------------------------------------------------------------------
 * Function:  Parallel_scalar_multiplication
 * Purpose:   Multiply each element of a vector by a scalar in parallel
 */
void Parallel_scalar_multiplication(double local_a[], int local_n, double scalar) {
    for (int i = 0; i < local_n; i++) {
        local_a[i] *= scalar;
    }
}
