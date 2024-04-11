#include<iostream>
#include "./pass1.h"
#include "./pass2.h"

using namespace std;

int main() {

    bool _pass1 = pass1();

    if(_pass1 == 0){

        cout<<"------- PASS 1 completed successfully -----------"<<'\n';

        ofstream symbol_table("./../data/SYMTAB.txt");
        ofstream block_table("./../data/BLOCKTAB.txt");

        // write in SYMTAB
        cout<<"---------------SYMTAB ------------------"<<endl;
        for(auto i:SYMTAB){
            symbol_table<<i.second.label<<" "<<i.second.block_no<<" "<<i.second.value<<" "<<i.second.isValid<<endl;
        }

        // Write in BLOCKTABLE
        cout<<"---------------BLOCKTABLE ------------------"<<endl;
        for(auto i:BLOCKTABLE){
            block_table<<i.second.block_name<<" "<<i.second.block_no<<" "<<hex<<i.second.block_locctr<<" "<<i.second.start_address<<endl;
        }

        bool _pass2 = pass2();
        if(_pass2 == 0) {
            cout<<"------------------- PASS 2 completed successfully --------------------"<<'\n';
        }
        else {
            cout<<"------------ Error in PASS 2 ----------------"<<endl;
        }

    }
    else{
        cout<<"------------ Error in PASS 1 ----------------"<<endl;
    }

    return 0;
}