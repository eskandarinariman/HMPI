/*
 * This program computes the Jacobi iterations to approximate the solution to a linear system of equations.
 * The code is a modification from the exercise described in the following site where further information can 
 * be found about the algorithm
 *
 * http://www.mcs.anl.gov/research/projects/mpi/tutorial/mpiexmpl/src/jacobi/C/main.html
 *
 * 
 */

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "mpi.h"
#include "common.h"

#define frac(x) (unsigned int)(((x)-(unsigned int)((x)))*1.e6)

// Define message tags. Useful for debugging purposes
#define DATA_TAG        0x10
#define PARAM_TAG       0x23
#define DIFFNORM_TAG    0x30
#define DONE_TAG        0x40

// Define surface parameters. 
// X and Y coordinates of the points in the grid. Useful for visualization purposes
#define X_LOW   -5.0
#define X_HIGH   5.0
#define Y_LOW   -5.0
#define Y_HIGH   5.0
#define DX (fabs(X_LOW-X_HIGH)/MAXCOL);
#define DY (fabs(Y_LOW-Y_HIGH)/MAXROW);

#define MAXCOL  10
#define MAXROW  10

// Uncomment to enable matrix printing to stdout
//#define PRINT_MATRIX_EN

// Set initial conditions, could be based on a function or random values
float 
initial_cond(float x, float y)
{
    return 100*x*x*exp(-x*x)*y*y*exp(-y*y);
    //return rand()%1000;
}

// Set initial values for the matrix
void
init_matrix(float **matrix, int nrows, int rank, int proc)
{
    float x,y;
    int i, j;
    y = Y_LOW+rank*(Y_HIGH-Y_LOW)/proc;
    // nrows includes the exchange rows, which should not be initialized
    for(i=1; i<nrows; i++)
    {
        x = X_LOW;
        for(j=0; j<MAXCOL; j++)
        {
	        matrix[i][j] = initial_cond(x,y);
            x += DX;
	    }
        y += DY;
    }
}

// Print to stdout the content of the matrix, mainly for debugging purposes
void 
print_matrix(float **matrix, int nrows, int rank, int proc)
{
    #ifdef PRINT_MATRIX_EN
    float x,y;
    int i, j;
    y = Y_LOW+rank*(Y_HIGH-Y_LOW)/proc;
    // nrows includes the exchange rows, which should not be printed
    for(i=1; i<nrows; i++)
    {
        x = X_LOW;
        for(j=0; j<MAXCOL; j++)
        {
	    PRINT("%f %f %f\r\n", x, y, matrix[i][j]);
            x += DX;
	}
        y += DY;
	PRINT("\r\n");
    }
    #endif
}

// Memory allocation
float **
allocate_matrix(int nrows, int ncols)
{
    float **matrix;
    int i;
    // allocate memory for local matrix. Each process does this
    if ( (matrix = (float**) malloc(nrows * sizeof(float *))) == NULL)
    {
        PRINT("Cannot allocate memory for matrix\r\n");
        for(;;);
    }
    else
    {
	for(i = 0; i < nrows; i++)
        {
            if ( (matrix[i] = (float*) malloc(ncols * sizeof(float))) == NULL)
            {
                PRINT("Cannot allocate memory for matrix\r\n");
                for(;;);
            }
        }
    }
    return matrix;
}

// Memory release
void
free_matrix(float **matrix, int nrows)
{
    int i;
    for(i = 0; i < nrows; i++)
    {
        free(matrix[i]);
    }
    
    if (matrix)
        free(matrix);
}

// ----------------------------------------------------------------------
int main( argc, argv )
int argc;
char **argv;
{
    int         rank, size, i, j, itcnt, q;
    float       diffnorm, gdiffnorm;
    float       **xlocal, **xnew;
    int         nrows;
    MPI_Status  status;
    double      t0, t1;


    PRINT( "%c[2J%c[H", 27, 27); // clean the terminal
    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    if ( rank == 0)
    {
        PRINT( "Jacobi test with %d processors\r\n", size);
        PRINT("File: %s\r\n",__FILE__);
        PRINT("Compiled %s %s\r\n",__DATE__,__TIME__);
    }

    PRINT("rank:%d, nP:%d, MAT:%dx%d\r\n", rank, size, MAXROW, MAXCOL);

    MPI_Barrier(MPI_COMM_WORLD);

    // divide the number of rows across all the ranks and 
    // add one row to the lower ranks in case the number of rows is not 
    // evenly divisible by the number of ranks.
    // Also, add two extra rows to exchange data with adjacent ranks
    nrows = MAXROW/size                         + 
            (rank < (MAXROW % size) ? 1 : 0)    + 
            2;

    // allocate memory for the matrices
    xlocal  = allocate_matrix(nrows, MAXCOL);
    xnew    = allocate_matrix(nrows, MAXCOL);

    // Rank 0 initializes its own xlocal (use xnew temporarily) and print it to the terminal
    if ( rank == 0 )
    {
        init_matrix(xnew, nrows, rank, size);
        PRINT("----- Initial Matrix ----\r\n");
        print_matrix(xnew, nrows, rank, size);

        // Now, initialize the slave's local matrices and send them
        for( i=1; i < size; i++)
        {
            int slave_nrows = (MAXROW/size) + (i<(MAXROW % size) ? 1 : 0) + 2; // I need to compute again nrows for each rank
            init_matrix(xlocal, slave_nrows, i, size);
            #ifdef VERBOSE
            print_matrix(xlocal, slave_nrows, i, size);
            #endif

            // Send one row at a time because there is no guarantee that the memory has been allocated contiguously between rows
            int ii;
            for(ii=0; ii<slave_nrows; ii++)
            { 
                MPI_Send(xlocal[ii], MAXCOL, MPI_FLOAT, i, DATA_TAG, MPI_COMM_WORLD);
            }
        }

        // Now rank 0 re-initializes its own xlocal using the temporary assignment of xnew
        for(i=0; i<nrows; i++)
            for(j=0; j<MAXCOL; j++)
                xlocal[i][j] = xnew[i][j];
    }
    else
    {
        int ii;
        for(ii=0; ii<nrows; ii++)
            MPI_Recv(xlocal[ii], MAXCOL, MPI_FLOAT, 0, DATA_TAG, MPI_COMM_WORLD, &status);
    }
    
    //-----------------------------
    // OK. All the matrices are initialized and the initial matrix has been printed to the terminal
    // Now, let's synchronize and compute!
    PRINT("computing...\r\n");
    MPI_Barrier(MPI_COMM_WORLD);

    itcnt = 0;
    t0 = TIME_STAMP();
    do 
    {
        // 
        // nrows include the two ghost rows: 0 and nrows-1
        //

        // Send down unless I'm at the bottom
        if (rank < size - 1) 
            MPI_Send( xlocal[nrows-2], MAXCOL, MPI_FLOAT, rank + 1, DATA_TAG, MPI_COMM_WORLD );

        // receive from above 
        if (rank > 0)
            MPI_Recv( xlocal[0], MAXCOL, MPI_FLOAT, rank - 1, DATA_TAG, MPI_COMM_WORLD, &status );

        // Send up unless I'm at the top
        if (rank > 0) 
            MPI_Send( xlocal[1], MAXCOL, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD );

        // 
        if (rank < size - 1) 
            MPI_Recv( xlocal[nrows-1], MAXCOL, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &status );
        
        // Compute new values (but not on boundary)
        itcnt ++;
        diffnorm = 0.0;
        for (i=1; i<nrows-1; i++) 
        {
            for (j=1; j<MAXCOL-1; j++) 
            {
                xnew[i][j] = (  xlocal[i][j+1] + xlocal[i][j-1] + xlocal[i+1][j] + xlocal[i-1][j]) / 4.0;
                diffnorm += (xnew[i][j] - xlocal[i][j]) * (xnew[i][j] - xlocal[i][j]);
            }
        }

        // Only transfer the interior points
        for (i=1; i<nrows-1; i++) 
        {
            for (j=1; j<MAXCOL-1; j++) 
            {
                xlocal[i][j] = xnew[i][j];
            }
        }
        //printf("%f\n", diffnorm);
        MPI_Allreduce( &diffnorm, &gdiffnorm, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD );
        printf("%f\n", gdiffnorm);
        gdiffnorm = sqrt( gdiffnorm );
        
        #ifdef VERBOSE
        if (rank == 0 )
        {
            PRINT
            ( 
                "At iteration %d, diff is %f\r\n",
                itcnt,
                gdiffnorm
            );
        }
        #endif
    } while ( gdiffnorm > 1.0e-2 && itcnt < 100 );
    t1 = TIME_STAMP();
    
    // We are done, now let's collect and print the results
    if ( rank == 0 )
    {       

        // Read back the final matrix
        PRINT("----- Final Matrix ----\r\n");
        print_matrix(xlocal, nrows, rank, size);

        for( i=1; i < size; i++)
        {
            int ii;
            int slave_nrows = (MAXROW/size) + (i<(MAXROW % size) ? 1 : 0) + 2; // need to compute again nrows for each rank
            for(ii=0; ii<slave_nrows; ii++)
                MPI_Recv(xlocal[ii], MAXCOL, MPI_FLOAT, i, DATA_TAG, MPI_COMM_WORLD, &status);
            print_matrix(xlocal, slave_nrows, i, size);
        }

        PRINT("----- Results ----\r\n");
        PRINT ( "At iteration %d, diff is %f, Time: %f\r\n", itcnt, gdiffnorm, (float)(t1-t0));
        

        // Now rank 0 re-initializes its own xlocal using the temporary assignment of xnew
        for(i=0; i<nrows; i++)
            for(j=0; j<MAXCOL; j++)
                xlocal[i][j] = xnew[i][j];
    }
    else
    {
        int ii;
        for(ii=0; ii<nrows; ii++)
            MPI_Send(xlocal[ii], MAXCOL, MPI_FLOAT, 0, DATA_TAG, MPI_COMM_WORLD);
    }


    free_matrix(xlocal, nrows);
    free_matrix(xnew, nrows);

    MPI_Finalize( );

    //PRINT("Done!\r\n");
    return 0;
}

