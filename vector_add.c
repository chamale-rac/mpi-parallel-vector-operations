/* File:     vector_add.c
 *
 * Purpose:  Implement vector addition
 *
 * Compile:  gcc -g -Wall -o vector_add vector_add.c
 * Run:      ./vector_add
 *
 * Input:    The order of the vectors, n, and the vectors x and y
 * Output:   The sum vector z = x+y
 *
 * Note:
 *    If the program detects an error (order of vector <= 0 or malloc
 * failure), it prints a message and terminates
 *
 * IPP:      Section 3.4.6 (p. 109)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Read_n(int* n_p);
void Allocate_vectors(double** x_pp, double** y_pp, double** z_pp, int n);
void Read_vector(double a[], int n, char vec_name[]);
void Print_vector(double b[], int n, char title[]);
void Vector_sum(double x[], double y[], double z[], int n);
void Generate_vector(double a[], int n);

/*---------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    // Check if the user provided the vector size as an argument
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <order of the vectors>\n", argv[0]);
        exit(-1);
    }

    // Receive n as an execution parameter
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Order of the vectors should be a positive integer\n");
        exit(-1);
    }

   double *x, *y, *z;
   clock_t start, end; // Variables to measure time
   double cpu_time_used;

   Allocate_vectors(&x, &y, &z, n);

   Generate_vector(x, n);
   Generate_vector(y, n);

   start = clock();
   Vector_sum(x, y, z, n);
    end = clock();

   // Calculate the time taken
   cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

   Print_vector(x, n, "=> The first vector is");
   Print_vector(y, n, "=> The second vector is");
   Print_vector(z, n, "=> The sum is");

   // Print the time taken for vector addition
   printf("Vector addition took %f seconds\n", cpu_time_used);

   free(x);
   free(y);
   free(z);

   return 0;
}  /* main */

/*---------------------------------------------------------------------
 * Function:  Read_n
 * Purpose:   Get the order of the vectors from stdin
 * Out arg:   n_p:  the order of the vectors
 *
 * Errors:    If n <= 0, the program terminates
 */
void Read_n(int* n_p /* out */) {
   printf("What's the order of the vectors?\n");
   scanf("%d", n_p);
   if (*n_p <= 0) {
      fprintf(stderr, "Order should be positive\n");
      exit(-1);
   }
}  /* Read_n */

/*---------------------------------------------------------------------
 * Function:  Allocate_vectors
 * Purpose:   Allocate storage for the vectors
 * In arg:    n:  the order of the vectors
 * Out args:  x_pp, y_pp, z_pp:  pointers to storage for the vectors
 *
 * Errors:    If one of the mallocs fails, the program terminates
 */
void Allocate_vectors(
      double**  x_pp  /* out */,
      double**  y_pp  /* out */,
      double**  z_pp  /* out */,
      int       n     /* in  */) {
   *x_pp = malloc(n*sizeof(double));
   *y_pp = malloc(n*sizeof(double));
   *z_pp = malloc(n*sizeof(double));
   if (*x_pp == NULL || *y_pp == NULL || *z_pp == NULL) {
      fprintf(stderr, "Can't allocate vectors\n");
      exit(-1);
   }
}  /* Allocate_vectors */

/*---------------------------------------------------------------------
 * Function:  Read_vector
 * Purpose:   Read a vector from stdin
 * In args:   n:  order of the vector
 *            vec_name:  name of vector (e.g., x)
 * Out arg:   a:  the vector to be read in
 */
void Read_vector(
      double  a[]         /* out */,
      int     n           /* in  */,
      char    vec_name[]  /* in  */) {
   int i;
   printf("Enter the vector %s\n", vec_name);
   for (i = 0; i < n; i++)
      scanf("%lf", &a[i]);
}  /* Read_vector */

/*---------------------------------------------------------------------
 * Function:  Print_vector
 * Purpose:   Print the contents of a vector (10 first and 10 last elements)
 * In args:   b:  the vector to be printed
 *            n:  the order of the vector
 *            title:  title for print out
 */
void Print_vector(
      double  b[]     /* in */,
      int     n       /* in */,
      char    title[] /* in */) {
    int i;
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
}  /* Print_vector */

/*---------------------------------------------------------------------
 * Function:  Vector_sum
 * Purpose:   Add two vectors
 * In args:   x:  the first vector to be added
 *            y:  the second vector to be added
 *            n:  the order of the vectors
 * Out arg:   z:  the sum vector
 */
void Vector_sum(
      double  x[]  /* in  */,
      double  y[]  /* in  */,
      double  z[]  /* out */,
      int     n    /* in  */) {
   int i;

   for (i = 0; i < n; i++)
      z[i] = x[i] + y[i];
}  /* Vector_sum */


/*---------------------------------------------------------------------
 * Function:  Generate_vector
 * Purpose:   Generate a vector with random numbers
 * In args:   n:  the order of the vector
 * Out arg:   a:  the vector to be generated
 */
void Generate_vector(double a[], int n) {
   srand(time(NULL)); // Seed for the random number generator
   for (int i = 0; i < n; i++) {
      a[i] = (double)rand() / RAND_MAX; // Generate a random number between 0 and 1
   }
}
