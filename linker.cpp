#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>
#include <string>
#include <bits/stdc++.h>
using namespace std;

int linenum = 0;
int lineoffset = 0;
//string fileTotake;
FILE* file;
static bool need_new_line = true;
string errstring;
static int linelen = 0;
static char linebuf[1024];
const char* DELIM = " \t\n";
static bool eofFlag = false;
map<string, int> symtable;
map<string, bool> def_used_table; 
map<string, int> module_map;
map<string, bool> multi_def_map;

void __parseerror(int errcode) {
 static char* errstr[] = {
 "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
 "SYM_EXPECTED", // Symbol Expected
 "ADDR_EXPECTED", // Addressing Expected which is A/E/I/R
 "SYM_TOO_LONG", // Symbol Name is too long
 "TOO_MANY_DEF_IN_MODULE", // > 16
 "TOO_MANY_USE_IN_MODULE", // > 16
 "TOO_MANY_INSTR", // total num_instr exceeds memory size (512)
 };
 //cout  << errstr[errcode] << endl;
 printf("Parse Error line %d offset %d: %s\n", linenum,lineoffset,errstr[errcode]);
 exit(-1);
}

void __instrerror(int errcode){
    
    if(errcode<=5){
    static char* errarr[] = {
        "Error: Absolute address exceeds machine size; zero used",
        "Error: Relative address exceeds module size; zero used",
        "Error: External address exceeds length of uselist; treated as immediate",
        "Error: This variable is multiple times defined; first value used",
        "Error: Illegal immediate value; treated as 9999",
        "Error: Illegal opcode; treated as 9999",
    };
    printf(" %s",errarr[errcode]);
    //cout  << "\t" << errarr[errcode] << endl;
    }
    else {
        //cout << errstring << "is not defined; zero used";
        // char* errch;
        // strcpy(errch,errstring.c_str());
        // //cout << errch << endl;

        //cout << errstring << endl;
        //char output[100];
        cout << " Error: " << errstring << " is not defined; zero used" << endl;
        //cout << "Error: is not defined; zero used" << endl;
        //sprintf( output,"Error: %s is not defined; zero used\n",errstring.c_str());
        //cout << output << endl;
    }

}

char * getToken()
{
    while(1) {
        if (need_new_line) {
            //linelen = strlen(linebuf);
            //cout << "linelen " <<linelen << endl; 
            lineoffset = 1;
            if (fgets(linebuf, 1024, file) == NULL) {
                eofFlag = true;
                //lineoffset = linelen + lineoffset;
               return NULL;
               } // EOF reached
            if((strcmp(linebuf,"\n") == 0)||(strcmp(linebuf,"\r\n") == 0)||(strcmp(linebuf,"\0") == 0)){
               linenum ++;
               continue; 
               } // if blank line go to next line;
            linenum++;
            char* tok = strtok(linebuf," \t\n");
            if (tok == NULL) // no tokens in line   
                continue; // we try with next line
            need_new_line = false;
            lineoffset = tok - linebuf + 1;
            //linelen = strlen(tok);
            return tok;
        }
        char* tok = strtok(NULL," \t\n");
        lineoffset = tok - linebuf + 1;
        //linelen = strlen(tok);
        if (tok != NULL) // found a token
            return tok;
        need_new_line = true;
       // we try with next line
    }
  
}

bool validSym(char * str){
    // printf("in valid symbool\n");
    if (strlen(str)>16){
        // printf("hello\n");
        //printf("Symbol greater than 16 characters");
        __parseerror(3);
        return false;
    }
    else{
        if(isalpha(str[0])){
            // printf("hi\n");
            // cout << strlen(str) << endl;
            for (int i=1; i<(strlen(str)-1); i++){
                // cout << str[i] << endl;
                if (isalnum(str[i]))
                    continue;
                else
                        return false;
            }
            return true;
        }
        
    }
}

void printSymTable(){
            printf("Symbol Table\n");
            map<string, int>::iterator itr;
            for (itr = symtable.begin(); itr != symtable.end(); ++itr) {
             cout << itr->first << "="
              << itr->second ;
            if(multi_def_map[itr->first]){
                printf(" Error: This variable is multiple times defined; first value used");
            }
            printf("\n");
           }
        }

string convertToString(char* a)
{   
    int size = strlen(a);
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

bool checkint(char * str){
    for (int i=0; i<(strlen(str)-1); i++){
        if (isdigit(str[i]))
            continue;
        else
            return false;

    }
    if(str[strlen[str]-1]=='\n' || str[strlen[str]-1]=='\0')
    return true;
    else
    return false;

}

int readNum(){
    char* c = getToken();
    if(c == NULL) {
        
        //eofFlag = true;
        // __parseerror(0);
    }
    else if(checkint(c)){
        return atoi(c);
    }
    else
        __parseerror(0);
}

char * readSym(){
    char * c = getToken();
    //cout << c << endl;
    if(c == NULL) {  
        // printf("hey");
        //eofFlag = true;
        // __parseerror(1);
    }
    if (validSym(c)){
        return c;
    }
    else {
        __parseerror(1);
    }
}

char* readinstr(){
    char * c = getToken();
    if(c == NULL){
       __parseerror(2); 
    }
    if(strlen(c) != 1){
        __parseerror(2);
    }
    else{
        if (c[0] == 'A'|| c[0] == 'I' || c[0] == 'E' || c[0] == 'R'){
            return c;
        }
        else{
            __parseerror(2);
        }
    }

}

int* CheckRule5(int instrcount, int module,string* symarray,int* addrarray, int defcount,int counter){
    for(int i = 0; i<defcount; i++){
        if(addrarray[i]>= instrcount){
            printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n",module,(symarray[i]).c_str(),addrarray[i],instrcount-1);
            addrarray[i] = counter;
        }
        else{
            addrarray[i] = addrarray[i] + counter;
        }
        
    }
    return addrarray;    

}

// bool check_opcode(int address){
//     int opcode = address/1000;
//     if(opcode<10)
//        return true;
//     else
//        return false;
// }

void check_used_symbols(bool* usedarr,string* usearr,int module,int usecount){
    for(int i = 0; i<usecount; i++){
        //cout << usedarr[i] << endl;
        //cout << usearr[i] << endl;
        if(!(usedarr[i])){
            cout << "Warning: Module " << module << ": " << usearr[i] << " appeared in the uselist but was not actually used\n";
            //printf("Warning: Module %d: %s appeared in the uselist but was not actually used",module,usearr[i].c_str());
            //cout << "\tWarning: Module" << module << ":" << usearr[i];
        }
    }
}

bool not_present_in_array(string str, string* symarray, int size){
    for(int i = 0; i<size; i++){
        if(symarray[i]==str){
            return false;
        }
    }
    return true;
      

}

bool check_multi_def_sym(char* sym, string* symarray, int size){
    //printf("Hi");
    if ((symtable.find(convertToString(sym))==symtable.end()) && not_present_in_array(convertToString(sym),symarray,size)){
        return false;
    }
    else 
      return true;
}

void initialise_def_used_table(){
    for(map<string, int>::iterator it = symtable.begin(); it != symtable.end(); ++it) {
        def_used_table.insert({it->first, false});
    }

}

void check_defined_symbols(){
    map<string, bool>::iterator itr;
    for (itr = def_used_table.begin(); itr != def_used_table.end(); ++itr) {
        if(!(itr->second)){
            printf("Warning: Module %d: %s was defined but never used\n",module_map[itr->first],itr->first.c_str());
        }
        
    }

}

void AppendSymModTable( string* symarray, int newdefarr[], int defcount, int module){
    for (int i = 0; i < defcount; i++){
        symtable.insert({symarray[i], newdefarr[i]});
        module_map.insert({symarray[i], module});
    }
}

void pass1(){
    int Module = 0; 
    int counter = 0;
    //map<string, int> symtable;
    while (!eofFlag){
        Module++;
        int defcount = readNum();
        if (eofFlag) break;
        //cout << defcount << endl;
        if(defcount>16){
            __parseerror(4);
        }
        string symarray[defcount];
        int adrarray[defcount];
        for (int i=0;i<defcount;i++) {
            char* sym = readSym();
            int val = readNum();
            // cout << sym << endl;
            bool check = check_multi_def_sym(sym,symarray,i);
            // cout << check << endl;
            if(!check){
            symarray[i] = convertToString(sym);
            adrarray[i] = val;
            }
            else{
               multi_def_map[convertToString(sym)] = true;
               i = i-1;
               defcount = defcount-1;
               //cout << "i" << i;
               //cout << "defcount" <<defcount ;
            }
        }
        // cout << "Printing Sym Array" << endl;
        // for (int i= 0 ; i< defcount; i++){
        //     cout << symarray[i] << endl;
        // }
        // cout << "Printing Addr Array" << endl;
        // for (int i= 0 ; i< defcount; i++){
        //     cout << adrarray[i] << endl;
        // }


        //use line
        int usecount = readNum();
        if(usecount>16){
            __parseerror(5);
        }
        //cout << usecount << endl;
        for (int i=0;i<usecount;i++) {
        char* sym = readSym();
        //cout << sym << endl;
        }
        // instruction line 
        int instrcount = readNum();
        if(instrcount>=511){
            __parseerror(6);
        }

        int* newdefarr = CheckRule5(instrcount, Module, symarray, adrarray, defcount, counter);

        AppendSymModTable(symarray, newdefarr, defcount,Module);
        //printf("instruction count %d",instrcount);
        for (int i=0;i<instrcount;i++) {
        char* addressmode = readinstr();
        int operand = readNum();
        //printf("addressmode is %s\n", addressmode);
        //cout << operand << endl;
        }
        
        counter = counter + instrcount;
    }  
    
}

void pass2(){

    int counter = 0;
    //cout << eofFlag << endl;
    int mmcounter = -1;
    int Module = 0;
    printf("\n");
    printf("Memory Map\n");
    while(!eofFlag){
    Module ++;  
    if (eofFlag) break;
    int defcount = readNum();
    for (int i=0;i<defcount;i++) {
            char* sym = readSym();
            //cout << sym << endl;
            int val = readNum();
            // cout << val << endl;
        }
    // printf("Hi");
    int usecount = readNum();
    string usearr[usecount];
    bool usedarr[usecount];
    for (int i = 0 ; i<usecount; i++){
        usedarr[i] = false;
    }
    for (int i=0;i<usecount;i++) {
        char* sym = readSym();
        string str = convertToString(sym);
        str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
        usearr[i] = str;
        //cout << usearr[i] << endl;
        //cout << "sym***" << symtable[usearr[i]] << endl;
        }  

    // for (int i = 0; i < usecount; i++){
    //     //printf("Use Array outside");
    //     cout << usearr[i] << "\n";   
    // }
    int instrcount = readNum();
    //printf("instrcount is %d",instrcount);
    
    
    for (int i=0;i<instrcount;i++) {
        char* instrmode = readinstr();
        int address = readNum();
        int mmaddress = 0;
        int errcode = -1;
        //string errstring = "";
        int rem = address%1000;
        int opcode = address/1000;
        if(instrmode[0] == 'R'){
            if(opcode>=10){
                mmaddress = 9999;
                errcode = 5;
            }
            else if(rem>=instrcount){
                mmaddress = address - rem + counter;
                errcode = 1;
            }
            else{
            mmaddress = address + counter;
            }
        }
        else if(instrmode[0] == 'E'){
            if(opcode>=10){
                mmaddress = 9999;
                errcode = 5;
            }
            else if(rem<usecount){
            string AssSym = usearr[rem];
            // cout << "AssSym"<<AssSym <<endl;
            // cout << "printing comparison";
            //cout << AssSym.compare("X21");
            //printSymTable();
            // auto itr = symtable.find(AssSym);
            // cout << itr->first << "and" << itr->second << endl;
            
            //cout << symtable[AssSym] << endl;
            usedarr[rem] = true;
            if(!symtable[AssSym]){
                //printf("Hi");
                mmaddress = address - rem;
                errcode = 6;
                errstring = AssSym;
            }
            else {  
                
                mmaddress = (address - rem) + symtable[AssSym];
                def_used_table[AssSym] = true;
            }
            }
            else{
                mmaddress = address;
                errcode = 2;
            }
 
        }
        else if(instrmode[0] == 'I'){
            if(opcode>=10){
            mmaddress = 9999;
            errcode = 4;
            }
            else mmaddress = address;   
        }
        else if(instrmode[0] == 'A'){
            int opcode = address/1000;
            int rem = address%1000;
            if(opcode>=10){
                mmaddress = 9999;
                errcode = 5;
            }
            else if(rem>511){
                mmaddress = address - rem;
                errcode = 0;
            }
            else
            mmaddress = address;
        }
        else{
            printf("incorrect address mode");
        }
        mmcounter++;
        printf("%03d: %04d",mmcounter, mmaddress);
        if(errcode!=-1){
        __instrerror(errcode);
        }
        printf("\n");
        }
        check_used_symbols(usedarr,usearr,Module,usecount);
        counter = counter + instrcount;
        
    }
    printf("\n");
    check_defined_symbols();
}



int main(int argc, char *argv[])
{
    file = fopen(argv[1], "r");
    pass1();
    printSymTable();
    eofFlag = false;
    fseek(file, 0, SEEK_SET);
    initialise_def_used_table();
    pass2();
}

