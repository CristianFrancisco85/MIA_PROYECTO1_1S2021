#ifndef RMUSR_H
#define RMUSR_H

#endif // RMUSR_H

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <structs.h>
#include <parser.h>

using namespace std;

class RMUSR_{
private:
    string user;
    bool statusFlag;
public:
    RMUSR_(){
        this->user="";
        this->statusFlag=false;
    };

    /**
      *Setter del User 
      * @param value: Nombre de usuario
     */
    void setUser(char* value);

    /**
      *Comienza la ejecucion del comando
     */
    void init();

    /**
     * Regresa ID del grupo y -1 si no existe
     * @param name: Nombre del grupo
     */
    bool buscarUsuario(string name);

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
};

extern bool loged;
extern Sesion sesion;

void RMUSR_::setUser(char *name){
    this->user = name;
    if(name[0] == '\"')
    {
        this->user = this->user.substr(1, this->user.length() - 2);
    }
}

void RMUSR_::setStatus(){
    if(this->user != ""){
        this->statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No se especifico el parametro User "<< "\x1B[0m" << endl;
    }
}

void RMUSR_::init(){
    setStatus();
    if(statusFlag){
        if(loged){
            if(sesion.user == 1 && sesion.group == 1){
                if(buscarUsuario(user)){
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
                        char auxContent[1000] = "\0";
                        char *endString;
                        char *lineToken = strtok_r(content,"\n",&endString);
                        
                        while(lineToken != NULL){
                            char id[2];
                            char tipo[2];
                            char userArr[15];
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
                                if(strcmp(tipo,"U") == 0){
                                    auxToken = strtok_r(NULL,",",&endToken);
                                    auxToken = strtok_r(NULL,",",&endToken);
                                    strcpy(userArr,auxToken);
                                    if(strcmp(userArr,user.data()) == 0){
                                        //Se encontro el grupo
                                        auxLine[0]='0';
                                    }
                                }
                            }                          
                            strcat(auxContent,auxLine);
                            lineToken = strtok_r(NULL,"\n",&endString);
                        }

                        //Se reescribe el users.txt
                        for(int i = 0; i < 12; i++){
                            if(inodo.i_block[i] != -1){
                                BloqueArchivos archivo;
                                for(int j = 0; j < 63; j++){
                                    archivo.b_content[j]=auxContent[i*64+j];
                                    fseek(file,super.s_block_start + sizeof(BloqueArchivos)*inodo.i_block[i],SEEK_SET);
                                    fwrite(&archivo,sizeof(BloqueArchivos),1,file);
                                }
                            }
                        }
                        
                        cout<< "\u001B[32m" << "[OK] Usuario Eliminado Exitosamente"<< "\x1B[0m" << endl;
                        if(sesion.sistemaType == 3){
                            char content[64];
                            char operacion[10];
                            char path[1];
                            strcpy(content,user.data());
                            strcpy(operacion,"rmusr");
                            path[0]='-';
                            guardarJournal(operacion,path,content);
                        }

                        fclose(file);
                    }
                }
                else {
                    cout<< "\u001B[31m" << "[BAD PARAM] El usuario que desea eliminar no existe"<< "\x1B[0m" << endl;
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

bool RMUSR_::buscarUsuario(string name){
    
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
            char user[15];
            char *endToken;
            char *auxToken = strtok_r(lineToken,",",&endToken);
            strcpy(id,auxToken);
            if(strcmp(id,"0") != 0){
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(tipo,auxToken);
                if(strcmp(tipo,"U") == 0){
                    auxToken = strtok_r(NULL,",",&endToken);
                    auxToken = strtok_r(NULL,",",&endToken);
                    strcpy(user,auxToken);
                    if(strcmp(user,name.c_str()) == 0){
                        return true;
                    }
                }
            }
            lineToken = strtok_r(NULL,"\n",&endString);
        }
        
    }

    return false;
}

void RMUSR_::guardarJournal(char* operacion,char *path,char *content){
    
    
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
            if(fread(&registroAux,sizeof(Journal),1,file)==0){
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