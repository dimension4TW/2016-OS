#include <iostream>
#include <string>
#include <fstream>
#include<stdlib.h>
#include<stdio.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<wait.h>
#include<sys/time.h>
#include<sys/resource.h>
#define RECORD_SIZE 4096

using namespace std;

int main(){

    int dataset_fd;
    fstream file_i, file_i2, file_o;
    file_i.open("data.txt",fstream::in);
    file_i2.open("data.txt",fstream::in);
    file_o.open("new_data.txt",fstream::out);
    struct stat sb;
    dataset_fd = open("data.txt", O_RDWR);
    fstat(dataset_fd,&sb);
    //cout<<sb.st_size<<endl;
    int size = sb.st_size/RECORD_SIZE; 

    int counter1 = 0, counter2 = 0;
    for(int i=0;i<size;i++){
        string tmp, key;
        file_i >> tmp;
        key = tmp.substr(0,4);
        file_o << key;
    }
    for(int i=0;i<size;i++){
        string tmp, data;
        file_i2 >> tmp;
        data = tmp.substr(4,4092);
        file_o << data <<endl;
    }
    file_i.close();
    file_o.close();
    dataset_fd = open("new_data.txt", O_RDWR);
    fstat(dataset_fd,&sb);
    //cout<<sb.st_size<<endl;;

    return 0;
}




