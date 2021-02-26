#ifndef RMDISK_H
#define RMDISK_H

#endif /// RMDISK_H

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <cctype>
#include <libgen.h>

using namespace std;

class RMDISK_{
private:
    string path;/// Path del disco
    bool statusFlag;// Indica si ha errores con el comando.
public:

    /**
     * Constructor
    */
    RMDISK_():path(""){};

    /**
     * Borra el archivo
    */
    void deleteDisk();

    /**
     * Verifica que los parametros sean validos 
    */
    void setStatus();

    /**
     * Setter del path
     * @param ruta: path del disco a eliminar
    */
    void setPath(char* ruta);
};

void RMDISK_::deleteDisk(){
    setStatus();
    if(this->statusFlag){
        
        string auxPath = this->path;
        FILE *file;
        file=fopen(path.c_str(),"r");
        if(file != NULL){
            string command = "rm '" + string(this->path) + "'";
            system(command.c_str());
            cout << "[OK] Disco eliminado exitosamente" << endl;
        }
        else{
            cout << "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
        }
    }
    else{
        cout << "\u001B[31m" << "[BAD PARAM] Falta parametro Path" << "\x1B[0m" << endl;
    }
}

void RMDISK_::setStatus(){
    this->statusFlag = false;
    if(this->path[0] == '/'){
        if(string(this->path) == "/"){
            cout << "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
        }
        else{
            this->statusFlag = true;
        }
    }
    else{
        cout << "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
    }
}

void RMDISK_::setPath(char * value){
    if(value[0]=='\"'){
        this->path = value;
        path = path.substr(1, path.length()-2);
    }
    else{
        this->path = value;
    }
}

