#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <iostream>
#include <scanner.h>
#include <parser.h>
#include <fstream>
#include <cctype>

using namespace std;

extern int yylex(void);
extern char *yytext;
extern int SourceLine;
extern FILE *yyin;
extern int yyparse(void);

string toLowercase(string n){
    string auxString;
    if( n != ""){
        for (char c: n){
            auxString+=tolower(c);
        }
        return auxString;
    }
    return n;
}

void parse(){
    system("clear");
    while(true){

        string inputCommand;
        std::cout << "\u001B[36m" << "[INPUT] :" << "\x1B[0m";
        std::getline(std::cin, inputCommand);

        if(inputCommand == "clear"){
            system("clear");
        }
        else if(inputCommand=="exit"){
            system("clear");
            break;
        }
        else if(inputCommand.length() ==0){

        }
        else if(toLowercase(inputCommand.substr(0,4))== "exec"){

            string path = inputCommand.substr(inputCommand.find("=")+1, inputCommand.length() - 1);
            if(path[0]== '\"'){
                path = path.substr(1,path.length()-2);
            }
            ifstream fe(path);
            string input;
            string linea;
            while(getline(fe, linea)){
                input +=linea;
                input += '\n';
            }
            input = toLowercase(input);
            SourceLine = 1;
            YY_BUFFER_STATE bufferState = yy_scan_string(input.c_str());
            if(yyparse()==0){
                cout<<"\u001B[32m" << "---Ejecucion de Script Terminada---"<< "\x1B[0m"<<endl;
            }
            else{
                cout<<"\u001B[31m"<< "---Errores sintacticos en el script---"<< "\x1B[0m"<<endl;
            }
            yy_delete_buffer(bufferState);
        }
        else{
            SourceLine = 1;
            inputCommand = toLowercase(inputCommand);
            inputCommand+='\n';
            YY_BUFFER_STATE bufferState = yy_scan_string(inputCommand.c_str());
            if(yyparse()==0){
                cout<< "\u001B[32m"<< "---Ejecucion de Comando Terminada---"<< "\x1B[0m"<<endl;
            }
            else{
                cout<<"\u001B[31m"<< "---Errores sintacticos en el comando---"<<endl;
            }
            yy_delete_buffer(bufferState);
        }
    }
}

int main(){
    parse();
}