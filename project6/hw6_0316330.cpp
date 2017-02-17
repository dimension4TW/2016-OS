#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include <map>
#include <iomanip>
using namespace std;

struct pageNode{
    
    pageNode *last;
    pageNode *next;
    string value;

    pageNode(string val){
        this->value = val;
    }
    
};

class Page{
    public:
        pageNode *MRU;
        pageNode *LRU;
        
        void addHelper(pageNode *current,string val){
            pageNode *tmp = new pageNode(val);
            this->MRU->last = tmp;
            tmp->next = this->MRU;
            this->MRU = tmp;
            
        }
        void delHelper(){
            this->LRU = this->LRU->last;
            delete this->LRU->next;
            this->LRU->next = NULL;
        }

        void deleteNode(){
            this->delHelper();
        }

        pageNode* add(string val) {
            if (MRU) {
                this->addHelper(MRU, val);
            } 
            else {
                this-> MRU = new pageNode(val);
                this-> LRU = MRU;
            }
            return this->MRU;
        }
};


int main() {

    cout<<"FIFO--"<<endl;
    cout<<"size\t"<<"miss\t"<<"hit\t"<<"page fault ratio"<<endl;
    for(int i=64;i<=512;i*=2){ 
        Page *mypage = new Page();
        map<string, pageNode*> lookup;
        fstream file_i;
        file_i.open("trace.txt",fstream::in);
        string op, addr;
        int hit = 0, miss = 0;
        double faultRate;
        int full = i;
        int current_full = 0;
        map<string,pageNode*>::iterator key;
        while(file_i>>op){
            file_i>>addr;
            addr = addr.substr(0,5);
            key = lookup.find(addr);
            if(key != lookup.end()){
                hit++;
            }
            else{
                miss++;
                if(current_full == full){
                    string delVal = mypage->LRU->value;
                    lookup.erase(delVal);
                    mypage->deleteNode();
                }
                else{
                    current_full++;
                }
                pageNode* addPage = mypage ->add(addr);
                lookup[addr] = addPage;
            }
        }

        cout<<i<<"\t"<<miss<<"\t"<<hit<<"\t"<<(double)miss/(double)(hit+miss)<<endl;
        delete mypage;
    }

    cout<<"LRU--"<<endl;
    cout<<"size\t"<<"miss\t"<<"hit\t"<<"page fault ratio"<<endl;
    for(int i=64;i<=512;i*=2){
        Page *mypage = new Page();
        map<string, pageNode*> lookup;
        fstream file_i;
        file_i.open("trace.txt",fstream::in);
        string op, addr;
        int hit = 0, miss = 0;
        double faultRate;
        int full = i;
        int current_full = 0;
        map<string,pageNode*>::iterator key;
        while(file_i>>op){
            file_i>>addr;
            addr = addr.substr(0,5);
            key = lookup.find(addr);
            if(key != lookup.end()){
                hit++;
                //cout<<"hit"<<endl;
                //update_key = lookup.find(addr);
                pageNode* update = key->second;
                if(update!=mypage->MRU){
                    if(update == mypage->LRU) {
                        mypage->LRU = mypage->LRU->last; 
                    }
                    else{
                        update->next->last = update->last;
                    }
                    update->last->next = update->next;
                    delete update;
                    pageNode* updatePage = mypage->add(addr);
                    key->second = updatePage;
                    //cout<<addr<<endl;
                }
            }
            else{
                miss++;
                //cout<<"miss"<<endl;
                if(current_full == full){
                    string delVal = mypage->LRU->value;
                    lookup.erase(delVal);
                    mypage->deleteNode();
                }
                else{
                    current_full++;
                }
                pageNode* addPage = mypage ->add(addr);
                lookup[addr] = addPage;
                //cout<<addr<<endl;
            }
        }
        cout<<i<<"\t"<<miss<<"\t"<<hit<<"\t"<<(double)miss/(double)(hit+miss)<<endl;
        //cout<<"page fault ratio: "<<(double)miss/(double)(hit+miss)<<setw(9)<<endl;
        delete mypage;
    }



}
