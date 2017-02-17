
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <iostream>
#include <cstdlib>

#include <string>
#include <sys/types.h>  
#include <dirent.h> 
#include <sys/stat.h>
#include <iomanip>

using namespace std;

string p;
string name;
int inode;
int q_max;
int q_min;
bool q_name;
bool q_inode;


int dir_recursive(char* path){  
        char glue='/';  


        struct stat buf;
        int res;
        res = stat(path, &buf);

        DIR * dp = opendir(path);  
       
        if (!dp){      
                return 1;  
        }  

        struct dirent *filename;      
        while((filename=readdir(dp))){  
                int f_inode;
                double f_size;
                string f_max;
                string f_min;
                int pathLength=strlen(path)+strlen(filename->d_name)+2;  
                char *pathStr = (char*)malloc(sizeof(char) * pathLength);  
                strcpy(pathStr, path);  

                int i=strlen(pathStr);  
                if(pathStr[i-1]!=glue){  
                        pathStr[i]=glue;  
                        pathStr[i+1]='\0';  
                }  

                strcat(pathStr, filename->d_name);
                
                struct stat buf;
                int res;
                res = stat(pathStr, &buf);
                 
                if (res != 0){
                    //perror ("Failed path string");
                }
                else{
                    
                    f_inode=buf.st_ino;
                    f_size=(double)buf.st_size/1048576;
                
                }  
        
                if(q_name && q_inode){
                    if(!strcmp(filename->d_name, name.c_str()) && f_size >= q_min && f_size<=q_max && inode == f_inode){
                        cout<<pathStr<<" "<<f_inode<<" "<<fixed<<setprecision(1)<<f_size<<" MB"<<endl;
                    }
                } 
                else if(q_name){
                    if(!strcmp(filename->d_name, name.c_str()) && f_size >= q_min && f_size<=q_max){
                        cout<<pathStr<<" "<<f_inode<<" "<<fixed<<setprecision(1)<<f_size<<" MB"<<endl;
                    }
                }

                else if(q_inode){
                    if(f_size >= q_min && f_size<=q_max && inode == f_inode){
                        cout<<pathStr<<" "<<f_inode<<" "<<fixed<<setprecision(1)<<f_size<<" MB"<<endl;
                    }
                }
                else{
                    if(f_size >= q_min && f_size<=q_max){
                        cout<<pathStr<<" "<<f_inode<<" "<<fixed<<setprecision(1)<<f_size<<" MB"<<endl;
                    }
                }

                if(!strcmp(filename->d_name,"..") || !strcmp(filename->d_name,".")){  
                        continue;  
                }  
                
                dir_recursive(pathStr);  

        }  

        closedir(dp);  

        return 1;  
}  



int main(int argc, char*argv[]){  

        int i=1;
        p="";
        name="";
        inode = 0;
        q_name = false;
        q_inode = false;
        q_min = 0;
        q_max =2147483647;

        while(i<argc){
                if(i==1){
                        p = argv[i];
                        i++;
                }
                else{
                        string tmp;
                        tmp = argv[i];
                        if(tmp == "-name"){
                                q_name = true;
                                name = argv[i+1];
                                i = i+2;
                        }
                        else if(tmp == "-inode"){
                                q_inode = true;
                                string tmp = argv[i+1];
                                inode = atoi(tmp.c_str());
                                i=i+2;
                        }
                        else if(tmp == "-size_min"){
                                string tmp = argv[i+1];
                                q_min = atoi(tmp.c_str());
                                i = i+2;
                        }
                        else if(tmp == "-size_max"){
                                string tmp = argv[i+1];
                                q_max = atoi(tmp.c_str());
                                i = i+2;
                        }

                } 
        } 
        char buff[128];
        memset(buff,0,sizeof(buff));
        strcpy(buff,argv[1]);      
        dir_recursive(buff);


        return 0;  
}  

