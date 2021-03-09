#ifndef MKGRP_H
#define MKGRP_H

#endif // MKGRP_H

#include <iostream>
#include <string.h>
#include <parser.h>
#include <structs.h>

using namespace std;
extern bool loged;
extern Sesion sesion;

class MKGRP_{
private:
    string name;
    bool statusFlag;
public:

    MKGRP_(){
        this->name="";
        this->statusFlag=false;
    }

    /**
      *Setter del Name 
      * @param value: Nombre de usuario
     */
    void setName(char* value);

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
     * @param FILE fp: stream del archivo que se esta leyendo
     * @param int tipo: tipo de bit a buscar (Inodo/Bloque)
     * @param char fit: ajuste de la particion 
    */
    int buscarBit(FILE *fp,char fit,char tipo);

    /**
     *Guarda una operacion en el Journal 
     * @param operacion: Nombre de la operacion
     * @param path: Path de la operacion
     * @param content: Data para operaciones en users.txt o mkfile int
     */
    void guardarJournal(char *operacion, char *path, char *content);

    /**
     * Regresa ID del grupo y -1 si no existe
     * @param name: Nombre del grupo
     */
    int buscarGrupo(string name);

    /**
     * Regresa ID para un nuevo grupo
     */
    int getIdGroup();

    /**
     *Verifica que no haya errores en el comando
    */
    void setStatus();

};

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
                    string grupoStr = to_string(getIdGroup())+",G,"+name+"\n";
                    addDataToUsers(grupoStr);
                    cout<< "\u001B[32m" << "[OK] Grupo creado exitosamente "<< "\x1B[0m" << endl;
                    if(sesion.sistemaType == 3){
                        char content[64];
                        char operacion[10];
                        for(int j=0;j<10;j++){
                            operacion[j]='\0';
                        }
                        char *nameC = new char[name.length() + 1];
                        strcpy(content,grupoStr.data());
                        strcpy(operacion,"mkgrp");
                        strcpy(nameC,name.c_str());
                        guardarJournal(operacion,nameC,content);
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

int MKGRP_::getIdGroup(){

    FILE *file = fopen(sesion.direccion.data(),"rb+");

    if(file!=NULL){
        //Se lee superbloque
        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee inodo de users.txt
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable), SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        char auxArr[1000] = "\0";
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                BloqueArchivos archivo;
                fseek(file,super.s_block_start,SEEK_SET);
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                fread(&archivo,sizeof(BloqueArchivos),1,file);
                strcat(auxArr,archivo.b_content);
            }
        }

        fclose(file);

        char *endString;
        char *lineToken = strtok_r(auxArr,"\n",&endString);
        int auxID = -1;
        while(lineToken != NULL){
            char id[2];
            char tipo[2];
            char *endToken;
            char *auxToken = strtok_r(lineToken,",",&endToken);
            strcpy(id,auxToken);
            if(strcmp(id,"0") != 0){
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(tipo,auxToken);
                if(strcmp(tipo,"G") == 0){
                    auxID = atoi(id);
                }
            }
            lineToken = strtok_r(NULL,"\n",&endString);
        }
        auxID++;
        return auxID;
    }
    else{
        return -1;
    }
}

void MKGRP_::guardarJournal(char* operacion,char *path,char *content){
    
    
    FILE *file = fopen(sesion.direccion.data(),"r+b");

    if(file!=NULL){
        Journal registro;
        Journal registroAux;
        
        //Se configura el registro
        registro.user = sesion.user;
        strcpy(registro.operationType,operacion);
        strcpy(registro.path,path);
        strcpy(registro.content,content);
        registro.date = time(nullptr);
        
        //Se lee superbloque
        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);
        //Se posiciona al principio del journal
        fseek(file,sesion.superStart + sizeof(SuperBloque),SEEK_SET);
        
        //Se busca ultimo registro
        while(ftell(file) < super.s_bm_inode_start){
            fread(&registroAux,sizeof(Journal),1,file);
            if(registroAux.content[0]=='\0'){
                break;
            }
        }
        //Se escribe el registro
        fseek(file,ftell(file)-sizeof(Journal),SEEK_SET);
        fwrite(&registro,sizeof(Journal),1,file);
        fclose(file);
    }
    else{

    }

    
}

int MKGRP_::buscarGrupo(string name){
    
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