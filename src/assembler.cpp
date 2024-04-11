#include<iostream>
#include "./pass1.h"
#include "./pass2.h"

using namespace std;

int main() {
    
    string input_file = "./../data/input.txt";
    bool _pass1 = pass1(input_file);

    if(_pass1 == 0){

        cout<<"------------ PASS 1 completed -----------"<<'\n';

        write_in_tables();

        string intermediate_file = "./../data/intermediate.txt";
        bool _pass2 = pass2(intermediate_file);
        if(_pass2 == 0) {
            cout<<"----------- PASS 2 completed ----------"<<'\n';
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