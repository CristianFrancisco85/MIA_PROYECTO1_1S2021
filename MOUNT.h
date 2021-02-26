#pragma once
#ifndef MOUNT_H
#define MOUNT_H

#endif /// MOUNT_H

#include <string.h>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <structs.h>
#include <list> 
#include <iterator> 
#include "parser.h"

using namespace std;

class MOUNT_{
private:
    string path;//Path del disco
    string name;//Nombre de la particion
    string id;//ID en la lista de particiones montadas
    char letter;//Letra del disco
    int number;//Numero de particion por disco
    bool statusFlag;// Indica si ha errores con el comando.
public:
    /**
     * Constructor
    */
    MOUNT_(){
        this->path="";
        this->name="";
        this->statusFlag=false;
    }

    /**
     * Setter del Path
     * @param c: path del disco para montar
    */
    void setPath(char *value);

    /**
     * Setter del Name
     * @param c: Nombre de la particion
    */
    void setName(char *c);

    /**
     * Setter del ID
    */
    void setId();

    /**
     * Setter de la letra del disco a la cual pertenece la particion
     * @param c: Letra del disco
    */
    void setLetter(char c);

    /**
     * Getter del nombre de la particion
    */
    string getName();

    /**
     * Getter de la ruta del disco de la partición.
    */
    string getPath();

    /**
     * Getter de la letra del disco a la cual pertenece la partición
    */
    char getLetter();

    /**
     * Getter del número de partición montada de un disco.
    */
    int getNumber();

    /**
     * Getter del ID en formato 97+ Numero + Letra Disco
    */
    string getId();

    /*
     * Configura del ID de la partición
    */
    string createId();

    /**
     * Verifica si los parametros son validos
    */
    void setStatus();

    /**
     * Comienza la operacion
    */
    void beginToMount();

    /**
     * Busca una particion primaria y regresa su indice
    */
    int findPartitionIndex();


    /**
     * Busca una particion logica y regresa su indice
    */
    int findLogicPartitionStart();

    /*
     * Monta la partición.
    */
    void mountPartition();

    /*
     * Verifica si la partición esta montada
    */
    bool isMounted();
};

extern list<MOUNT_> *mounted;

void MOUNT_::setId(){
    this->id = this->createId();
}

void MOUNT_::setPath(char *value){
    path = value;
    if(path[0] == '\"'){
        path = path.substr(1, path.size()-2);
    }   
}

void MOUNT_::setName(char *c){
    name = c;
    if(name[0] == '\"'){
        name = name.substr(1, name.size()-2);
    }     
}

void MOUNT_::setLetter(char c){
    this->letter = c;
}

void MOUNT_::setStatus(){

    if(this->path !="" && this->name != ""){
        statusFlag = true;
    }
    if(this->path == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
    }
    if(this->name == ""){
        cout << "\u001B[31m" << "[BAD PARAM] Name no valido" << "\x1B[0m" << endl;
    }
}

string MOUNT_::getName(){
    return this->name;
}

string MOUNT_::getPath(){
    return this->path;
}

char MOUNT_::getLetter(){

    char auxChar = 'a';
    list<MOUNT_>::iterator i;
    for(i = mounted->begin(); i!=mounted->end(); i++ ){
        //Si son del mismo disco
        if(i->path == this->path){
            auxChar++;
        }
    }
    this->letter = auxChar;
    return auxChar;
}

int MOUNT_::getNumber(){
    if(mounted->size() == 0){
        this->number=1;
        return 1;
    }
    else{
        int auxInt = 1;
        list<MOUNT_>::iterator i;
        list<string> *marcadas = new list<string>();
        for(i=mounted->begin(); i!=mounted->end(); i++){
            list<string>::iterator findIter;
            findIter = find(marcadas->begin(), marcadas->end(),i->path);

            if(findIter == marcadas->end() && i->path != this->path){
                marcadas->push_back(i->path);
                auxInt++;
            }
            if(i->path == this->path){
                this->number=auxInt;
                return auxInt;
            }
        }
        this->number=auxInt;
        return auxInt;
    }
}

string MOUNT_::getId(){
    return this->id;
}

string MOUNT_::createId(){
    string aux(1,getLetter());
    return "97"+to_string(this->getNumber())+aux;
}

bool MOUNT_::isMounted(){
    list<MOUNT_>::iterator i;
    for(i = mounted->begin();i != mounted->end();i++){
        if(i->name == this->name && i->path == this->path){
            return  true;
        }
    }
    return  false;
}

void MOUNT_::beginToMount(){
    setStatus();
    if(this->statusFlag){
        setId();
        cout<<"Montando particion "<< this->getId() <<" ..."<< endl;
        mountPartition();
    }
    else{
       cout<< "\u001B[31m" << "[BAD PARAM] Error en parametros del comando" << "\x1B[0m" << endl;
    }
}

int MOUNT_::findPartitionIndex(){
    FILE *file;
    file=fopen(path.c_str(),"rb+");
    if(file != NULL){
        //Se lee el Master Boot Record
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        for(int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_status != '1'){
                //Si es la particion
                if(strcmp(mbr_partitions[i]->part_name,name.c_str()) == 0){
                    fclose(file);
                    return i;
                }
            }
        }
        fclose(file);
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }
    //No se encontro particion primaria con ese nombre
    return -1;
}

int MOUNT_::findLogicPartitionStart(){
    FILE *file;
    file=fopen(path.c_str(),"rb+");
    if(file != NULL){
        
        int extendedIndex = -1;
        //Se lee el MBR
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        //Se busca en el arreglo de particiones la particion extendida
        for(int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_type == 'E'){
                extendedIndex = i;
                break;
            }
        }
        //Si existe un particion extendida
        if(extendedIndex != -1){
            //Se lee Extended Boot Record
            EBR ebr;
            fseek(file, mbr_partitions[extendedIndex]->part_start,SEEK_SET);
            fread(&ebr,sizeof (EBR),1,file);
            fseek(file, mbr_partitions[extendedIndex]->part_start,SEEK_SET);
            
            //se busca la particion logica
            while(fread(&ebr,sizeof(EBR),1,file)!=0){

                if(strcmp(this->getName().c_str(),ebr.part_name) == 0){
                    //Retorna el inicio del EBR de la particion logica encontrada
                    fclose(file);
                    return (ebr.part_start - sizeof(EBR));
                }
                //Si ya no hay una EBR que le siga
                if(ebr.part_next == -1){
                    break;
                }
                //Se pasa al siguiente EBR
                else {
                    fseek(file,ebr.part_next,SEEK_SET);
                }
            }
        }
        fclose(file);
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe disco en el path "<< "\x1B[0m" << endl;
    }
    //No se encontro particion logica con ese nombre
    return -1;
}

void MOUNT_::mountPartition(){
    int partitionIndex;


    //Si es una particion primaria
    partitionIndex = findPartitionIndex();
    if(partitionIndex != -1){
        FILE*file= fopen(this->path.c_str(),"r+b");
        //Se lee MBR
        MBR master;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof (MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;
        //Se cambia el estatus a montado
        mbr_partitions[partitionIndex]->part_status = '2';

        //Se actualiza MBR
        fseek(file,0, SEEK_SET);
        fwrite(&master,sizeof (MBR),1,file);
        fseek(file,0,SEEK_SET);
        //Se vuelve a leer MBR
        fread(&master, sizeof (MBR),1,file);
        fclose(file);

        if(isMounted()){
            cout<< "\u001B[31m" << "[BAD PARAM] La particion ya estaba montada" << "\x1B[0m" << endl;
            
        }
        else{
            mounted->push_back(*this);
            cout<< "\u001B[32m" << "[OK] La particion " <<this->name<<" ha sido montada con ID: "<<this->id<< "\x1B[0m" << endl;
        }
        return;
    }

    //Si es particion logica
    partitionIndex = findLogicPartitionStart();
    if(partitionIndex != -1){
        FILE * file = fopen(getPath().c_str(),"r+b");
        //Se lee el EBR
        EBR ebr;
        fseek(file, partitionIndex, SEEK_SET);
        fread(&ebr, sizeof(EBR),1,file);
        ebr.part_status = '2';
        //Se reescribe el EBR
        fseek(file,partitionIndex,SEEK_SET);
        fwrite(&ebr,sizeof(EBR),1, file);
        fclose(file);

        if(isMounted()){
            cout<< "\u001B[31m" << "[BAD PARAM] La particion ya estaba montada" << "\x1B[0m" << endl;
        }
        else{
            mounted->push_back(*this);
            cout<< "\u001B[32m" << "[OK] La particion " <<this->id<<" ha sido montada"<< "\x1B[0m" << endl;
        }
        return;
    }



}

