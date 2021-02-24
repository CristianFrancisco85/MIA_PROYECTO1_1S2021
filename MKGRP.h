#ifndef MKGRP_H
#define MKGRP_H

#endif // MKGRP_H

#include <iostream>
#include <string.h>
#include <parser.h>
#include <structs.h>

using namespace std;

class MKGRP_{
private:
    string name;
    bool statusFlag;
public:

    MKGRP_():name(""), statusFlag(false){
        this->name="";
        this->statusFlag=false;
    }

    /**
      *Setter del User 
      * @param value: Nombre de usuario
     */
    void setName(char* value);

    /**
      *Verifica que no haya errores en el comando
     */
    void setStatus();

    /**
      *Comienza la ejecucion del comando
     */
    void init();
    
    /**
      *Agrega informacion a al archivo users.txt
     */
    void addDataToUsers(string newData);

    /**
     * Regresa el bit libre en el bitmap de inodos o bloques segun el ajuste
     * @param FILE fp: stream del archvio que se esta leyendo
     * @param int tipo: tipo de bit a buscar (Inodo/Bloque)
     * @param char fit: ajuste de la particion 
    */
    int buscarBit(FILE *fp,char fit,char tipo);

    /**
     *Guarda una operacin ele Journal 
     * @param name: Nombre del grupo
     */
    void guardarJournal(char *operacion, int tipo, int permisos, char *nombre, char *content);

    /**
     * Regresa ID del grupo y -1 si no existe
     * @param name: Nombre del grupo
     */
    int buscarGrupo(string name);

    /**
     * Regresa ID para un nuevo grupo
     */
    int getIdGroup();

};

extern bool loged;
extern Sesion sesion;

void MKGRP_::setName(char *value){
    this->name = value;
    if(this->name[0] == '\"'){
        this->name = this->name.substr(1, this->name.length() - 2);
    }
}

void MKGRP_::setStatus(){
    if(this->name != ""){
        this->statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No se especifico el parametro name "<< "\x1B[0m" << endl;
    }
}

void MKGRP_::init(){
    setStatus();
    if(this->statusFlag){
        if(loged){
            if(sesion.user == 1 && sesion.group == 1){

                int grupo = buscarGrupo(name);
                if(grupo == -1){
                    int idGrp = getIdGroup();
                    string grupoStr = to_string(idGrp)+",G,"+name+"\n";
                    addDataToUsers(grupoStr);
                    cout<< "\u001B[32m" << "[OK] Grupo creado exitosamente "<< "\x1B[0m" << endl;
                    if(sesion.sistemaType == 3){
                        char aux[64];
                        char operacion[10];
                        char content[2];
                        strcpy(aux,grupoStr.data());
                        strcpy(operacion,"mkgrp");
                        memset(content,0,2);
                        guardarJournal(operacion,0,0,aux,content);
                    }
                }
                else{
                    cout<< "\u001B[31m" << "[BAD PARAM] Ya existe un grupo con el mismo nombre "<< "\x1B[0m" << endl;
                }
            }
            else{
                cout<< "\u001B[31m" << "[BAD CONTEXT] Tiene que iniciar sesion como ROOT "<< "\x1B[0m" << endl;
            }
        }
        else{
            cout<< "\u001B[31m" << "[BAD CONTEXT] Tiene que iniciar sesion para ejecutar este comando "<< "\x1B[0m" << endl;
        }
    }
}

void MKGRP_::addDataToUsers(string newData){

    FILE *file = fopen(sesion.direccion.data(), "r+b");

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

    //Se lee el bloque de apuntadores
    BloqueArchivos archivo;
    fseek(file,super.s_block_start + sizeof(BloqueArchivos)*lastBlock,SEEK_SET);
    fread(&archivo,sizeof(BloqueArchivos),1,file);

    //Espacio Usado en el blqoue
    int used = strlen(archivo.b_content);
    //Espacio Libre en el blqoue
    int free = 63 - used;
    //Tamaño de la informacion nueva
    int size = newData.size();


    if( size <= free){
        
        //Se escribe infromacion en el bloque
        strcat(archivo.b_content,newData.data());
        
        //Se lee Inodo 
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);
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
    else{
        string aux = "";
        string aux2 = "";
        int i = 0;

        while(i < free){
            aux += newData.at(i);
            i++;
        }
        while(i < size){
            aux2 += newData.at(i);
            i++;
        }

        //Se guarda lo que se pueda en el primer bloque
        strcat(archivo.b_content,aux.data());
        fseek(file,super.s_block_start + sizeof(BloqueArchivos)*lastBlock,SEEK_SET);
        fwrite(&archivo,sizeof(BloqueArchivos),1,file);
        BloqueArchivos auxArchivo;
        strcpy(auxArchivo.b_content,aux2.data());
        int bit = buscarBit(file,'B',sesion.fit);
    
        //Se actualiza y escribe inodo
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);
        inodo.i_size = inodo.i_size + newData.length();
        inodo.i_mtime = time(nullptr);
        inodo.i_block[lastBlock] = bit;
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fwrite(&inodo,sizeof(InodeTable),1,file);

        //Se actualiza bitmap y tabla de bloques
        fseek(file,super.s_block_start + (sizeof(BloqueArchivos)*bit),SEEK_SET);
        fwrite(&auxArchivo,sizeof(BloqueArchivos),1,file);
        fseek(file,super.s_bm_block_start + bit,SEEK_SET);
        fputc('2',file);

        //Se actualiza cantidad de bloques e inodos disponibles
        super.s_first_blo = super.s_first_blo + 1;
        super.s_free_blocks_count = super.s_free_blocks_count - 1;
        fseek(file,sesion.superStart,SEEK_SET);
        fwrite(&super,sizeof(SuperBloque),1,file);
    }
    fclose(file);
}

//REVISAR
int MKGRP_::buscarBit(FILE *file,char fit,char tipo){

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
        for(int i = 0; i < sizeBitMap; i++){
            fseek(file,inicioBitMap + i,SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                freeBit = i;
                return freeBit;
            }
        }
        if(freeBit == -1){
            cout<<"Ya no hay bits disponibles";
            return -1;
        }
    }
    else if(fit == 'B'){
        int libres = 0;
        int aux = -1;
    
        for(int i = 0; i < sizeBitMap; i++){
            fseek(file,inicioBitMap + i,SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                libres++;
                if(i+1 == sizeBitMap){
                    if(aux == -1 || aux == 0){
                        aux = libres;
                    }
                    else{
                        if(aux > libres){
                            aux = libres;
                        }
                    }
                    libres = 0;
                }
            }
            else if(auxBit == '1'){
                if(aux == -1 || aux == 0){
                    aux = libres;
                }
                else{
                    if(aux > libres){
                        aux = libres;
                    }
                }
                libres = 0;
            }
        }

        for(int i = 0; i < sizeBitMap; i++){
            fseek(file,inicioBitMap + i, SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                libres++;
                if(i+1 == sizeBitMap){
                    return ((i+1)-libres);
                }
            }
            else if(auxBit == '1'){
                if(aux == libres){
                    return ((i+1) - libres);
                }
                libres = 0;
            }
        }

        return -1;

    }
    else if(fit == 'W'){
        int libres = 0;
        int aux = -1;

        for (int i = 0; i < sizeBitMap; i++) {
            fseek(file,inicioBitMap + i, SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                libres++;
                if(i+1 == sizeBitMap){
                    if(aux == -1 || aux == 0){
                        aux = libres;
                    }
                    else {
                        if(aux < libres){
                            aux = libres;
                        }
                    }
                    libres = 0;
                }
            }
            else if(auxBit == '1'){
                if(aux == -1 || aux == 0){
                    aux = libres;
                }
                else{
                    if(aux < libres){
                        aux = libres;
                    }
                }
                libres = 0;
            }
        }

        for (int i = 0; i < sizeBitMap; i++) {
            fseek(file,inicioBitMap + i, SEEK_SET);
            auxBit = fgetc(file);
            if(auxBit == '0'){
                libres++;
                if(i+1 == sizeBitMap){
                    return ((i+1) - libres);
                }
            }
            else if(auxBit == '1'){
                if(aux == libres){
                    return ((i+1) - libres);
                }
                libres = 0;
            }
        }

        return -1;
    }

    return 0;
}

int MKGRP_::getIdGroup(){

    FILE *file = fopen(sesion.direccion.data(),"rb+");

    char auxArr[400] = "\0";
    int auxID = -1;

    //Se lee superbloque
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    //Se lee inodo de users.txt
    InodeTable inodo;
    fseek(file,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,file);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BloqueArchivos archivo;
            fseek(file,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BloqueArchivos),1,file);
            }
            strcat(auxArr,archivo.b_content);
        }
    }

    fclose(file);

    char *endString;
    char *token = strtok_r(auxArr,"\n",&endString);
    while(token != NULL){
        char id[2];
        char tipo[2];
        char *endToken;
        char *auxToken = strtok_r(token,",",&endToken);
        strcpy(id,auxToken);
        if(strcmp(id,"0") != 0){
            auxToken = strtok_r(NULL,",",&endToken);
            strcpy(tipo,auxToken);
            if(strcmp(tipo,"G") == 0){
                auxID = atoi(id);
            }
        }
        token = strtok_r(NULL,"\n",&endString);
    }
    auxID++;
    return auxID;
}

void MKGRP_::guardarJournal(char* operacion,int tipo,int permisos,char *nombre,char *content){
    
    Journal journal;
    
    journal.journal_owner = sesion.user;
    journal.journal_permissions = permisos;
    journal.journal_type = tipo;
    journal.journal_date = time(nullptr);
    strcpy(journal.journal_name,nombre);
    strcpy(journal.journal_content,content);
    strcpy(journal.journal_operation_type,operacion);
    

    FILE *file = fopen(sesion.direccion.data(),"r+b");

    Journal registroAux;
    bool auxBool = false;

    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    int journalStart = sesion.superStart + sizeof(SuperBloque);
    int journalEnd = super.s_bm_inode_start;

    fseek(file,journalStart,SEEK_SET);
    while((ftell(file) < journalEnd) && !auxBool){
        fread(&registroAux,sizeof(Journal),1,file);
        if(registroAux.journal_type != 0 && registroAux.journal_type != 1)
            auxBool = true;
    }
    fseek(file,ftell(file)- static_cast<int>(sizeof(Journal)),SEEK_SET);
    fwrite(&journal,sizeof(Journal),1,file);
    fclose(file);
}

int MKGRP_::buscarGrupo(string name){
    
    FILE *fp = fopen(sesion.direccion.data(),"r+b");

    char cadena[400] = "\0";
    SuperBloque super;
    InodeTable inodo;

    fseek(fp,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,fp);

    //Se lee el inodo de users.txt
    fseek(fp,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,fp);

    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            BloqueArchivos archivo;
            fseek(fp,super.s_block_start,SEEK_SET);
            for(int j = 0; j <= inodo.i_block[i]; j++){
                fread(&archivo,sizeof(BloqueArchivos),1,fp);
            }
            strcat(cadena,archivo.b_content);
        }
    }

    fclose(fp);

    char *endString;
    char *token = strtok_r(cadena,"\n",&endString);
    while(token != NULL){
        char id[2];
        char tipo[2];
        char group[15];
        char *endToken;
        char *auxToken = strtok_r(token,",",&endToken);
        strcpy(id,auxToken);
        if(strcmp(id,"0") != 0){
            auxToken = strtok_r(NULL,",",&endToken);
            strcpy(tipo,auxToken);
            if(strcmp(tipo,"G") == 0){
                strcpy(group,endToken);
                if(strcmp(group,name.data()) == 0)
                    return atoi(id);
            }
        }
        token = strtok_r(NULL,"\n",&endString);
    }

    return -1;
}