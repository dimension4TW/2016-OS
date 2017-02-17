#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>


using namespace std;

unsigned int *C, *A, *B;

int shmidC, shmidA, shmidB;

int main(){

    int n;
    cout<<"Enter your dimension: ";
    cin>>n;
    int fork_num;

    for( fork_num = 1;fork_num<=16;fork_num++){
      //cout<<"Need to fork "<<fork_num<<endl;
      struct timeval start, end;
      gettimeofday(&start, 0);
      shmidA = shmget(IPC_PRIVATE, sizeof(unsigned int)*n*n, IPC_CREAT | 0600);
      A = (unsigned int *)shmat(shmidA, NULL, 0);
      shmidB = shmget(IPC_PRIVATE, sizeof(unsigned int)*n*n, IPC_CREAT | 0600);
      B = (unsigned int *)shmat(shmidB, NULL, 0);
      shmidC = shmget(IPC_PRIVATE, sizeof(unsigned int)*n*n, IPC_CREAT | 0600);
      C = (unsigned int *)shmat(shmidC, NULL, 0);
      int part = n/fork_num;
      pid_t pid;
      for(int I=0;I<n;I++){
          for(int J=0;J<n;J++){
            C[I*n+J]= 0;
            A[I*n+J] = B[I*n+J] = I*n+J;
        }
      }

      int i,j,k;
      unsigned int sum = 0;
      for(int t = 0;t<fork_num;t++){
        int EndOfFork;
        if(t == fork_num-1) EndOfFork = n;
        else EndOfFork = (t+1)*part;
        //cout<<"Fork_num: "<<t<<endl;
        pid = fork();
        if(pid == 0){
                    //cout<<"C init!"<<endl;
          //cout<<"Fork No."<<t<<" count from "<<t*part<<endl; 
          for( i=t*part ; i< EndOfFork ; i++ ){
            for( j=0 ; j<n ; j++ ){
              for( k=0 ; k<n ; k++ ){
                //C[i*n + j] += A[i][k] * B[k][j];
                C[i*n+j] += A[i*n+k] * B[k*n+j];
          
              }
            }
          }
          //cout<<t<<" done!"<<endl;
          shmdt(C);
          exit(0);
        }
      }

      if(pid > 0){
        for(int w=0;w<fork_num;w++){
          wait(NULL);
          //cout<<C[3]<<endl;
        }
        for( int c =0 ; c<n*n ; c++ ){
          //cout<<"C["<<c<<"]="<<C[c]<<endl;
          sum += C[c];
        }
        
        //cout<<"Fork_num: "<<fork_num<<" Sum: "<<sum<<endl;
        shmdt(C);
        shmctl(shmidC, IPC_RMID, 0);
        shmdt(B);
        shmctl(shmidB, IPC_RMID, 0);       
        shmdt(A);
        shmctl(shmidA, IPC_RMID, 0);
      }
      gettimeofday(&end, 0);
      int sec = end.tv_sec-start.tv_sec;
      int usec = end.tv_usec-start.tv_usec;
      printf("Multiplying matrices using %d process\n",fork_num);
      printf("Elapsed time: %f sec, Checksum: %u\n",sec+(usec/1000000.0), sum);
    }
    return 0;
 }



