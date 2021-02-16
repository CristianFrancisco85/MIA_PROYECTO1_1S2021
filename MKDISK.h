#ifndef MKDISK_H
#define MKDISK_H
#endif /// MKDISK_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <libgen.h>
#include <fstream>
#include <chrono>
#include <ctime>
#include <structs.h>

using namespace std;

class MKDISK_{
private:
    int size;//Atributo size
    char fit[2];// Tipo de fit
    string unit;// Tipo de Unidades
    char path[255];// Path del disco
    bool statusFlag;// Indica si ha errores con el comando.
    MBR masterBootRecord;// MasterBootRecord

public:

    /**
     * Constructor
    */
    MKDISK_(){
        //Fit Predeterminado Primer Ajuste
        this->fit[0]='F';
        this->fit[1]='F';
        //Unidades Predeterminadas MegaBytes
        this->unit="m";

    };

    /**
     * Size Setter
     * @param size: Cantidad de unidades que ocupará el disco.
    */
    void setSize(char* size);

    /**
     * Fit Setter
     * @param fit: Tipo de ajuste del disco.
    */
    void setFit(char* fit);

    /**
     * Units Setter
     * @param unit: Indica si son megabytes, kilobytes
    */
    void setUnit(char* unit);

    /**
     * path Setter
     * @param path: Path del disco
    */
    void setPath(char* path);

    /**
     * Verifica si los parametros son validos
    */
    void setStatus();

    /**
     * Getter del fit
     * @return Letra del fit
    */
    char getFit();

    /**
     * Getter del size
     * @return Tamo del disco en kilobytes
    */
    int getSize();

    /**
     * Getter de las unidades
     * @return Multiplcador de las unidades
    */
    int getUnit();

    /**
     * Crea el disco
    */
    int createDisk();
    
    /**
     * Crea y configura el Master Boot Record
    */
    void configureMaster();

};

void MKDISK_::setSize(char *value){
    this->size =atoi(value);
}

void MKDISK_::setFit(char *value){
    strcpy(this->fit, value);
    this->fit[0] = toupper(fit[0]);
    this->fit[1] = toupper(fit[1]);
}

void MKDISK_::setUnit(char *value){
    this->unit=value;
}

void MKDISK_::setPath(char *value){
    if(value[0]=='\"'){
        string aux = value;
        aux = aux.substr(1,aux.length()-2);
        strcpy(this->path, aux.c_str());
    }
    else{
        strcpy(this->path, value);
    }
}

void MKDISK_::setStatus(){
    this->statusFlag = false;
    if(this->path[0] == '/' && this->size > 0){
        if(string(this->path) != "/"){
            this->statusFlag = true;
        }
        else{
            cout << "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
        }
    }
    else{
        if(this->path[0] != '/'){
            cout << "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
        }
        else{
            cout << "\u001B[31m" << "[BAD PARAM] Size no valido" << "\x1B[0m" << endl;
        }
        
    }
}

int MKDISK_::getSize(){
    return this->size*this->getUnit();
}

int MKDISK_::getUnit(){
    if(this->unit=="m"){
        return 1024;
    }
    return 1;
}

int MKDISK_::createDisk(){
    setStatus();
    if(this->statusFlag){
        string pathSinName = this->path;
        string pathConName = this->path;
        char* ruta_aux = this->path;

        const size_t lastSlash = pathSinName.find_last_of("/");
        if (string::npos != lastSlash){
            pathSinName = pathSinName.substr(0, lastSlash);
        }

        //Si crea directorio si es necesario
        string comando = "sudo mkdir -p \'";
        comando+= dirname(path);
        comando+= '\'';
        system(comando.c_str());

        //Se otorga permiso
        comando = "sudo chmod -R 777 \'";
        comando+= dirname(ruta_aux);
        comando += '\'';
        system(comando.c_str());
        
        //Se abre un stream de datos en el directorio
        FILE *f;
        f = fopen(pathConName.c_str(),"wb");
        if(f == NULL){
            return 0;
        }
        //Se configura el Master Boot Record
        configureMaster();
        //Se graba MBR
        fseek(f,0,SEEK_SET);
        fwrite(&this->masterBootRecord, sizeof (MBR),1,f);

        //Se escribe disco
        int diskSize = getSize();
        for(int i = 0; i < diskSize; ++i){
            char bloqueDatos[1024];
            //Contenido Inicial
            if(i==0){
                bloqueDatos[0] = '0';
            }
            fwrite(&bloqueDatos, sizeof(bloqueDatos), 1, f);
        }

        //Se lee MBR
        fseek(f,0,SEEK_SET);
        fread(&masterBootRecord,sizeof (MBR),1,f);
        fclose(f);

        cout << "[OK] Disco creado exitosamente" << endl;
        //cout << masterBootRecord.mbr_tamano<<endl;
        return 1;
    }
    return 0;
}

void MKDISK_::configureMaster(){
    //Tamaño
    masterBootRecord.mbr_tamano = this->getSize()*1024;

    ///Fecha y hora
    time_t now = time(0);
    masterBootRecord.mbr_fecha_creacion = now;

    /// Firma del disco
    srand(time(NULL));
    int num = rand();
    num=1+rand()%(100000-1);
    masterBootRecord.mbr_disk_signature = num;

    /// Guardando tipo de fit
    masterBootRecord.disk_fit = getFit();
}

char MKDISK_::getFit(){
    return toupper(this->fit[0]);
}

