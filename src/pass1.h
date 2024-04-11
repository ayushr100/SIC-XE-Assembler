#ifndef PASS1_H
#define PASS1_H

#include<iostream>
#include "./functions.h"
using namespace std;

bool pass1(string input_file){
    
    ifstream pass1_input(input_file);
    ofstream pass1_output("./../data/intermediate.txt");
    ofstream pass1_error("./../data/error.txt");

    bool ERROR_FLAG_PASS1 = false;

    string line;
    string label, opcode, operand;

    int LOCCTR = 0;

    int current_block_no = 0;
    string current_block_name = "DEFAULT";
    int total_blocks = 0;
    int line_no=0;
    int star_count=0;// for BASE *
    int literal_count=0;

    //handling start and use

    while(getline(pass1_input, line)){
        line_no++;
        //comment or blank line
        if(!process_line_pass1(line, label, opcode, operand)){
            pass1_output<<line<<endl;
            continue;
        }

        //check label has length <=6
        if(label.size()>6) {
            pass1_error<<"Label size is greater than 6 at line no. "<<line_no<<endl;
            pass1_output<<".-------- ERROR --------------"<<endl;
            ERROR_FLAG_PASS1=true;
            continue;
        }

        if(opcode=="START"){
            //store program name
            program_name = label;
            LOCCTR = stoi(operand, nullptr, 16);                                 //initial location counter
            if(BLOCKTABLE.find("DEFAULT") == BLOCKTABLE.end()) {
                BLOCKTABLE["DEFAULT"] = {"DEFAULT",  total_blocks, 0, 0, 0};
                total_blocks++;
                current_block_no=BLOCKTABLE["DEFAULT"].block_no;
                pass1_output<<line_no<< " "<<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            
            break;
        }
        
        else if(opcode=="USE"){
            //add in BLOCKTABLE
            if(operand==""){
                operand="DEFAULT";
            }
            BLOCKTABLE[operand] = {operand, total_blocks, 0, 0, 0};
            total_blocks++;
            current_block_no=BLOCKTABLE[operand].block_no;
            LOCCTR = BLOCKTABLE[operand].block_locctr;
            pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            continue;
        }
        
        else{
            pass1_error<<"START assembler directive not found at line no. "<<line_no<<endl;
            pass1_output<<".--------ERROR--------------"<<endl;
            ERROR_FLAG_PASS1=true;
            break;
        }
         
    }

    //handling further lines
    while(getline(pass1_input, line)){
        
        line_no++;
        //comment or blank line
        if(!process_line_pass1(line, label, opcode, operand)){
            pass1_output<<line<<endl;
            continue;
        }
        //checklabel length <=6
        if(label.size()>6) {
            pass1_error<<"Label size is greater than 6 at line no. "<<line_no<<endl;
            pass1_output<<".--------ERROR--------------"<<endl;
            ERROR_FLAG_PASS1=true;
            continue;
        }

        else if(opcode=="START"){
            pass1_error<<"START appeared for the second time at line no. "<<line_no<<endl;
            pass1_output<<".--------ERROR--------------"<<endl;
            ERROR_FLAG_PASS1=true;
            continue;
        }
        
        else if(opcode=="USE"){
            if(label!=""){
                pass1_error<<"Trying to assign label to USE assembler directive at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            //update LOCCTR of previous block
            BLOCKTABLE[current_block_name].block_locctr=LOCCTR;

            if(operand==""){
               operand="DEFAULT";
            }

            if(BLOCKTABLE.find(operand) == BLOCKTABLE.end()){
                BLOCKTABLE[operand] = {operand, total_blocks, 0, 0, 0};
                total_blocks++;
                current_block_no=BLOCKTABLE[operand].block_no;
                LOCCTR = BLOCKTABLE[operand].block_locctr;
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            else{
                current_block_no=BLOCKTABLE[operand].block_no;
                LOCCTR = BLOCKTABLE[operand].block_locctr;
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }

            current_block_name=operand;
        }
        
        else if(opcode=="EQU"){
            if(label==""){
                pass1_error<<"No label assigned to "<< opcode <<" at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
           
            if(operand=="" || operand[0]=='='){
                pass1_error<<"Trying to assign wrong operand to "<< opcode <<" at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }

            //if operand is decimal value
            else if(is_operand_absolute(operand)){
                int value = stoi(operand, nullptr, 10);
                SYMTAB[label] = {label, current_block_no, value, 2};                //absolute label
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            // if operand is a symbol
            else if(SYMTAB.find(operand) != SYMTAB.end()){
                SYMTAB[label] = {label, current_block_no, SYMTAB[operand].value, 1};                //relative label
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            else if(operand == "*") {
                //direct value assignment to label
                SYMTAB[label] = {label, current_block_no, LOCCTR, 1};//relative label
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            
            else{
                //handle expression 
                bool isValid = true;
                bool isRelative = false;
                int value = 0;
                expression_handler(operand, value, isValid, isRelative);
                if(isValid){
                    if(isRelative){
                        SYMTAB[label] = {label, current_block_no, value, 1};                //relative label
                    }
                    else{
                        SYMTAB[label] = {label, current_block_no, value, 2};                    //absolute label
                    }
                    pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                }
                else{
                    pass1_error<<"Expression for EQU is invalid at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }    
            }
        }
        
        else if(opcode=="ORG"){
            if(label!=""){
                pass1_error<<"Trying to assign label to "<< opcode <<" at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
           
            if(operand=="" || operand[0]=='='){
                pass1_error<<"Trying to assign wrong operand to "<< opcode <<" at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }

            //if operand is decimal value
            else if(is_operand_absolute(operand)){
                LOCCTR = stoi(operand, nullptr, 10);
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            // if operand is a symbol
            else if(SYMTAB.find(operand) != SYMTAB.end()){
                LOCCTR=SYMTAB[operand].value;
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            else{
                //handle expression
                bool isValid = true;
                bool isRelative = false;
                int value = 0;
                expression_handler(operand, value, isValid, isRelative);
                if(isValid){
                    if(isRelative){
                        LOCCTR = value;
                    }
                    else{
                        LOCCTR = value;
                    }
                    pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                }
                else{
                    pass1_error<<"Expression for ORG is invalid at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
            }
        }

        else if(opcode=="BASE"){
            if(label!=""){
                pass1_error<<"Trying to assign label to BASE at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            //literal
            if(operand=="*"){
                star_count++;
                char ch='0'+star_count;
                operand+=ch;
                LITTAB.insert({operand, LOCCTR});
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
        }

        else if(opcode=="NOBASE"){
            if(!(label!="" && operand!="")){
                //error
            }
            pass1_output<<line_no<< " "  
            <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
        }

        else if(opcode=="LTORG"){
            if(label!="" || operand!=""){
                //error
                if(label!="") pass1_error<<"Trying to assign label to LTORG at line no. "<<line_no<<endl;
                else pass1_error<<"Trying to assign label to LTORG at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            pass1_output<< line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;

            for(auto i:LITTAB){
                if(i.first[0]=='*'){//this is for BASE *
                    SYMTAB[i.first] = {i.first, current_block_no, i.second, 1};                 //relative label
                }
                else if(i.first[0]=='='){
                    SYMTAB[i.first] = {i.first, current_block_no, LOCCTR, 1};                           //relative label
                    pass1_output<<line_no<<" "  <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" BYTE "<<i.first<<endl;
                    if(i.first[1]=='C'){
                        LOCCTR+=i.first.length()-4;
                    }
                    else if(i.first[1]=='X' && i.first.length()%2==0){
                        LOCCTR+=(i.first.length()-4)/2;
                    }
                }
    
            }
            LITTAB.clear();
        }

        else if(opcode=="WORD"){
            if(operand==""){
                //error
                pass1_error<<"No operand for WORD at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            if(label!=""){
                if(SYMTAB.find(label)!=SYMTAB.end()){
                    pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                else{
                    SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                    //relative label
                }
            }

            if(!is_operand_absolute(operand)) {
                //error
                pass1_error<<"Value for WORD is invalid at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }

            pass1_output<<line_no<< " "<<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            LOCCTR+=3;
        }

        else if(opcode=="BYTE"){
            if(operand=="") {
                pass1_error<<"No operand for BYTE at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            if(label!=""){
                if(SYMTAB.find(label)!=SYMTAB.end()){
                    //error
                    pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                else{
                    SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                       //relative label
                }
            }

            if(operand[0]=='X'){
                if(!(operand[1]=='\'' && operand[operand.length()-1]=='\'') || (operand.length()-3)%2!=0){
                   //error
                    pass1_error<<"Value for BYTE is invalid at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                pass1_output<<line_no<< " "  
                <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                LOCCTR+=(operand.length()-3)/2;
            }
            else if(operand[0]=='C'){
                if(!(operand[1]=='\'' && operand[operand.length()-1]=='\'')){
                    // error
                    pass1_error<<"Value for BYTE is invalid at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                pass1_output<<line_no<< " "  
                <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                LOCCTR+=operand.length()-3;
            }
            else{
                //error
                pass1_error<<"Value for BYTE is invalid at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
        }

        else if(opcode=="RESW"){
            if(operand==""){
                //error
                pass1_error<<"No value for BYTE present at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            if(label!=""){
                if(SYMTAB.find(label)!=SYMTAB.end()){
                    //error
                    pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                else{
                    SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                       //relative label
                }
            }
            if(is_operand_absolute(operand)) {
                pass1_output<<line_no<< " "  
                <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                LOCCTR+=3*stoi(operand, nullptr, 10);
            }
            else{
                //error
                pass1_error<<"Value for RESW is invalid at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
        }

        else if(opcode=="RESB"){
            if(operand==""){
                //error
                pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            if(label!=""){
                if(SYMTAB.find(label)!=SYMTAB.end()){
                    //error
                    pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                else{
                    SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                       //relative label
                }
            }
            if(is_operand_absolute(operand)) {
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                LOCCTR+=stoi(operand, nullptr, 10);
            }
            else{
                //error
                pass1_error<<"Value for RESB is invalide at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
            
        }

        else if(opcode=="END"){
            if(label!=""){
                pass1_error<<"Trying to assign label to END at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
            }
            else {
                pass1_output<<line_no<< " " <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            }
            //clear LITTAB
            for(auto i:LITTAB){
                if(i.first[0]=='*'){                            //this is for BASE *
                    SYMTAB[i.first] = {i.first, current_block_no, i.second, 1};                             //relative label
                }
                else if(i.first[0]=='='){
                    SYMTAB[i.first] = {i.first, current_block_no, LOCCTR, 1};                           //relative label
                    pass1_output<<line_no<<"  "  <<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" BYTE "<<i.first<<endl;
                    if(i.first[1]=='C'){
                        LOCCTR+=i.first.length()-4;
                    }
                    else if(i.first[1]=='X' && i.first.length()%2==0){
                        LOCCTR+=(i.first.length()-4)/2;
                    }
                    
                }
    
            }
            BLOCKTABLE[current_block_name].block_locctr=LOCCTR;
            LITTAB.clear();
            break;
        }
       
        else if(opcode[0]=='+'){
            if(OPTAB.find(opcode.substr(1)) != OPTAB.end() && OPTAB[opcode.substr(1)].second==3){
                //handle label
                if(label != "") {
                    if(SYMTAB.find(label)!=SYMTAB.end()){
                        //error
                        pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                        pass1_output<<".--------ERROR--------------"<<endl;
                        ERROR_FLAG_PASS1=true;
                        continue;
                    }
                    else{
                        SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                           //relative label
                    }
                }
                //handle operand
                if(operand==""){
                }
                else if(operand=="*"){
                    //error
                    pass1_error<<"Invalid operand present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    LOCCTR+=4;
                    continue;
                }
                else if(operand=="=*"){
                    operand = "";
                    operand += "#";
                    operand += to_string(LOCCTR);

                }
                else if(operand[0]=='=' && SYMTAB.find(operand.substr(1)) != SYMTAB.end()){
                    int value=SYMTAB[operand.substr(1)].value;
                    operand = "";
                    operand += "#";
                    operand += to_string(value);

            
                }
                else if( (operand.size()>4) && ((operand.substr(0,3)=="=X\'"&& operand[operand.size()-1]=='\'') ||( operand.substr(0,3) == "=C\'" && operand[operand.size()-1]=='\''))){
                    // literal_count++;
                    LITTAB.insert({operand, 0});
                }
                
                pass1_output<<line_no<< " "<<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
                
                LOCCTR+=4;
            }
            else{
                //error
                pass1_error<<"Given opcode is not a format 4 instruction at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                continue;
            }
        }
        
        else if(OPTAB.find(opcode)!=OPTAB.end()){
            if(label!=""){
                if(SYMTAB.find(label)!=SYMTAB.end()){
                    //error
                    pass1_error<<"Duplicate symbol present at line no. "<<line_no<<endl;
                    pass1_output<<".--------ERROR--------------"<<endl;
                    ERROR_FLAG_PASS1=true;
                    continue;
                }
                else{
                    SYMTAB[label] = {label, current_block_no, LOCCTR, 1};                           //relative label
                }
            }

            //handle operand
            if(operand==""){
            }
            else if(operand=="*"){
                //error
                pass1_error<<"Invalid operand present at line no. "<<line_no<<endl;
                pass1_output<<".--------ERROR--------------"<<endl;
                ERROR_FLAG_PASS1=true;
                LOCCTR+=OPTAB[opcode].second;
                continue;
            }
            else if(operand=="=*"){
                operand = "";
                operand += "#";
                operand += to_string(LOCCTR);
            }
            else if(operand[0]=='=' && SYMTAB.find(operand.substr(1)) != SYMTAB.end()){
                int value=SYMTAB[operand.substr(1)].value;
                operand = "";
                operand += "#";
                operand += to_string(value);
        
            }
            else if( (operand.size()>4) && ((operand.substr(0,3)=="=X\'"&& operand[operand.size()-1]=='\'') ||( operand.substr(0,3) == "=C\'" && operand[operand.size()-1]=='\''))){
                // literal_count++;
                LITTAB.insert({operand, 0});
            }
            pass1_output<<line_no<< " "<<decimal_to_hex(LOCCTR,5)<<" "<< current_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<endl;
            
            LOCCTR+=OPTAB[opcode].second;
        }
       
        else{
            //error
            pass1_error<<"Invalid opcode present at line no. "<<line_no<<endl;
            pass1_output<<".--------ERROR--------------"<<endl;
            ERROR_FLAG_PASS1=true;
        }

    }               //END OF FILE

    pass1_output.close();
    pass1_error.close();
    pass1_input.close();

    //update BLOCKTABLE and program_length
    for(auto &i : BLOCKTABLE) {
        i.second.block_length = i.second.block_locctr;
    }

    vector<pair<int, pair<int, string>>> vec;
    for(auto i:BLOCKTABLE){
        vec.push_back({i.second.block_no, {i.second.block_length, i.second.block_name}});
    }
    sort(vec.begin(), vec.end());

    //update starting address of each block
    int sum=0;
    for(auto i:vec){
        BLOCKTABLE[i.second.second].start_address=sum;
        sum+=i.second.first;
    }
    program_length = sum;                   //total length of program  

    // update BLOCK_DATA
    for(auto data: vec){
        BLOCK_DATA.push_back({BLOCKTABLE[data.second.second].start_address, data.second.first});
    }

    return ERROR_FLAG_PASS1;
}

#endif
