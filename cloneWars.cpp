#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <algorithm>
#include <iomanip>
using namespace std;

// int Rank(int, int, int*);
void merge_sort(int* A, int n, int my_rank, int p);

void clear( int* A, int n)    //Sets everything to 0
{
  for(int i = 0; i < n; i++)
    A[i] = 0;
}

int main(int argc, char* argv[]) 
{

	int my_rank;        // my CPU number for this process
  int p;              // number of CPUs that we have
  int source;         // rank of the sender
  int dest;           // rank of destination
  int tag = 0;
  char message[100];        // message number
  MPI_Status status;  // return status for receive

  // Start MPI
  MPI_Init(&argc, &argv);

  // Find out my rank!
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // Find out the number of processes!
  MPI_Comm_size(MPI_COMM_WORLD, &p);


  /* PROGRAM STARTS HERE */

  srand(time(NULL));
  int n = 6;
  int size = pow(2, n);

  int* A = new int[size];
  //if(my_rank ==0) 
  {
  	for(int x=0; x < size; x++) 
  	{
  		A[x] = rand() % 100;
  	}
  }

  // bcast the entire array A to all processors
  MPI_Bcast(&A[0], size, MPI_INT, 0, MPI_COMM_WORLD);

  if (my_rank == 0) 
   {
   		cout << "Unsorted Array" << endl;
    	for (int x = 0; x < size; x++) 
    	{
      		cout << A[x] << " ";
    	} cout << endl;
    }

  merge_sort(A, size, my_rank, p);

  if (my_rank == 0) 
   {
   		cout << "Sorted Array" << endl;
    	for (int x = 0; x < size; x++) 
    	{
      		cout << A[x] << " ";
    	} cout << endl;
    }

  MPI_Finalize();
  return 0;
}

// smerge function
void smerge(int* A, int A_front, int A_back, int* B, int B_front, int B_back, int* C, int C_front, int C_back) 
{

  while (A_front <= A_back && B_front <= B_back)
    if (A[A_front] < B[B_front])
      C[C_front++] = A[A_front++];
    else
      C[C_front++] = B[B_front++];
  while (A_front <= A_back) 
  	C[C_front++] = A[A_front++];
  while (B_front <= B_back)
   	C[C_front++] = B[B_front++];
}

int Rank(int valToFind, int size, int* Array)
{
    if(size == 1)
    	{
            if(valToFind < Array[0])
            	{
            		return 0;
            	}
            else 
            {
            	return 1;
            }
    	}
    else
    	{
        	if(valToFind < Array[size/2]){
        		return Rank(valToFind, (size/2), Array);
        }
        else
        	{
                return (size/2) + Rank(valToFind, size/2, &Array[size/2]);
        	}
    }
}

void pmerge(int* A, int* B, int* C, int size, int my_rank, int p)
{
	//Local Arrays
	

	for(int i = my_rank; i < 7; i+=p)
	{
		local_A[i] = 1 + (i*5);
		local_B[i] = 1 + (i*5);
	}

	 for(int i = my_rank; i < 7; i += p)
	 {
	 	local_A[i+7] = Rank(A[0+i*5], (size/2), B);
	 	local_B[i+7] = Rank(B[0+i*5], (size/2), A);
	 }

	 MPI_Allreduce(local_A, win_A, 4, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	 MPI_Allreduce(local_B, win_B, 4, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	 int* win_C = new int [size];
	 clear(win_C, size);

	 //Rank Arrays

	 for( int i = my_rank; i < 14+1; i+=p)
	 {
	 	smerge(A, win_A[i], win_A[i+1], B, win_B[i], win_B[i+1], win_C, win_A[i]+win_B[i], win_A[i+1]+win_B[i+1]);
	 }

	 MPI_Allreduce(win_C, C, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
}

void merge_sort(int* A, int n, int my_rank, int p)
{
	if(n == 32)
	{
		sort(A, A+n);
	} else 
	{
		int middle = n/2;
		merge_sort(&A[0], middle, my_rank, p);
		merge_sort(&A[middle], middle, my_rank, p);
		pmerge(&A[0], &A[middle], A, n, my_rank, p);
	}
}