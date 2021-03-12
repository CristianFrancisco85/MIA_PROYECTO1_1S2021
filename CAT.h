#ifndef MKFILE_H
#define MKFILE_H

#endif // MKFILE_H
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <MOUNT.h>
#include <list>
#include <iterator>
#include <structs.h>

using namespace std;

extern list<MOUNT_> *mounted;
extern bool loged;
extern Sesion sesion;

class CAT_{
private:
    list <string> *paths;//Lista donde se guardaran las rutas
    bool statusFlag; //Indica si hay errores en el comando
    
public:

    CAT_(){
        this->statusFlag=true;
    }

    /**
     * Setter para agregar una ruta al path
     * @param value: Ruta del nuevo archivo
    */
    void addPath(char *value);

    /**
     * Verifica si los parametros son validos
    */
    void setStatus();

    /**
     * Inicia la ejecucion del comando
    */
    void init();

    /**
     * Verifica si existe una carpeta o archivo 
     * @param file = Stream del disco 
     * @param path = Path de la carpeta o archivo
     * @return Numero de inodo o -1 si no existe
    */
    int buscarCarpetaArchivo(FILE *file, char* path);

    /**
     * Muestra el contenido de un archivo en la ruta
     * @param file = Stream del disco 
     * @param path = Path de la carpeta o archivo
     * @return True si se pudo mostrar false si no
    */
    bool mostrarContenidoArchivo(FILE *file, char* path);

    /**
     * Verifica si tiene los permisos de lectura
     * @param flagUser = Indica si es propietario de la carpeta padre
     * @param flagGrp = Indica si su grupo es propietario de la carpeta padre
     * @return True si los tiene , false si no
    */
    bool permisosDeLectura(int permisos, bool flagUsr, bool flagGrp);

};

void CAT_::addPath(char *value){
    this->paths->push_back(value);
}

void CAT_::setStatus(){
    if(this->paths->size()==0){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] No se especifico ninguna ruta en el comando CAT"<< "\x1B[0m" <<endl;
    }
}

void CAT_::init(){
    setStatus();
    if(this->statusFlag){
        if(loged){
            list<string>::iterator it;
            FILE *file = fopen(sesion.direccion.c_str(),"rb+");
            char auxChar[255];
            for(it=this->paths->begin(); it != this->paths->end(); it++){
                strcpy(auxChar,(*it).c_str());
                mostrarContenidoArchivo(file,auxChar);
            }
            fclose(file);
        }
        else{
            cout<< "\u001B[31m" << "[BAD CONTEXT] Tiene que loguearse para ejecutar este comando"<< "\x1B[0m" <<endl;
        }
    }
}

int CAT_::buscarCarpetaArchivo(FILE *file, char* path){

    int cont = 0;
    int byteInodo = 0;
    list<string> lista = list<string>();
    char *token = strtok(path,"/");
    

    while(token != NULL){
        lista.push_back(token);
        token = strtok(NULL,"/");
        cont++;
    }

    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);
    byteInodo = super.s_inode_start;
  
    for (int i = 0; i < cont; i++) {
       
        fseek(file,byteInodo,SEEK_SET);
        InodeTable inodo;
        fread(&inodo,sizeof(InodeTable),1,file);

        bool flag = false;

        for(int j = 0; j < 15; j++){
            if(inodo.i_block[j] != -1){

                int byteBloque = super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[j];
                string auxString="";
                fseek(file,byteBloque,SEEK_SET);

                BloqueCarpetas carpeta;
                //Apuntadores directos
                if(j < 12){
                    
                    fread(&carpeta,sizeof(BloqueCarpetas),1,file);
                    for (int m = 0; m < 4; m++) {

                        list<string>::iterator iterador = lista.begin();
                        for(int it=0; it<i; it++){
                            iterador++;
                        }
                        auxString = *iterador;

                        if( (i == cont - 1) && (strcmp(carpeta.b_content[m].b_name,auxString.c_str()) == 0)){
                            return carpeta.b_content[m].b_inodo;
                        }
                        else if( (i != cont - 1) && (strcmp(carpeta.b_content[m].b_name,auxString.c_str()) == 0)){
                            byteInodo = super.s_inode_start + sizeof(InodeTable)*carpeta.b_content[m].b_inodo;
                            flag = true;
                            break;
                        }
                    }
                }
                //Apuntador indirecto
                else if(j == 12){

                    BloqueApuntadores apuntador;
                    fread(&apuntador,sizeof(BloqueApuntadores),1,file);
                    for(int m = 0; m < 16; m++){
                        if(apuntador.b_pointers[m] != -1){

                            byteBloque = super.s_block_start + sizeof(BloqueCarpetas)*apuntador.b_pointers[m];
                            fseek(file,byteBloque,SEEK_SET);
                            fread(&carpeta,sizeof(BloqueCarpetas),1,file);

                            for (int m = 0; m < 4; m++) {

                                list<string>::iterator iterador = lista.begin();
                                for(int it=0; it<i; it++){
                                    it++;
                                }
                                auxString = *iterador;

                                if((i == cont - 1) && (strcmp(carpeta.b_content[m].b_name,auxString.c_str()) == 0)){
                                    return carpeta.b_content[m].b_inodo;
                                }
                                else if((i != cont - 1) && (strcmp(carpeta.b_content[m].b_name,auxString.c_str()) == 0)){
                                    byteInodo = super.s_inode_start + sizeof(InodeTable)*carpeta.b_content[m].b_inodo;
                                    flag = true;
                                    break;
                                }
                            }
                            if(flag){
                                break;
                            }
                        }
                    }
                }

                if(flag){
                    break;
                }
            }
        }
    }

    return -1;
}

bool CAT_::mostrarContenidoArchivo(FILE *file, char* path){
    int inodoNum = buscarCarpetaArchivo(file,path);
    if(inodoNum != -1){
        
        SuperBloque super;
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable)*inodoNum,SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        bool auxBool1=inodo.i_uid == sesion.user;
        bool auxBool2=inodo.i_gid == sesion.group;
        bool permisos = permisosDeLectura(inodo.i_perm,auxBool1,auxBool2);


        if( (sesion.user == 1 && sesion.group == 1) || permisos ){

            string contentStr = "";
            for (int i = 0; i < 15; i++) {
                if(inodo.i_block[i] != -1){
                    if(i < 12){
                        BloqueArchivos archivo;
                        fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[i],SEEK_SET);
                        fread(&archivo,sizeof(BloqueCarpetas),1,file);
                        contentStr += archivo.b_content;
                    }
                    else if(i == 12){
                        BloqueApuntadores apuntador;
                        fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[i],SEEK_SET);
                        fread(&apuntador,sizeof(BloqueApuntadores),1,file);
                        for(int j = 0; j < 16; j++){
                            if(apuntador.b_pointers[j] != -1){
                                BloqueArchivos archivo;
                                fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*apuntador.b_pointers[j],SEEK_SET);
                                fread(&archivo,sizeof(BloqueArchivos),1,file);
                                contentStr += archivo.b_content;
                            }

                        }
                    }
                }
            }
            cout<< "\u001B[35m" << "--------------------------Contenido de "<<path<<" --------------------------"<<"\x1B[0m" <<endl;
            cout<< "\u001B[35m" << contentStr <<"\x1B[0m" <<endl;
            cout<< "\u001B[35m" << "----------------------------------------------------------------------------"<<"\x1B[0m" <<endl;
        }
        else{
            cout<< "\u001B[31m" << "[BAD CONTEXT] No cuenta con los permisos de lectura"<< "\x1B[0m" <<endl;
        }
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] No existe archivo en la ruta "<<path<< "\x1B[0m" <<endl;
    }


}

bool CAT_::permisosDeLectura(int permisos, bool flagUsr, bool flagGrp){
    string aux = to_string(permisos);
    int propietario = aux[0] - '0';
    int grupo = aux[1] - '0';
    int otros = aux[2] - '0';

    if( (propietario >= 3) && flagUsr){
        return true;
    }
    else if( (grupo >= 3) && flagGrp){
        return true;
    }
    else if(otros >= 3){
        return true;
    }

    return false;
}