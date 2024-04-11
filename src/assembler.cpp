#include<iostream>
#include "./pass1.h"
#include "./pass2.h"

using namespace std;

int main() {

    bool _pass1 = pass1();

    if(_pass1 == 0){

        cout<<"------------ PASS 1 completed successfully -----------"<<'\n';

        write_tables();

        bool _pass2 = pass2();
        if(_pass2 == 0) {
            cout<<"----------- PASS 2 completed successfully ----------"<<'\n';
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