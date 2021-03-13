#pragma once
#ifndef RMGRP_H
#define RMGRP_H

#endif // RMGRP_H

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <structs.h>
#include <parser.h>

using namespace std;
extern bool loged;
extern Sesion sesion;

class RMGRP_{
private:
    string name;
    bool statusFlag;
public:
    RMGRP_(){
        this->name="";
        this->statusFlag=false;
    };

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
     * Regresa ID del grupo y -1 si no existe
     * @param name: Nombre del grupo
     */
    int buscarGrupo(string name);

    /**
     *Verifica que no haya errores en el comando
    */
    void setStatus();

    /**
     *Guarda una operacion en el Journal 
     * @param operacion: Nombre de la operacion
     * @param path: Path de la operacion
     * @param content: Data para operaciones en users.txt o mkfile int
     */
    void guardarJournal(char *operacion, char *path, char *content);

    /**
     *Realiza la eliminacion del grupo
    */
    void deleteGroup();

};

void RMGRP_::setName(char *name){
    this->name = name;
    if(name[0] == '\"')
    {
        this->name = this->name.substr(1, this->name.length() - 2);
    }
}

void RMGRP_::setStatus(){
    if(this->name != ""){
        this->statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No se especifico el parametro name "<< "\x1B[0m" << endl;
    }
}

void RMGRP_::init(){
    setStatus();
    if(statusFlag){
        if(loged){
            if(sesion.user == 1 && sesion.group == 1){
                if(buscarGrupo(name) != -1){
                    deleteGroup();
                    cout<< "\u001B[32m" << "[OK] Grupo Eliminado Exitosamente"<< "\x1B[0m" << endl;
                    if(sesion.sistemaType == 3){
                        char content[64];
                        char operacion[10];
                        char path[10];
                        for(int j=0;j<10;j++){
                            path[j]='\0';
                        }
                        strcpy(content,name.data());
                        strcpy(operacion,"rmgrp");
                        guardarJournal(operacion,path,content);
                    }
                }
                else {
                    cout<< "\u001B[31m" << "[BAD PARAM] El grupo que desea eliminar no existe"<< "\x1B[0m" << endl;
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

int RMGRP_::buscarGrupo(string name){
    
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
        for(int i = 0; i < 15; i++){
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

void RMGRP_::guardarJournal(char* operacion,char *path,char *content){
    
    
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
            if(strcmp(registroAux.operationType,"mkgrp") != 0 && strcmp(registroAux.operationType,"mkusr") != 0  && strcmp(registroAux.operationType,"rmusr") != 0 
            && strcmp(registroAux.operationType,"rmgrp") != 0 && strcmp(registroAux.operationType,"mkdir") != 0  && strcmp(registroAux.operationType,"mkfile") != 0 ){
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

void RMGRP_::deleteGroup(){
    FILE *file = fopen(sesion.direccion.data(),"r+b");

    if(file!=NULL){
        
        //Se lee superbloque
        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee inodo de user.txt
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        //Se lee el contenido de users.txt
        char content[1000] = "\0";
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                BloqueArchivos archivo;
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                fread(&archivo,sizeof(BloqueArchivos),1,file);
                strcat(content,archivo.b_content);
            }
        }
        //Se parsea el contenido
        string auxContent;
        char *endString;
        char *lineToken = strtok_r(content,"\n",&endString);

        while(lineToken != NULL){
            char id[2];
            char tipo[2];
            char group[15];
            char *auxLine;
            auxLine = static_cast<char*>(malloc(strlen(lineToken)+2));
            strcpy(auxLine, lineToken);
            strcat(auxLine,"\n"); 
            char *endToken;
            char *auxToken = strtok_r(lineToken,",",&endToken);
            strcpy(id,auxToken);
            
            if(strcmp(id,"0") != 0){
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(tipo,auxToken);
                if(strcmp(tipo,"G") == 0){
                    strcpy(group,endToken);
                    if(strcmp(group,name.data()) == 0){
                        //Se encontro el grupo
                        auxLine[0]='0';

                    }
                }
            }                          
            auxContent+=auxLine;
            lineToken = strtok_r(NULL,"\n",&endString);
        }
        
        //Se reescribe el users.txt
        for(int i = 0; i < 12; i++){
            if(inodo.i_block[i] != -1){
                BloqueArchivos *archivo = new BloqueArchivos;
                string auxString = "";
                for(int j = 0; j <= 64; j++){
                    if(i*64+j<auxContent.size()&&(int)auxContent.at(i*64+j)!=1){
                        auxString += auxContent.at(i*64+j);  
                    }                       
                }
                strcpy(archivo->b_content,auxString.data());
                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                fwrite(archivo,sizeof(BloqueArchivos),1,file);
            }
        }

        fclose(file);
    }

}