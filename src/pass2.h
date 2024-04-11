#ifndef PASS2_H
#define PASS2_H

#include<iostream>
#include "./functions.h"
using namespace std;

bool pass2(){
    //initialising files
    ifstream pass2_in("./../data/intermediate.txt");
    ofstream pass2_list("./../data/listing_file.txt");
    ofstream pass2_obj("./../data/object_program.txt");
    ofstream pass2_err("./../data/error.txt");

    bool ERROR_FLAG_PASS2 = false;

    string line;
    string line_no, label, opcode, operand;
    int locctr, program_block_no;

    int BASE_REGISTER_VALUE = 0;
    bool BASE_RELATIVE_ADDRESSING = false;

    struct text_record current_text_record;
    current_text_record.start_address = 0;

    string current_object_code;
    int prev_block_no=0;

    //flsg to see if text record is crested inside RESW or RESB 
    //so when next instruction comes it should give starting address to text record
    bool prev_RESW_RESB = 0;

    vector<string> MODIFICATION_RECORDS;
    string end_record = "E^";


    //-----------------------------HEADER RECORD-------------------------------------------
    //handling start and use for header record
    string header_record="H^";
    while(getline(pass2_in,line)){
        //cout<<line<<endl;
        //neglecting comments and blank lines
        if(!pass2_line_scraper(line, line_no, locctr, program_block_no, label, opcode, operand)){
            pass2_list<<line<<endl;
            continue;
        }
        
        else if(opcode=="START"){
            int sz = 6-program_name.length();
            header_record+=program_name;
            while(sz--) {
                header_record += "_";
            }
            header_record+="^";

            string start_address = decimalToTwosComplement(stoi(operand, nullptr, 16),6);
            header_record += start_address;
            header_record+="^";
            string header_program_length = decimalToTwosComplement(program_length,6);
            header_record += header_program_length;
            pass2_obj<<header_record<<endl;
            pass2_list<<line<<endl;

            prev_block_no = program_block_no;

            break;
        }
        
        else if(opcode=="USE"){
            prev_block_no = program_block_no;
            pass2_list<<line<<endl;
            continue;
        }
    }
    
    
    //---------------TEXT RECORD, MODIFICATION RECORD, END RECORD-------------------------
    //handling further lines
    while(getline(pass2_in, line)) {
        current_object_code = "";

        //neglecting comments and blank lines
        if(!pass2_line_scraper(line, line_no, locctr, program_block_no, label, opcode, operand)){
            pass2_list<<line<<endl;
            continue;
        }

        if(opcode == "BASE") {
            pass2_list<<line<<endl;
            if(SYMTAB.find(operand) != SYMTAB.end()) {
                BASE_REGISTER_VALUE = SYMTAB[operand].value;
                BASE_RELATIVE_ADDRESSING = 1;
            }
            else if(check_operand_absolute(operand)) {
                BASE_REGISTER_VALUE = stoi(operand, nullptr, 10);
                BASE_RELATIVE_ADDRESSING = 1;
            }
            else {
                //handle expressions
                bool isValid = true;
                bool isRelative = false;
                int value = 0;
                handle_expression(operand, value, isValid, isRelative);
                if(isValid){
                    BASE_REGISTER_VALUE = value;
                    BASE_RELATIVE_ADDRESSING = 1;
                }
                else{
                    ERROR_FLAG_PASS2 = 1;
                    pass2_err<<"Invalid expression for BASE at "<<line_no<<endl;
                    continue;
                }
            }
        }

        else if(opcode == "NOBASE") {
            pass2_list<<line<<endl;
            BASE_RELATIVE_ADDRESSING = 0;
        }

        else if(opcode == "LTORG") {
            pass2_list<<line<<endl;
            continue;
        }

        else if(opcode == "EQU") {
            pass2_list<<line<<endl;
            continue;
        }

        else if(opcode == "ORG") {
            pass2_list<<line<<endl;
            continue;
        }

        else if(opcode == "BYTE") {

            if(operand[0]=='=') {
                if(operand[1]=='C') {
                    for(int i=3; i<operand.length()-1; i++) {
                        char ch = operand[i];
                        int x = ch;
                        current_object_code += decimalToTwosComplement(x, 2);
                    }
                }
                else {
                    for(int i=3; i<operand.length()-1; i++) {
                        current_object_code += operand[i];
                    }
                }
            }
            else {
                if(operand[0]=='C') {
                    for(int i=2; i<operand.length()-1; i++) {
                        char ch = operand[i];
                        int x = ch;
                        current_object_code += decimalToTwosComplement(x, 2);
                    }
                }
                else {
                    for(int i=2; i<operand.length()-1; i++) {
                        current_object_code += operand[i];
                    }
                }
            }

            if(prev_RESW_RESB) {
                prev_RESW_RESB = 0;
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
            }

            if((program_block_no != prev_block_no) || (current_text_record.object_code_length + current_object_code.length() > 60)) {
                //store current text record
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);
                //create new text record
                current_text_record = text_record();
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
                prev_block_no = program_block_no;
            }
            else {
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
            }

            //handle listing file
            if(operand[0]=='=') { //literal pool BYTE
                pass2_list<<line_no<<" "<<decimalToTwosComplement(locctr, 5)<<" "<<program_block_no<<" "<<"*"<<" "<<operand<<" "<<current_object_code<<endl;
            }
            else { // common BYTE
                pass2_list<<line_no<<" "<<decimalToTwosComplement(locctr, 5)<<" "<<program_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<" "<<current_object_code<<endl;
            }

        }
        
        else if(opcode == "WORD") {
            pass2_list<<line<<endl;
            current_object_code = decimalToTwosComplement(stoi(operand, nullptr, 10), 6);

            if(prev_RESW_RESB) {
                prev_RESW_RESB = 0;
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
            }

            if((program_block_no != prev_block_no) || (current_text_record.object_code_length + current_object_code.length() > 60)) {
                //store current text record
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);
                //create new text record
                current_text_record = text_record();
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
                prev_block_no = program_block_no;
            }
            else {
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
            }
        }
       
        else if(opcode == "RESW" || opcode == "RESB") {
            pass2_list<<line<<endl;
            if(current_text_record.object_code_length != 0) {
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);

                current_text_record = text_record();
            }
            prev_RESW_RESB = 1;
        }
        
        else if(opcode == "USE") {
            pass2_list<<line<<endl;
            continue;
        }

        // format-4
        else if(opcode[0] == '+') {
            
            int ni = 3;
            bool need_modification_record = 0;
            int xbpe = 1;
            string opcode_value = OPTAB[opcode.substr(1)].first;
            
            //handle x bit
            if(operand.size() > 2 && operand.substr(operand.size()-2) == ",X") {
                xbpe += 8;
                operand = operand.substr(0, operand.size()-2);
            }
            
            //handle ni bit
            if(operand[0] == '#') {
                ni = 1;
                operand = operand.substr(1);
            }
            else if(operand[0] == '@') {
                ni = 2;
                operand = operand.substr(1);
            }
            
            //handle operand
            if(operand.size() > 0) {
                if(opcode != "RSUB") {
                    bool isValid = true;
                    bool isRelative = false;
                    int value = 0;
                    handle_expression(operand, value, isValid, isRelative);
                    if(isValid) {
                        // is an expression
                        if(isRelative) {
                            need_modification_record = 1;
                        }
                    }
                    else if(check_operand_absolute(operand)) {
                        value = stoi(operand, nullptr, 10);
                    }
                    else if(SYMTAB.find(operand) != SYMTAB.end()) {
                        if(SYMTAB[operand].isValid == 1) {
                            need_modification_record = 1;
                        }
                        int symbol_block_no = SYMTAB[operand].block_no;

                        // if label is relative then value is relative so need to add starting address of block
                        if(need_modification_record) value = SYMTAB[operand].value+BLOCK_DATA[symbol_block_no].start_address;
                        //if label is absolute then value is absolute hence no need to add starting address of block
                        else value = SYMTAB[operand].value;
                    }
                    else {
                        ERROR_FLAG_PASS2 = 1;
                        pass2_err<<"Invalid operand for format-4 instruction at "<<line_no<<endl;
                        continue;
                    }

                    int tmp = stoi(opcode_value, nullptr, 16);
                    current_object_code = decimalToTwosComplement(tmp + ni, 2) + decimalToTwosComplement(xbpe, 1) + decimalToTwosComplement(value, 5);
                }
                else {
                    ERROR_FLAG_PASS2 = 1;
                    pass2_err<<"Invalid operand for format-4 instruction at "<<line_no<<endl;
                    continue;
                }
            }
            else {
                if(opcode == "RSUB") {
                    current_object_code = "4F100000";
                }
                else {
                    ERROR_FLAG_PASS2 = 1;
                    pass2_err<<"Invalid operand for format-4 instruction at "<<line_no<<endl;
                    continue;
                }
            }

            if(prev_RESW_RESB) {
                prev_RESW_RESB = 0;
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
            }

            //handle text record
            if((program_block_no != prev_block_no) || (current_text_record.object_code_length + current_object_code.length() > 60)) {
                //store current text record
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);
                //create new text record
                current_text_record = text_record();
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
                prev_block_no = program_block_no;
            }
            else {
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
            }

            //handle modification record
            if(need_modification_record) {
                string modification_record = "M^";
                int address = BLOCK_DATA[program_block_no].start_address + locctr + 1;
                modification_record += decimalToTwosComplement(address, 6);
                modification_record += "^05";
                MODIFICATION_RECORDS.push_back(modification_record);
            }

            //handle listing file
            pass2_list<<line_no<<" "<<decimalToTwosComplement(locctr, 5)<<" "<<program_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<" "<<current_object_code<<endl;

        }

        // format-1,2,3
        else if(OPTAB.find(opcode) != OPTAB.end()) {
      
            int format = OPTAB[opcode].second;
            int zero = 0;
            string opcode_value = OPTAB[opcode].first;
            
            //opcode has format 1
            if(format == 1) {
                current_object_code = opcode_value;            
            }
            //opcode has format 2
            else if(format == 2) {
                bool error_flag = 0;
                string str = handleFormat2(opcode, operand, error_flag);
                if(error_flag) {
                    ERROR_FLAG_PASS2 = 1;
                    pass2_err<<"Invalid operand for format-2 instruction at "<<line_no<<endl;
                    continue;
                }
    
                current_object_code = opcode_value + str;
            }
            //opcode has format 3
            else {
                int ni = 3;
                int xbpe = 0;
                //handle x bit
                if(operand.size() > 2 && operand.substr(operand.size()-2) == ",X") {
                    xbpe += 8;
                    operand = operand.substr(0, operand.size()-2);
                }
                //handle ni bit
                if(operand[0] == '#') {
                    ni = 1;
                    operand = operand.substr(1);
                }
                else if(operand[0] == '@') {
                    ni = 2;
                    operand = operand.substr(1);
                }

                //handle operand
                if(operand.size() > 0) {
                    if(opcode != "RSUB") {
                        bool isValid = true;
                        bool isRelative = false;
                        int value = 0;
                        handle_expression(operand, value, isValid, isRelative);
                        if(isValid) {
                            // is an expression
                        }
                        else if(check_operand_absolute(operand)) {
                            value = stoi(operand, nullptr, 10);
                        }
                        else if(SYMTAB.find(operand) != SYMTAB.end()) {
                            if(SYMTAB[operand].isValid == 1) {
                                isRelative = 1;
                            }
                            int symbol_block_no = SYMTAB[operand].block_no;
                            value = SYMTAB[operand].value+BLOCK_DATA[symbol_block_no].start_address;
                        }
                        else {
                            ERROR_FLAG_PASS2 = 1;
                            pass2_err<<"Invalid operand for format-3 instruction at "<<line_no<<endl;
                            continue;
                        }
                        //handle direct or relative addressing
                        if(isRelative) {
                            int PC_value = locctr + 3;
                            int displacement = value - PC_value;
                            if(displacement >= -2048 && displacement <= 2047) {
                                xbpe += 2;
                                value = displacement;
                            }
                            else {
                                if(BASE_RELATIVE_ADDRESSING) {
                                    displacement = value - BASE_REGISTER_VALUE;
                                    if(displacement >= 0 && displacement <= 4095) {
                                        xbpe += 4;
                                        value = displacement;
                                    }
                                    else {
                                        ERROR_FLAG_PASS2 = 1;
                                        pass2_err<<"Displacement is out of bounds for both PC-relative and BASE-relative addressing in format-3 instruction at "<<line_no<<endl;
                                        continue;
                                    }
                                }
                                else {
                                    ERROR_FLAG_PASS2 = 1;
                                    pass2_err<<"Displacement is out of bounds for PC-relative addressing and BASE-relative addressing is not enabled in format-3 instruction at "<<line_no<<endl;
                                    continue;
                                }
                            }
                        }

                        int tmp = stoi(opcode_value, nullptr, 16);
                        current_object_code = decimalToTwosComplement(tmp + ni, 2) + decimalToTwosComplement(xbpe, 1) + decimalToTwosComplement(value, 3);
                    }
                    else {
                        ERROR_FLAG_PASS2 = 1;
                        pass2_err<<"Operand given for RSUB in format-3 instruction at "<<line_no<<endl;
                        continue;
                    }
                }
                else {
                    if(opcode == "RSUB") {
                        current_object_code = "4F0000";
                    }
                    else {
                        ERROR_FLAG_PASS2 = 1;
                        pass2_err<<opcode<<" requires operand for format-3 instruction at "<<line_no<<endl;
                        continue;
                    }
                }

            }

            if(prev_RESW_RESB) {
                prev_RESW_RESB = 0;
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
            }

            // handle text record
            if((program_block_no != prev_block_no) || (current_text_record.object_code_length + current_object_code.length() > 60)) {
                //store current text record
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);
                //create new text record
                current_text_record = text_record();
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;

                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
                prev_block_no = program_block_no;
            }
            else {
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
            }

            //handle listing file
            pass2_list<<line_no<<" "<<decimalToTwosComplement(locctr, 5)<<" "<<program_block_no<<" "<<label<<" "<<opcode<<" "<<operand<<" "<<current_object_code<<endl;
        }
        
        else if(opcode == "END") {

            if(prev_RESW_RESB) {
                prev_RESW_RESB = 0;
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
            }

            if((program_block_no != prev_block_no) || (current_text_record.object_code_length + current_object_code.length() > 60)) {
                //store current text record
                current_text_record.length = current_text_record.object_code_length/2;
                TEXT_RECORDS.push_back(current_text_record);
                //create new text record
                current_text_record = text_record();
                current_text_record.start_address = BLOCK_DATA[program_block_no].start_address + locctr;
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
                prev_block_no = program_block_no;
            }
            else {
                current_text_record.object_code += "^" + current_object_code;
                current_text_record.object_code_length += current_object_code.length();
            }

            //write end record

            if(operand != "") {
                if(SYMTAB.find(operand) != SYMTAB.end()) {
                    end_record += decimalToTwosComplement(SYMTAB[operand].value, 6);
                }
                else {
                    ERROR_FLAG_PASS2 = 1;
                    pass2_err<<"Undefined label present at "<<line_no<<endl;
                    continue;
                }
            }
            else {
                end_record += decimalToTwosComplement(BLOCK_DATA[0].start_address, 6);
            }

            //handle listing file
            pass2_list<<"     "<<label <<opcode<<" "<<operand<<" "<<current_object_code<<endl;

            continue;
        }
        
        else {
            ERROR_FLAG_PASS2 = 1;
            pass2_err<<"Invalid opcode present at "<<line_no<<endl;
            continue;
        }
    }           //END OF INTERMEDIATE FILE

    // push last text record if exist
    if(current_object_code.size() > 0) {
        current_text_record.length = current_text_record.object_code_length/2;
        TEXT_RECORDS.push_back(current_text_record);
    }

    //write text records
    for(auto i:TEXT_RECORDS) {
        string text_record = "T^";
        text_record += decimalToTwosComplement(i.start_address, 6);
        text_record += "^";
        text_record += decimalToTwosComplement(i.length, 2);
        text_record += i.object_code;
        pass2_obj<<text_record<<endl;
    }

    // write modification records
    for(auto i:MODIFICATION_RECORDS) {
        pass2_obj<<i<<endl;
    }

    //write end record
    pass2_obj<<end_record<<endl;

    return ERROR_FLAG_PASS2;
}

#endif
