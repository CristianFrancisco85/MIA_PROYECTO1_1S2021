#ifndef RECOVERY_H
#define RECOVERY_H

#endif // RECOVERY_H
#include <string.h>
#include <stdio.h>
#include <MOUNT.h>
#include <iostream>
#include <list>
#include <iterator>
#include <structs.h>

#include "MKFILE.h"
#include "MKDIR.h"

extern MKFILE_* mkfile_;
extern MKDIR_* mkdir_;

using namespace std;
extern list<MOUNT_> *mounted;
extern bool loged;
extern Sesion sesion;

class RECOVERY_{

private:
    string id; //ID de la particion
    bool statusFlag; //Indica si hay errores en el comando
public:

    RECOVERY_(){
        this->id="";
        this->statusFlag=false;
    }

    /**
     * Setter del ID
     * @param id: ID de la particion a recuperar
    */
    void setId(char* id);

    /**
     * Getter del ID
    */
    string getId();

    /**
     * Verifica si hay errores en el comando
    */
    void setStatus();

    /*
     * Inicia el proceso de recuperacion
    */
    void init();

    /*
     * Trata de recuperar la particion
    */
    void makeRecovery(MOUNT_ *disk);

    /**
      *Agrega informacion a al archivo users.txt
    */
    void addDataToUsers(string newData);

    /**
     * Retorna el struct del usuario si existe
     * @param name: Nombre del usuario
     */
    Usuario getUsuario(int idUser);

    /**
     * Regresa ID del grupo y -1 si no existe
     * @param name: Nombre del grupo
     */
    int buscarGrupo(string name);

    /**
     * Regresa el bit libre en el bitmap de inodos o bloques segun el ajuste
     * @param FILE fp: stream del archivo que se esta leyendo
     * @param int tipo: tipo de bit a buscar (Inodo/Bloque)
     * @param char fit: ajuste de la particion 
    */
    int buscarBit(FILE *fp,char fit,char tipo);

};

void RECOVERY_::setId(char * id){
    this->id = id;
    if(this->id[0] == '\"'){
        this->id = this->id.substr(1,this->id.size()-2);
    }
}

string RECOVERY_::getId(){
    return this->id;
}

void RECOVERY_::setStatus(){
    if(this->id != ""){
        statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] Se tiene que especificar ID de la particion" << "\x1B[0m" << endl;
    }
}

void RECOVERY_::init(){
    setStatus();
    if(this->statusFlag){
        int partIndex = -1;
        MOUNT_ *disk;
        list<MOUNT_>::iterator i;
        for(i=mounted->begin(); i != mounted->end(); i++){
            if(i->getId() == this->id){
                disk = &*i;
                break;
            }
        }

        if(disk!=NULL){
            makeRecovery(disk);
            if(loged){
                if(strcmp(disk->getPath().c_str(),sesion.direccion.c_str())==0){
                    makeRecovery(disk);
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] No esta logueado en la particion indicada por el parametro -id" << "\x1B[0m" << endl;
                }  
            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] Necisita loguearse para efectuar esta operacion" << "\x1B[0m" << endl;
            } 
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] La particion no esta montada" << "\x1B[0m" << endl;
        }           
    }

}

void RECOVERY_::makeRecovery(MOUNT_ *disk){

    FILE *file = fopen(disk->getPath().c_str(),"rb+");

    if(file!=NULL){


        SuperBloque super;
        int inicioJournal = 0;
        int inicioSuper = 0;
        
        int tempUsr = sesion.user;
        int tempGrp = sesion.group;

        int partIndex = disk->findPartitionIndex();

        if(partIndex != -1){
            MBR master;
            fread(&master,sizeof(MBR),1,file);

            Partition *mbr_partitions[4];
            mbr_partitions[0]=&master.mbr_partition_1;
            mbr_partitions[1]=&master.mbr_partition_2;
            mbr_partitions[2]=&master.mbr_partition_3;
            mbr_partitions[3]=&master.mbr_partition_4;

            fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
            inicioJournal =mbr_partitions[partIndex]->part_start + sizeof(SuperBloque);
            inicioSuper = mbr_partitions[partIndex]->part_start;
            fread(&super,sizeof(SuperBloque),1,file);
        }
        else{
            partIndex = disk->findLogicPartitionStart();
            if(partIndex != -1){
                EBR ebr;
                fseek(file,partIndex,SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);
                fread(&super,sizeof(SuperBloque),1,file);
            }
        }

        Journal registro;
        Usuario user;
        if(super.s_filesystem_type == 3){
            fseek(file,inicioJournal,SEEK_SET);
            while(ftell(file) < super.s_bm_inode_start){
                fread(&registro,sizeof(Journal),1,file);

                if(strcmp(registro.operationType,"mkgrp") == 0 || strcmp(registro.operationType,"mkusr") == 0){
                    sesion.user = 1;
                    sesion.group = 1;
                    addDataToUsers(registro.content);
                }
                else if(strcmp(registro.operationType,"mkdir") == 0){
                    user = getUsuario(registro.user);
                    sesion.user = user.id_usr;
                    sesion.group = user.id_grp;
                    mkdir_->setPath(registro.path);
                    mkdir_->setPParam();
                    mkdir_->init();
                    mkdir_= new MKDIR_();
                }
                else if(strcmp(registro.operationType,"mkfile") == 0){
                    user = getUsuario(registro.user);
                    sesion.user = user.id_usr;
                    sesion.group = user.id_grp;

                    mkfile_->setPath(registro.path);
                    mkfile_->setRParam();
                                       
                    bool aux;
                    string auxString;
                    auxString.append(registro.content);
                    if(isdigit(registro.content[0]))
                        mkfile_->setSize(registro.content);
                    else{
                        mkfile_->setCont(registro.content);
                    }
                    mkfile_->init();
                    mkfile_= new MKFILE_();
                                
                }
            }
            cout<< "\u001B[32m" << "[BAD PARAM] Se ha recuperado la informacion exitosamente" << "\x1B[0m" << endl;
            sesion.user = tempUsr;
            sesion.group = tempGrp;
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] No se puede recuperar en un sistema ext2" << "\x1B[0m" << endl;
        }  
        
    }
}

void RECOVERY_::addDataToUsers(string newData){

    FILE *file = fopen(sesion.direccion.data(), "r+b");

    if(file!=NULL){

        //Se lee SuperBloque
        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee inodo se users.txt
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        //Se busca el ultimo bloque
        int lastBlock = 0;
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                lastBlock = inodo.i_block[i];
            }
        }

        //Se lee el el ultimo bloque
        BloqueArchivos archivo;
        fseek(file,super.s_block_start + sizeof(BloqueArchivos)*lastBlock,SEEK_SET);
        fread(&archivo,sizeof(BloqueArchivos),1,file);

        //Espacio Usado en el ultimo bloque 
        int used = strlen(archivo.b_content);
        //Espacio Libre en el blqoue
        int free = 64 - used;


        if( newData.size() > free){

            string aux = "";
            string aux2 = "";
            int i = 0;

            while(i < free){
                aux += newData.at(i);
                i++;
            }
            while(i < newData.size()){
                aux2 += newData.at(i);
                i++;
            }

            //Se guarda lo que se pueda en el primer bloque
            strcat(archivo.b_content,aux.data());
            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*lastBlock,SEEK_SET);
            fwrite(&archivo,sizeof(BloqueArchivos),1,file);
            //Se crea un bloque nuevo para escribir el faltante
            BloqueArchivos auxArchivo;
            strcpy(auxArchivo.b_content,aux2.data());

            int bit = buscarBit(file,sesion.fit,'B');
            
            //Se actualiza y escribe inodo
            inodo.i_size = inodo.i_size + newData.length();
            inodo.i_mtime = time(nullptr);  
            inodo.i_block[lastBlock] = bit;
            fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
            fwrite(&inodo,sizeof(InodeTable),1,file);

            //Se actualiza bitmap y se escribe nuevo bloques
            fseek(file,super.s_bm_block_start + bit,SEEK_SET);
            fputc('2',file);
            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bit,SEEK_SET);
            fwrite(&auxArchivo,sizeof(BloqueArchivos),1,file);

            //Se actualiza cantidad de bloques e inodos disponibles
            super.s_first_blo = super.s_first_blo + 1;
            super.s_free_blocks_count = super.s_free_blocks_count - 1;
            fseek(file,sesion.superStart,SEEK_SET);
            fwrite(&super,sizeof(SuperBloque),1,file); 
            

        }
        else{
            //Se escribe infromacion en el bloque
            strcat(archivo.b_content,newData.data());
            
            //Se configura Inodo
            inodo.i_size = inodo.i_size + newData.length();
            inodo.i_mtime = time(NULL);
            //Se actualiza Inodo
            fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
            fwrite(&inodo,sizeof(InodeTable),1,file);

            //Se actualiza Bloque
            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*lastBlock,SEEK_SET);
            fwrite(&archivo,sizeof(BloqueArchivos),1,file);

        }
        fclose(file);
    }
}

Usuario RECOVERY_::getUsuario(int idUser){
    
    FILE *file = fopen(sesion.direccion.data(),"r+b");
    Usuario userStr;

    if(file!=NULL){

        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee el inodo de users.txt
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        char content[1000] = "\0";
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                BloqueArchivos archivo;
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                fread(&archivo,sizeof(BloqueArchivos),1,file);
                strcat(content,archivo.b_content);
            }
        }

        fclose(file);

        
        
        char *endString;
        char *lineToken = strtok_r(content,"\n",&endString);
        while(lineToken != NULL){
            char tipo[2];
            char *endToken;
            char *auxToken = strtok_r(lineToken,",",&endToken);
            //ID
            userStr.id_usr=atoi(auxToken);
            if(userStr.id_usr == idUser){
                //Tipo
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(tipo,auxToken);
                if(strcmp(tipo,"U") == 0){
                    //Grupo
                    auxToken = strtok_r(NULL,",",&endToken);
                    strcpy(userStr.group,auxToken);   
                    userStr.id_grp=buscarGrupo(auxToken);
                    //Username
                    auxToken = strtok_r(NULL,",",&endToken);
                    strcpy(userStr.username,auxToken);                  
                    //Password
                    auxToken = strtok_r(NULL,",",&endToken);
                    strcpy(userStr.password,auxToken);   
                    return userStr;
                }
            }
            lineToken = strtok_r(NULL,"\n",&endString);
        }
        
    }

    return userStr;
}

int RECOVERY_::buscarGrupo(string name){
    
    FILE *file = fopen(sesion.direccion.data(),"r+b");

    if(file!=NULL){

        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee el inodo de users.txt
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        char content[1000] = "\0";
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                BloqueArchivos archivo;
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                fread(&archivo,sizeof(BloqueArchivos),1,file);
                strcat(content,archivo.b_content);
            }
        }

        fclose(file);

        char *endString;
        char *lineToken = strtok_r(content,"\n",&endString);
        while(lineToken != NULL){
            char id[2];
            char tipo[2];
            char group[15];
            char *endToken;
            char *auxToken = strtok_r(lineToken,",",&endToken);
            strcpy(id,auxToken);
            if(strcmp(id,"0") != 0){
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(tipo,auxToken);
                if(strcmp(tipo,"G") == 0){
                    strcpy(group,endToken);
                    if(strcmp(group,name.data()) == 0){
                        return atoi(id);
                    }
                }
            }
            lineToken = strtok_r(NULL,"\n",&endString);
        }
    }
    

    return -1;
}

int RECOVERY_::buscarBit(FILE *file,char fit,char tipo){

    //Se lee superbloque
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    //Se lee las propiedades del bitmap
    int sizeBitMap = 0;
    int inicioBitMap = 0;
    if(tipo == 'B'){
        sizeBitMap = super.s_blocks_count;
        inicioBitMap = super.s_bm_block_start;
    }
    else if(tipo == 'I'){
        sizeBitMap = super.s_inodes_count;
        inicioBitMap = super.s_bm_inode_start;
    }

    char auxBit = '0';
    int freeBit = -1;
    
    if(fit == 'F'){
        for(int i = inicioBitMap; i < inicioBitMap+sizeBitMap; i++){
            fseek(file,i,SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                return i-inicioBitMap;
            }
        }
        cout<<"Ya no hay bits disponibles";
        return -1;
        
    }
    else if(fit == 'W'){
        int bitsLibres = 0;
        int aux = 0;
        int aux2 = -1;
    
        for(int i = inicioBitMap ; i < inicioBitMap+sizeBitMap; i++){
            fseek(file,i,SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '1'|| auxBit == '2'|| auxBit == '3'){
                if(aux != 0){
                    if(bitsLibres > aux){
                        aux = bitsLibres;
                        aux2 = i+1 -bitsLibres;
                    } 
                }
                else{
                    aux = bitsLibres;
                    aux2 = i+1 -bitsLibres;
                }
                bitsLibres = 0;
            }
            else if(auxBit == '0'){
                bitsLibres++;
                if(inicioBitMap+sizeBitMap == i+1){
                    if(aux != 0 ){
                        if(bitsLibres > aux){
                            aux = bitsLibres;
                            aux2 = i+1 -bitsLibres;
                        }
                    }
                    else{
                        aux = bitsLibres;
                        aux2 = i+1 -bitsLibres;
                    }
                    bitsLibres = 0;
                }
            }
        }

        return aux2-inicioBitMap;
    }
    else if(fit == 'B'){
        int bitsLibres = 0;
        int aux = 0;
        int aux2 = -1;
        
        for(int i = inicioBitMap ; i < inicioBitMap+sizeBitMap; i++){
            fseek(file,i,SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '1'||auxBit == '2'||auxBit == '3'){
                if(aux != 0){
                    if(bitsLibres < aux){
                        aux = bitsLibres;
                        aux2 = i+1 -bitsLibres;
                    } 
                }
                else{
                    aux = bitsLibres;
                    aux2 = i+1 -bitsLibres;
                }
                bitsLibres = 0;
            }
            else if(auxBit == '0'){
                bitsLibres++;
                if(inicioBitMap+sizeBitMap == i+1){
                    if(aux != 0 ){
                        if(bitsLibres < aux){
                            aux = bitsLibres;
                            aux2 = i+1 -bitsLibres;
                        }
                    }
                    else{
                        aux = bitsLibres;
                        aux2 = i+1 -bitsLibres;
                    }
                    bitsLibres = 0;
                }
            }
        }
        return aux2-inicioBitMap;
    }
    
    return 0;
}