#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef W
#define W 20                                    // Width
#endif
int main(int argc, char **argv) {
		
		
  int L = atoi(argv[1]);                        // Length
  int iteration = atoi(argv[2]);                // Iteration
  srand(atoi(argv[3]));                         // Seed
  float d = (float) random() / RAND_MAX * 0.2;  // Diffusivity
  int *temp = malloc(L*W*sizeof(int));          // Current temperature
  int *next = malloc(L*W*sizeof(int));          // Next time step
  

  int count = 0, balance = 0; 
      
  for (int i = 0; i < L; i++) {
	for (int j = 0; j < W; j++) {
		temp[i*W+j] = random()>>3;
	}
  } 
  
  
  
  int my_rank;   /* My process rank */
  int p;         /* The number of processes*/ 
  int global_min;
  int all_balance;
  MPI_Status status;   
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
 
 
  
  int left = ( L / p ) * my_rank;
  int right = (( L / p ) * (my_rank + 1));  
  
  int temp_size = right - left ; 

  int *temp_rec1 = malloc(W*sizeof(int));
  int *temp_rec2 = malloc(W*sizeof(int));
  

  while (iteration--) {     // Compute with up, left, right, down points
    balance = 1;
    count++;	
    for (int i = left; i < right; i++) {
      for (int j = 0; j < W; j++) {
        float t = temp[i*W+j] / d;
        t += temp[i*W+j] * -4;
        t += temp[(i - 1 <  0 ? 0 : i - 1) * W + j];
        t += temp[(i + 1 >= L ? i : i + 1)*W+j];
        t += temp[i*W+(j - 1 <  0 ? 0 : j - 1)];
        t += temp[i*W+(j + 1 >= W ? j : j + 1)];
        t *= d;
        next[i*W+j] = t ;
        if (next[i*W+j] != temp[i*W+j]) {
          balance = 0;
        }
      }
    }
	MPI_Allreduce(&balance, &all_balance, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	
    if (all_balance==p) {
      break;
    }
	
    int *tmp = temp;
	temp = next;
    next = tmp;	
	  
   
	if(my_rank > 0 && iteration>0){
		for(int j=0; j < W ; j++){
			temp_rec1[j] = temp[left*W+j];
		}
		MPI_Send(temp_rec1,W,MPI_INT,my_rank-1,1,MPI_COMM_WORLD);
	}
	
	if(my_rank < p-1 && iteration>0){
		for(int j=0; j < W ; j++){
			temp_rec2[j] = temp[(right-1)*W+j];
		}
		MPI_Send(temp_rec2,W,MPI_INT,my_rank+1,2,MPI_COMM_WORLD);
	}
	if((my_rank > 0) && (iteration>0)){
		MPI_Recv(temp_rec1,W,MPI_INT,my_rank-1,2,MPI_COMM_WORLD,&status);
		for (int j = 0; j < W; j++) {
			temp [(left-1)*W+j]= temp_rec1[j];
		}
	}
	if((my_rank < p-1) && (iteration>0)){
		MPI_Recv(temp_rec2,W,MPI_INT,my_rank+1,1,MPI_COMM_WORLD,&status);
		for (int j = 0; j < W; j++) {
			temp [right*W+j]=temp_rec2[j];
		}
	}			
  }
  
  int min = temp[left*W];
  
  for (int i = left; i < right; i++) {
    for (int j = 0; j < W; j++) {
      if (temp[i*W+j] < min) {
        min = temp[i*W+j];
      }
    }
  }
  MPI_Reduce(&min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
  
  if(my_rank==0){
	printf("Size: %d*%d, Iteration: %d, Min Temp: %d\n", L, W, count, global_min);
  }
  MPI_Finalize();
  return 0;
}
