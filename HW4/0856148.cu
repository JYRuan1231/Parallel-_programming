/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* DESCRIPTION :
*
Serial Concurrent Wave Equation - C Version
*
This program implements the concurrent wave equation
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>
# include <cuda_runtime.h>

# define MAXPOINTS 1000000
# define MAXSTEPS 1000000
# define MINPOINTS 20
# define PI 3.14159265
# define DIM_GRID 1
# define BLOCK_SIZE 256



void check_param ( void ) ;
void init_line ( void ) ;
void update ( void ) ;
void printfinal ( void ) ;

int nsteps ,                    /*number of time steps */
    tpoints ,                   /*total points along string */
    rcode ;                     /*generic return code */
float values [ MAXPOINTS +2],   /*values at time t */
      oldval [ MAXPOINTS +2],   /*values at time (t - dt ) */
      newval [ MAXPOINTS +2];   /*values at time ( t + dt ) */

float *d_values, *d_oldval, *d_newval;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                      Checks input values from parameters
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void check_param ( void )
{
    char tchar [20];
    /* check number of points , number of iterations */
    while (( tpoints < MINPOINTS ) || ( tpoints > MAXPOINTS ) ) {
        printf ( "Enter number of points along vibrating string [%d-%d]: "
                , MINPOINTS , MAXPOINTS ) ;
        scanf ( "%s" , tchar ) ;
        tpoints = atoi ( tchar ) ;
        if (( tpoints < MINPOINTS ) || ( tpoints > MAXPOINTS ) )
            printf ( "Invalid. Please enter value between %d and %d\n" ,
                    MINPOINTS , MAXPOINTS ) ;
    }
    while (( nsteps < 1) || ( nsteps > MAXSTEPS ) ) {
        printf ( "Enter number of time steps [1-%d]: " , MAXSTEPS ) ;
        scanf ( "%s" , tchar ) ;
        nsteps = atoi ( tchar ) ;
        if (( nsteps < 1) || ( nsteps > MAXSTEPS ) )
            printf ( "Invalid. Please enter value between 1 and %d\n" ,
                    MAXSTEPS ) ;
    }
    printf ( "Using points = %d, steps = %d\n", tpoints, nsteps) ;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                   Initialize points on line
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
__global__ void init_line (float* values_d, float* oldvalue_d, int tpoints)
{   
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;
    float fac = 2.0 * PI;
    float x;
    for ( int i = idx; i <= tpoints; i+=stride) {
        x = (float)(i-1)/(tpoints-1);           //might loss some precision.
         values_d[i] = __sinf(fac * x);
         oldvalue_d [i] = values_d[i];
    }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                   Calculate new values using wave equation
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*               Update all values along line a specified number of times
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
__global__ void update(float* values_d, float* oldvalue_d, int tpoints, int nsteps)
{
    int i, j;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = gridDim.x * blockDim.x;
    float newval;
    /* Update values for each time step */
    for ( i = 1; i <= nsteps ; i++) {
        /* Update points along line for this time step */
        for ( j = index; j <= tpoints; j+=stride) {
            if (( j == 1) || ( j == tpoints ) )
                newval = 0.0;
            else
                newval  = (1.82)*values_d[j] - oldvalue_d[j];

            oldvalue_d [ j ] = values_d [ j ];   /* Update old values with new values */
            values_d [ j ] = newval;
        }
       
    }
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                   Print final results
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void printfinal()
{
    int i ;
    for( i = 1; i <= tpoints ; i ++){
        printf("%6.4f ", values[i]);
        if( i %10 == 0)
            printf("\n");
    }
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                           Main program
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int main (int argc, char *argv[])
{   
    sscanf (argv[1],"%d" ,&tpoints);
    sscanf (argv[2],"%d" ,&nsteps);

    int numBlock = (tpoints+1+BLOCK_SIZE)/BLOCK_SIZE;  
    cudaMalloc(&d_values, (MAXPOINTS+2) * sizeof(float));
    cudaMalloc(&d_oldval, (MAXPOINTS+2) * sizeof(float));
    check_param();

    printf("Initializing points on the line...\n");
    init_line<<<numBlock, BLOCK_SIZE>>>(d_values, d_oldval, tpoints);
    
    cudaMemcpy(values, d_values, (MAXPOINTS+2) * sizeof(float), cudaMemcpyDeviceToHost);
    for(int i = 1; i<=tpoints; i++){
        printf("%f\n", *(values+i));
    }
    
    
    printf("Updating all points for all time steps...\n");
    update<<<numBlock, BLOCK_SIZE>>>(d_values, d_oldval, tpoints, nsteps);

    printf("Printing final results...\n");
    cudaMemcpy(values, d_values, (MAXPOINTS+2) * sizeof(float), cudaMemcpyDeviceToHost);
    printfinal();

    printf("\nDone.\n\n");
    return 0;
}