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
#include <queue>
#include <string>
#include <sstream> 
#define ACTIVE 1
#define KILLED 0

using namespace std;

struct job{
    int left;
    int right;
    int level;
    int status;
};

queue<job> job_list;
sem_t job_lock; // for queue lock
sem_t mutex; //for job
sem_t thread_limit;
sem_t finish;
sem_t kill_thread;


pthread_t tid[16];

int A[2000000], B[2000000];

bool cmp (int i, int j){return(i<j);}

void* Sort(void*)
{
    
    while(1){
        
        sem_wait(&(thread_limit));
        sem_wait(&(mutex));
  
        job current_job, next_jobl, next_jobr;
        sem_wait(&(job_lock));
        current_job = job_list.front();
        job_list.pop();
        sem_post(&(job_lock));  

        int myright = current_job.right;
        int myleft = current_job.left;
        int mylevel = current_job.level;
        int mystatus = current_job.status;
        //cout<<"current job level = "<< mylevel<<endl;
        
        if(mystatus == KILLED){
            sem_post(&(kill_thread));
            return NULL;
        }
        else{
 
            if(mylevel <= 7){ 
                int wall = myleft-1;
                int pivot;
  

                if(myright > myleft){
                    for(int current = myleft; current < myright;current++){
                    if(A[current] < A[myright]){
                        wall++;
                        swap(A[current],A[wall]);
                    }
                    }
                    swap(A[wall+1],A[myright]);
                }
                pivot = wall+1;
                next_jobl.left = myleft;
                next_jobl.right = pivot-1;
                next_jobl.level = 2*mylevel;
                next_jobl.status = ACTIVE;

                next_jobr.left = pivot+1;
                next_jobr.right = myright;
                next_jobr.level = mylevel*2+1;
                next_jobr.status = ACTIVE;

                sem_wait(&(job_lock));
                job_list.push(next_jobl);
                //cout<<"current level: "<<current_job.level<<", push level: "<<next_jobl.level<<endl;
                job_list.push(next_jobr);
                //cout<<"current level: "<<current_job.level<<", push level: "<<next_jobr.level<<endl;
                sem_post(&(job_lock));
                sem_post(&(mutex));
                sem_post(&(mutex));
    
            }
            else{
                if(myright > myleft){
                    sort(A+myleft,A+myright+1,cmp);
                }
                sem_post(&(finish));
            }
        }

        sem_post(&(thread_limit));

    }
}

int main(void)
{
    int i;
    int err;
    int size; 
    string inputfile = "input.txt";
    fstream file_i;
    file_i.open(inputfile.c_str(), ios::in);
    
    file_i >> size;
    for(int z=0;z<size;z++){
        file_i >> B[z];
    }


    for(i=1;i<=8;i++){
        struct timeval start_s, end_s, start_m, end_m;
        stringstream convert; // stringstream used for the conversion
        convert << i;//add the value of Number to the characters in the stream
        string filename = "output_" + convert.str() +".txt";
        //cout<<filename<<endl;
        fstream file_o;
        file_o.open(filename.c_str(), ios::out);
       
        /* 
        while(!job_list.empty()){
            job_list.pop();
        } 
        */

        for(int z=0;z<size;z++){
            A[z] = B[z];
        }

        sem_init(&(job_lock),0,1);
        sem_init(&(mutex),0,0);
        sem_init(&(finish),0,0);
        sem_init(&(thread_limit),0,i);
        sem_init(&(kill_thread),0,0);
        
        job first_job;
        first_job.left = 0; 
        first_job.right = size - 1;
        first_job.level = 1;
        first_job.status = ACTIVE;
        job_list.push(first_job);
        sem_post(&(mutex));

        gettimeofday(&start_m,0); 
        for(int j =0;j<i;j++){
            err = pthread_create(&(tid[j]),NULL, &Sort, NULL);
        }
        
        for(int j=0;j<8;j++){
            sem_wait(&(finish));
        }
        for(int j=0;j<i;j++){
            job last_job;
            last_job.left = 0;
            last_job.right = 0;
            last_job.level = 0;
            last_job.status = KILLED;
            sem_wait(&(job_lock));
            job_list.push(last_job);
            sem_post(&(job_lock));
            sem_post(&(mutex));
        }
        for(int j=0;j<i;j++){
            sem_wait(&(kill_thread));
        }
        gettimeofday(&end_m,0);

        int sec_m = end_m.tv_sec-start_m.tv_sec;
        int usec_m = end_m.tv_usec-start_m.tv_usec;
        printf("n= %d, elased %f ms\n",i,sec_m*1000+(usec_m/1000.0));

        for(int t=0;t<size;t++){
            file_o<<A[t]<<" ";
        }  
        //cout<<i<<" done"<<endl;  
    }

    return 0;
}
