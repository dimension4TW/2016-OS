#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <iostream>
#include <semaphore.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sys/time.h>

using namespace std;

pthread_t tid[16];
sem_t mutex[24];

int A[2000000], B[2000000];
int myleft[16], myright[16], size;
int ID[16];

bool cmp (int i, int j){return(i<j);}

void* Sort(void *I)
{

  int pivot;
  int i = *(int*)I;
  //cout<<"Thread :"<<i<<endl;
  sem_wait(&(mutex[i]));
  //cout<<"Thread: "<< i<<" Running"<<endl;
  int wall = myleft[i]-1;
  

  if(myright[i]>myleft[i]){
    //pivot = myright[i];
    for(int current = myleft[i]; current < myright[i];current++){
      if(A[current] < A[myright[i]]){
        wall++;
        swap(A[current],A[wall]);
      }
    }
    swap(A[wall+1],A[myright[i]]);
  }
  pivot = wall+1;
  //cout<<"Thread :"<<i<<" done!"<<endl;
  myleft[2*i] = myleft[i];
  myright[2*i] = pivot-1;
  //if(wall!=myright[i]) myleft[2*i+1] = wall+1;
  //else myleft[2*i+1] = wall;
  myleft[2*i+1] = pivot+1;
  myright[2*i+1] = myright[i];
  //printf("Thread %d myleft[2i]=%d myright[2i]=%d myleft[2i+1]=%d myright[2i+1]=%d\n",i,myleft[2*i], myright[2*i], myleft[2*i+1], myright[2*i+1]);
  sem_post(&(mutex[2*i]));
  sem_post(&(mutex[2*i+1]));
  return NULL;
}

void* BSort(void *I){
  int i = *(int*)I;
  sem_wait(&(mutex[i]));
  //cout<<"Thread :"<<i<<"Running"<<endl;
  if(myright[i] > myleft[i]){

    sort(A+myleft[i],A+myright[i]+1,cmp);
    /*
   // printf("\n");
    for(int j=myleft[i];j<=myright[i];j++){
      for(int k=myleft[i];k<=myright[i]-1;k++){
        if(A[k] > A[k+1]) swap(A[k],A[k+1]);
      }
    }
    for(int x = myleft[i]; x<=myright[i] ;x++){
     // printf("%d ",A[x]);
    }
    //printf("\n");
    */
  }
  sem_post(&(mutex[i+8]));
  return NULL;
}

int main(void)
{
  int i;
  int err;
  struct timeval start_s, end_s, start_m, end_m;
  string inputfile;
  cin >> inputfile;
  fstream file_i, file_o1, file_o2;
  file_i.open(inputfile.c_str(), ios::in);
  file_o1.open("output1.txt", ios::out); //MT
  file_o2.open("output2.txt", ios::out); //ST
  for(int j=0;j<16;j++){
    ID[j] = j;
  }
  //cout<<"Size :";
  file_i >> size;
  myleft[1] = 0;
  myright[1] = size-1;
  //A = (int*) malloc ((i+1)*sizeof(int));
  for(int z = 0;z<size;z++){
    file_i>>A[z];
    B[z] = A[z];
  }
  /*
  printf("input:\n");
  for(int j=0;j<size;j++){
    cout<<A[j]<<" ";
  }
  cout<<endl;
  */
  
  //cout<<"--------------------Init----------------"<<endl;
  for(int j=1;j<24;j++){
    sem_init(&(mutex[j]), 0, 0);
  }

  sem_init(&(mutex[1]), 0, 1);
  //cout<<"--------------------Init Done----------------"<<endl;
  gettimeofday(&start_m,0);
  for(i=1;i<16;i++)
  {
    if(i<=7){
      err = pthread_create(&(tid[i]), NULL, &Sort, (void *)&(ID[i]));
      if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));

    }
    else{  
      err = pthread_create(&(tid[i]), NULL, &BSort, (void *)&(ID[i]));
      if(err != 0){
        printf("fail !\n");
      }
    }
  }
  
  for(int w = 16 ;w<24;w++){
    sem_wait(&(mutex[w]));
  }
  gettimeofday(&end_m,0);

  printf("multi-thread ");
  for(int j=0;j<size;j++){
    file_o1 << A[j] <<" ";
    //printf("%d ",A[j]);
  }
  cout<<endl;
 int sec_m = end_m.tv_sec-start_m.tv_sec;
 int usec_m = end_m.tv_usec-start_m.tv_usec;
 printf("elased %f ms\n",sec_m*1000+(usec_m/1000.0));
  
 printf("single thread ");
 gettimeofday(&start_s, 0);
 sort(B, B+size, cmp);
 gettimeofday(&end_s,0);
 for(int j=0;j<size;j++){
   file_o2<<B[j]<<" ";
 }
 cout<<endl;
 int sec_s = end_s.tv_sec-start_s.tv_sec;
 int usec_s = end_s.tv_usec-start_s.tv_usec;
 printf("elased %f ms\n",sec_s*1000+(usec_s/1000.0));




  return 0;
}
