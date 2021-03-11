#pragma once
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
#include <libgen.h>

using namespace std;

extern list<MOUNT_> *mounted;
extern bool loged;
extern Sesion sesion;
enum returnType {fileExist,fileCreated,badPermissions,badContent,badPath,folderCreated};

class MKFILE_{
private:
        string path;//Ruta donde se crea el archivo
        string cont;//Ruta del archivo que contiene el contenido del nuevo archivo
        int size; //Tamaño del nuevo archivo
        bool rParam; //Parametro R, que crea carpertas si no existen
        bool statusFlag; // Indica si hay errores en el comando
        
public:

    MKFILE_(){
        this->path="";
        this->cont="";
        this->size=0;
        this->rParam=false;
        this->statusFlag=true;
    }

    /**
     * Setter del Path
     * @param value: Ruta del nuevo archivo
    */
    void setPath(char *value);

    /**
     * Setter del Cont
     * @param value: Ruta (real) del archivo
    */
    void setCont(char *value);

    /**
     * Setter del Size
     * @param value: Tamaño en bytes del nuevo archivo
    */
    void setSize(char *value);

    /**
     * Setter del parametro R
    */
    void setRParam();

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
     * Crea un archivo nuevo
     * @param index = Indice del inodo donde se creara el archivo
     * @param tempPath = Path donde se creara el archivo
     * @return Enum de returnType 
    */
    returnType nuevoArchivo(int index, char* tempPath);

    /**
     * Busca si hay un bloque de archivo libre en cierto inodo
     * @param file = Indice del inodo donde se comienza
     * @param numInodo = Indice del inodo
     * @param content = Apuntador donde se guardara el indice en el bloque de carpetas donde esta el bloque libre libre si hay
     * @param bloque = Apuntador donde se guardara el indice del bloque de carpetas libre si hay
     * @param pointer = Apuntador donde se guardara el indice del bloque de apuntadores libre si hay
     * @param inodo = Apuntador al struct del Inodo donde esta el bloque libre
     * @param carpeta = Apuntador al struct del Bloque de Carpetas donde esta el bloque libre
     * @param apuntadores = Apuntador al struct del Bloque de Apuntadores donde esta el bloque libre
     * @return Verdadero si hay un bloque libre
    */
    bool getBloqueArchivo(FILE* file,int indexInodo,int &content,int &bloque,int &pointer,InodeTable &inodo,BloqueCarpetas &carpeta, BloqueApuntadores &apuntadores);

    /**
     * Vericia si el usuario tienen permisos para crear el archivo
     * @param int permisos = permisos del usuario
     * @param bool flagUser = indica si es propietario del archivo
     * @param bool flagGroup = indica si es pertenece al grupo del archivo
     * @return True si tiene permisos y flase si no los tiene
    */
    bool permisoDeEscritura(int permisos, bool flagUser, bool flagGroup);

    /**
     * Regresa el bit libre en el bitmap de inodos o bloques segun el ajuste
     * @param FILE fp: stream del archivo que se esta leyendo
     * @param char tipo: tipo de bit a buscar (Inodo/Bloque)
     * @param char fit: ajuste de la particion 
    */
    int buscarBit(FILE *fp,char fit,char tipo);

    /**
     * Crea una carpeta nueva
     * @param index = Indice del inodo donde se creara el archivo
     * @param tempPath = Path donde se creara la carpeta
     * @return Enum de returnType
    */
    returnType nuevaCarpeta(FILE *file, char *tempPath, int index);

    /**
     *Guarda una operacion en el Journal 
     * @param operacion: Nombre de la operacion
     * @param path: Path de la operacion
     * @param content: Data para operaciones en users.txt o mkfile int
     */
    void guardarJournal(char *operacion, char *path, char *content);


};

void MKFILE_::setPath(char *value){
    this->path=value;
}

void MKFILE_::setCont(char *value){
    this->cont=value;
}

void MKFILE_::setSize(char* value){
    this->size = atoi(value);
}

void MKFILE_::setRParam(){
    this->rParam=true;
}

void MKFILE_::setStatus(){
    if(this->path == ""){
        this->statusFlag=false;
        cout<< "\u001B[31m" << "[BAD PARAM] Path no valido" << "\x1B[0m" << endl;
    }
    if(this->size < 0){
        this->statusFlag=0;
        cout << "\u001B[31m" << "[BAD PARAM] Size no valido" << "\x1B[0m" << endl;
    }
    if(this->cont !=""){
        FILE *file = fopen(this->cont.c_str(),"r");
        if(file==NULL){
            this->statusFlag=0;
            cout << "\u001B[31m" << "[BAD PARAM] No existe archivo en la ruat de -cont" << "\x1B[0m" << endl;
            return;
        }
        fclose(file);
    }
}

void MKFILE_::init (){
    setStatus();
    if(this->statusFlag){
        if(loged){

            FILE *file = fopen(sesion.direccion.c_str(),"rb+");
            int res = -1;

            if(file!=NULL){
                SuperBloque super;
                fseek(file,sesion.superStart,SEEK_SET);
                fread(&super,sizeof(SuperBloque),1,file);

                char auxPath[600];
                strcpy(auxPath,this->path.c_str());
                if(buscarCarpetaArchivo(file,auxPath) != -1){
                    // PREGUNTAR SI DESEA SOBREESCRIBIR EL ARCHIVO
                    cout<< "\u001B[33m" << "[WARNING] El archivo ya existe. ¿Desea sobreescribirlo ? Y/N "<< "\x1B[0m" << endl;
                }
                else{
                    fclose(file);
                    strcpy(auxPath,path.c_str());
                    res = nuevoArchivo(0,auxPath);

                    switch (res){

                    case fileCreated:
                        cout<< "\u001B[32m" << "[OK] El archivo se ha creado exitosamente"<< "\x1B[0m" <<endl;
                        if(sesion.sistemaType == 3){
                            char operacion[10];
                            char pathChar[500];
                            char content[500];
                            strcpy(operacion,"mkfile");
                            strcpy(pathChar,this->path.c_str());
                            if(this->cont.length() != 0){
                                strcpy(content,this->cont.c_str());
                                guardarJournal(operacion,pathChar,content);
                            }
                            else{
                                strcpy(content,to_string(this->size).c_str());
                                guardarJournal(operacion,pathChar,content);
                            }
                        }
                        break;
                    case badPermissions:
                        cout<< "\u001B[31m" << "[BAD CONTEXT] No cuenta con los permisos de escritura necesarios"<< "\x1B[0m" <<endl;
                        break;
                    case badContent:
                        cout<< "\u001B[31m" << "[BAD PARAM] El archivo especificado por el parametro -cont no existe"<< "\x1B[0m" <<endl;
                        break;
                    case badPath:
                        cout<< "\u001B[32m" << "[BAD PARAM] La ruta especificada por el parametro -path no existe"<< "\x1B[0m" <<endl;
                        break;
                    default:
                        cout<<"ESTO NO DEBERIA IMPRIMIRSE NUNCA"<<endl;
                        break;
                    }

                }
                
            }

        }
        else{
            cout<< "\u001B[31m" << "[BAD CONTEXT] Tiene que iniciar sesion para ejecutar este comando "<< "\x1B[0m" << endl;
        }
    }
}

int MKFILE_::buscarCarpetaArchivo(FILE *file, char* path){

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

returnType MKFILE_::nuevoArchivo(int index, char *tempPath){

    FILE *file = fopen(sesion.direccion.c_str(),"rb+");
    string defaultContent = "0123456789";
    list<string> lista = list<string>();

    char auxPath[500];
    strcpy(auxPath,tempPath);

    char dirName[500];
    strcpy(dirName,dirname(auxPath));

    char nombreAC[100];
    strcpy(auxPath,tempPath);
    strcpy(nombreAC,basename(auxPath));

    string content = "";
    if(this->cont.length() != 0){
        FILE *newContent = fopen(this->cont.c_str(),"r");
        if(newContent!=NULL){
            fseek(newContent,0,SEEK_SET);
            for (int i = 0; i < this->size; i++){
                content += fgetc(newContent);
            }
            fseek(newContent,0,SEEK_END);
            this->size = ftell(newContent);
            fclose(newContent);
        }
        else{
            fclose(newContent);
            return badContent;
        }
    }

    //Se separan los nombres de las directorios si se necesitara crearlos
    strcpy(auxPath,tempPath);
    char *token = strtok(auxPath,"/");
    int auxCont = 0;
    while(token != NULL){
        auxCont++;
        lista.push_back(token);
        token = strtok(NULL,"/");
    }
    strcpy(auxPath,tempPath);

    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    InodeTable inodo,newInodo;
    BloqueCarpetas carpetas, carpetaAux ,newCarpetas;
    BloqueApuntadores apuntadores;
    
    //Una carpeta
    if(auxCont == 1){
        int contentP = 0;
        int apuntadorLibre = 0;
        int pointer = 0;
        char myChar;
        if(getBloqueArchivo(file,index,contentP,apuntadorLibre,pointer,inodo,carpetas,apuntadores)){

            bool auxBool1=inodo.i_uid == sesion.user;
            bool auxBool2=inodo.i_gid == sesion.group;
            bool permisos = permisoDeEscritura(inodo.i_perm,auxBool1,auxBool2);

            if((sesion.user == 1 && sesion.group == 1) || permisos ){

                int bitInodo = buscarBit(file,sesion.fit,'I');

                //Se configura y escribe el nuevo inodo
                newInodo.i_uid = sesion.user;
                newInodo.i_gid = sesion.group;
                newInodo.i_size = 0;
                newInodo.i_atime = time(nullptr);
                newInodo.i_ctime = time(nullptr);
                newInodo.i_mtime = time(nullptr);
                for(int i = 0; i < 15; i++){
                    newInodo.i_block[i] = -1;
                }  
                newInodo.i_type = '1';
                newInodo.i_perm = 664;
                fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                fwrite(&newInodo,sizeof(InodeTable),1,file);
                //Se registra el inodo en el bitmap de inodos
                fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                myChar='1';
                fwrite(&myChar,sizeof(char),1,file);

                //Se agrega el archivo al bloque de carpetas
                carpetas.b_content[contentP].b_inodo = bitInodo;
                index=bitInodo;
                strcpy(carpetas.b_content[contentP].b_name,nombreAC);
                fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[apuntadorLibre],SEEK_SET);
                fwrite(&carpetas,sizeof(BloqueCarpetas),1,file);

                if(size != 0){
                    double numBloques = ceil(size/64.00);
                    long contentChar = 0;
                    for (int i = 0; i < numBloques; i++) {
                        BloqueArchivos archivo;
                        for(int j=0;j<64;j++){
                            archivo.b_content[j]='\0';
                        }

                        if(size > 64){
                            for(int j = 0; j < 64; j++){
                                if(content.length() == 0){
                                    archivo.b_content[j] = defaultContent[contentChar];
                                    contentChar++;
                                    if(contentChar == 10){
                                        contentChar=0;
                                    }
                                }
                                else{
                                    archivo.b_content[j] = content[contentChar];
                                    contentChar++;
                                }
                            }                               
                            size = size - 64;
                        }
                        else{
                            for (int j = 0; j < size; j++) {
                                if(content.length() == 0){
                                    archivo.b_content[j] = defaultContent[contentChar];
                                    contentChar++;
                                    if(contentChar == 10){
                                        contentChar=0;
                                    }
                                }
                                else{
                                    archivo.b_content[j] = content[contentChar];
                                    contentChar++;
                                }
                            }
                        }
                        
                        //Apuntadores Directos
                        if(i<12){
                            
                            int bitBloque = buscarBit(file,sesion.fit,'B');
                            //Se marca bloque en el bitmap
                            fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                            myChar='2';
                            fwrite(&myChar,sizeof(char),1,file);
                            
                            //Se guarda bloque en el inodo
                            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                            fread(&inodo,sizeof(InodeTable),1,file);
                            if(inodo.i_size == 0){
                                inodo.i_size = size+64;
                            }
                            inodo.i_block[i] = bitBloque; 
                            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                            fwrite(&inodo,sizeof(InodeTable),1,file);
                            //Se guarda el bloque
                            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                            fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                        }
                        //Apuntadore Indirecto Simple Primera Vez
                        else if(i == 12){

                            int bitBloqueApuntadores = buscarBit(file,sesion.fit,'B');

                            //Se guarda el bloque apuntador en el bitmap
                            fseek(file,super.s_bm_block_start + bitBloqueApuntadores,SEEK_SET);
                            myChar='3';
                            fwrite(&myChar,sizeof(char),1,file);
                            //Se guarda el bloque en el inodo
                            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                            fread(&inodo,sizeof(InodeTable),1,file);
                            inodo.i_block[i] = bitBloqueApuntadores;
                            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                            fwrite(&inodo,sizeof(InodeTable),1,file);                            
                            
                            //Se crea el bloque de apuntadores
                            int bitBloque = buscarBit(file,sesion.fit,'B');
                            apuntadores.b_pointers[0] = bitBloque;
                            fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                            myChar='2';
                            fwrite(&myChar,sizeof(char),1,file);
                            for(int i = 1; i < 16; i++){
                                apuntadores.b_pointers[i] = -1;
                            }
                            fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloqueApuntadores,SEEK_SET);
                            fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                            //Se guarda el bloque
                            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                            fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                        }
                        //Apuntador Indirecto Simple n-vez
                        else if(i > 12 && i < 28){

                            //Se lee Inodo
                            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                            fread(&inodo,sizeof(InodeTable),1,file);
                            //Se lee bloque de apuntadores
                            fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                            fread(&apuntadores,sizeof(BloqueApuntadores),1,file);                           
                            
                            int bitBloque = buscarBit(file,sesion.fit,'B');
                            //Se crean los bloques de archivos y se registran en el bitmap
                            fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                            myChar = '2';
                            fwrite(&myChar,sizeof(char),1,file);
                            //Se configura y reescribe bloque de apuntadores
                            int bloqueLibre = 0;
                            for (int m = 0; m < 16; m++) {
                                if(apuntadores.b_pointers[m] == -1){
                                    bloqueLibre = m;
                                    break;
                                }
                            }
                            apuntadores.b_pointers[bloqueLibre] = bitBloque;
                            fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                            fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);
                           
                            //Se guarda el bloque
                            fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                            fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                        }

                    }
                    //Se reescribe el superbloque
                    super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                    super.s_first_blo = super.s_first_blo + numBloques;
                }
                //Se reescribe el superbloque
                super.s_first_ino = super.s_first_ino + 1;
                super.s_free_inodes_count = super.s_free_inodes_count - 1;
                fseek(file,sesion.superStart,SEEK_SET);
                fwrite(&super,sizeof(SuperBloque),1,file);
                return fileCreated;
            }
            else{
                return badPermissions;
            }
        }
        //Los bloques carpetas estan llenos
        else{
            pointer=-1;
            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
            fread(&inodo,sizeof(InodeTable),1,file);
            //Se busca un apuntador libre en el inodo
            for (int i = 0; i < 15; i++) {
                if(i<12){
                    if(inodo.i_block[i] == -1){
                        apuntadorLibre = i;
                        break;
                    }
                }
                else if(i == 12){
                    if(inodo.i_block[i] == -1){
                        apuntadorLibre = 12;
                        break;
                    }
                    else{
                        fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                        fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                        for(int j = 0; j < 16; j++){
                            if(apuntadores.b_pointers[j] == -1){
                                apuntadorLibre = 12;
                                pointer = j;
                                break;
                            }
                        }
                        if(pointer!= -1){
                            break;
                        }
                    }
                }
            }

            bool auxBool1=inodo.i_uid == sesion.user;
            bool auxBool2=inodo.i_gid == sesion.group;
            bool permisos = permisoDeEscritura(inodo.i_perm,auxBool1,auxBool2);
            if((sesion.user == 1 && sesion.group == 1) || permisos ){

                //Apuntadores Directos
                if(apuntadorLibre < 12){

                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    //Se actualiza bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se guarda el bloque en el inodo
                    inodo.i_block[apuntadorLibre] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable) * index,SEEK_SET);
                    fwrite(&inodo,sizeof(InodeTable),1,file);

                    
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    //Se registra inodo en el bitmap
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se crea bloque de carpetas
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    index=bitInodo;
                    strcpy(newCarpetas.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    
                    //Se configura y escribe el nuevo inodo
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '1';
                    newInodo.i_perm = 664;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);

                    if(size != 0){
                        double numBloques = ceil(size/64.00);
                        long contentChar = 0;

                        for (int i = 0; i < numBloques; i++) {
                            BloqueArchivos archivo;
                            for(int j=0;j<64;j++){
                                archivo.b_content[j]='\0';
                            }

                            if(size > 64){
                                for(int j = 0; j < 64; j++){
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }                               
                                size = size - 64;
                            }
                            else{
                                for (int j = 0; j < size; j++) {
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }
                            }
                            
                            //Apuntadores Directos
                            if(i<12){
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se marca bloque en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                
                                //Se guarda bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                if(inodo.i_size == 0){
                                    inodo.i_size = size+64;
                                }
                                inodo.i_block[i] = bitBloque; 
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntadore Indirecto Simple Primera Vez
                            else if(i == 12){

                                int bitBloqueApuntadores = buscarBit(file,sesion.fit,'B');

                                //Se guarda el bloque apuntador en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloqueApuntadores,SEEK_SET);
                                myChar='3';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se guarda el bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                inodo.i_block[i] = bitBloqueApuntadores;
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                
                                
                                //Se crea el bloque de apuntadores
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                apuntadores.b_pointers[0] = bitBloque;
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                for(int i = 1; i < 16; i++){
                                    apuntadores.b_pointers[i] = -1;
                                }
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloqueApuntadores,SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntador Indirecto Simple n-vez
                            else if(i > 12 && i < 28){

                                //Se lee Inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                //Se lee bloque de apuntadores
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                                int bloqueLibre = 0;
                                for (int m = 0; m < 16; m++) {
                                    if(apuntadores.b_pointers[m] == -1){
                                        bloqueLibre = m;
                                        break;
                                    }
                                }
                                
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se crean los bloques de archivos y se registran en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar = '2';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se configura y reescribe bloque de apuntadores
                                apuntadores.b_pointers[bloqueLibre] = bitBloque;
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }

                        }
                        //Se reescribe sl superbloque
                        super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                        super.s_first_blo = super.s_first_blo + numBloques;
                    }
                    //Se reescribe sl superbloque
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return fileCreated;

                }
                //Apuntador Indirecto Simple Primera Vez
                else if(apuntadorLibre ==12 && pointer==-1){

                    //Se guarda bloque en el inodo
                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    inodo.i_block[apuntadorLibre] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                    fwrite(&inodo,sizeof(InodeTable),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='3';
                    fwrite(&myChar,sizeof(char),1,file);

                    
                    int bitBloqueCarpeta = buscarBit(file,sesion.fit,'B');
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloqueCarpeta,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se crea el bloque de apuntadores
                    apuntadores.b_pointers[0] = bitBloqueCarpeta;
                    for(int i = 1; i < 16; i++){
                        apuntadores.b_pointers[i] = -1;
                    }
                    fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloque,SEEK_SET);
                    fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                    
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    //Se marca en el bitmap de inodos
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se crea la el bloque carpeta del apuntador
                    for(int i = 0; i < 4; i++){
                        strcpy(carpetaAux.b_content[i].b_name,"");
                        carpetaAux.b_content[i].b_inodo = -1;
                    }
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    index=bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloqueCarpeta,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BloqueCarpetas),1,file);                   

                    //Se crea el nuevo inodo del archivo
                    bitBloque = buscarBit(file,sesion.fit,'B');
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '0';
                    newInodo.i_perm = 664;
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    
                    if(size != 0){
                        double numBloques = ceil(size/64.00);
                        long contentChar = 0;

                        for (int i = 0; i < numBloques; i++) {
                            BloqueArchivos archivo;
                            for(int j=0;j<64;j++){
                                archivo.b_content[j]='\0';
                            }

                            if(size > 64){
                                for(int j = 0; j < 64; j++){
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }                               
                                size = size - 64;
                            }
                            else{
                                for (int j = 0; j < size; j++) {
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }
                            }
                            
                            //Apuntadores Directos
                            if(i<12){
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se marca bloque en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                
                                //Se guarda bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                if(inodo.i_size == 0){
                                    inodo.i_size = size+64;
                                }
                                inodo.i_block[i] = bitBloque; 
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntadore Indirecto Simple Primera Vez
                            else if(i == 12){

                                int bitBloqueApuntadores = buscarBit(file,sesion.fit,'B');

                                //Se guarda el bloque apuntador en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloqueApuntadores,SEEK_SET);
                                myChar='3';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se guarda el bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                inodo.i_block[i] = bitBloqueApuntadores;
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                
                                
                                //Se crea el bloque de apuntadores
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                apuntadores.b_pointers[0] = bitBloque;
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                for(int i = 1; i < 16; i++){
                                    apuntadores.b_pointers[i] = -1;
                                }
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloqueApuntadores,SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntador Indirecto Simple n-vez
                            else if(i > 12 && i < 28){

                                //Se lee Inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                //Se lee bloque de apuntadores
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                                int bloqueLibre = 0;
                                for (int m = 0; m < 16; m++) {
                                    if(apuntadores.b_pointers[m] == -1){
                                        bloqueLibre = m;
                                        break;
                                    }
                                }
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se crean los bloques de archivos y se registran en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar = '2';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se configura y reescribe bloque de apuntadores
                                apuntadores.b_pointers[bloqueLibre] = bitBloque;
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }

                        }
                        //Se reescribe sl superbloque
                        super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                        super.s_first_blo = super.s_first_blo + numBloques;
                    }
                    //Se reescribe sl superbloque
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return fileCreated;


                }
                //Apuntador Indirecto Simple n-vez
                else if(apuntadorLibre == 12 && pointer!=-1){

                    
                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se registra bloque en el bloque de apuntadores
                    apuntadores.b_pointers[pointer] = bitBloque;
                    fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                    fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                    
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    //Se marca en el bitmap de inodos
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se crea el bloque de carpetas
                    for(int i = 0; i < 4; i++){
                        strcpy(carpetaAux.b_content[i].b_name,"");
                        carpetaAux.b_content[i].b_inodo = -1;
                    }
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    index=bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BloqueCarpetas),1,file);
                    

                    //Se crea el nuevo Inodo
                    bitBloque = buscarBit(file,sesion.fit,'B');//Carpeta
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '0';
                    newInodo.i_perm = 664;
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);

                    if(size != 0){
                        double numBloques = ceil(size/64.00);
                        long contentChar = 0;

                        for (int i = 0; i < numBloques; i++) {
                            BloqueArchivos archivo;
                            for(int j=0;j<64;j++){
                                archivo.b_content[j]='\0';
                            }

                            if(size > 64){
                                for(int j = 0; j < 64; j++){
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }                               
                                size = size - 64;
                            }
                            else{
                                for (int j = 0; j < size; j++) {
                                    if(content.length() == 0){
                                        archivo.b_content[j] = defaultContent[contentChar];
                                        contentChar++;
                                        if(contentChar == 10){
                                            contentChar=0;
                                        }
                                    }
                                    else{
                                        archivo.b_content[j] = content[contentChar];
                                        contentChar++;
                                    }
                                }
                            }
                            
                            //Apuntadores Directos
                            if(i<12){
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se marca bloque en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                
                                //Se guarda bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                if(inodo.i_size == 0){
                                    inodo.i_size = size+64;
                                }
                                inodo.i_block[i] = bitBloque; 
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntadore Indirecto Simple Primera Vez
                            else if(i == 12){

                                int bitBloqueApuntadores = buscarBit(file,sesion.fit,'B');

                                //Se guarda el bloque apuntador en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloqueApuntadores,SEEK_SET);
                                myChar='3';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se guarda el bloque en el inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                inodo.i_block[i] = bitBloqueApuntadores;
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fwrite(&inodo,sizeof(InodeTable),1,file);
                                
                                
                                //Se crea el bloque de apuntadores
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                apuntadores.b_pointers[0] = bitBloque;
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar='2';
                                fwrite(&myChar,sizeof(char),1,file);
                                for(int i = 1; i < 16; i++){
                                    apuntadores.b_pointers[i] = -1;
                                }
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloqueApuntadores,SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }
                            //Apuntador Indirecto Simple n-vez
                            else if(i > 12 && i < 28){

                                //Se lee Inodo
                                fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                                fread(&inodo,sizeof(InodeTable),1,file);
                                //Se lee bloque de apuntadores
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                                int bloqueLibre = 0;
                                for (int m = 0; m < 16; m++) {
                                    if(apuntadores.b_pointers[m] == -1){
                                        bloqueLibre = m;
                                        break;
                                    }
                                }
                                
                                
                                int bitBloque = buscarBit(file,sesion.fit,'B');
                                //Se crean los bloques de archivos y se registran en el bitmap
                                fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                                myChar = '2';
                                fwrite(&myChar,sizeof(char),1,file);
                                //Se configura y reescribe bloque de apuntadores
                                apuntadores.b_pointers[bloqueLibre] = bitBloque;
                                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                                fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);
                   
                                //Se guarda el bloque
                                fseek(file,super.s_block_start + sizeof(BloqueArchivos)*bitBloque,SEEK_SET);
                                fwrite(&archivo,sizeof(BloqueArchivos),1,file);

                            }

                        }
                        //Se reescribe sl superbloque
                        super.s_free_blocks_count = super.s_free_blocks_count - numBloques;
                        super.s_first_blo = super.s_first_blo + numBloques;                        
                    }
                    //Se reescribe sl superbloque
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return fileCreated;
                    
                }

            }
            else{
                return badPermissions;
            }
        }
    }
    //Hay que crear las carpetas
    else{
        int auxindex = buscarCarpetaArchivo(file,dirName);

        if(auxindex == -1){       
            if(rParam){
                string auxStr1,auxStr2;
                auxindex=0;
                for (int i = 0; i < auxCont; i++) {
                    auxStr1 = "";
                    if(i == auxCont-1){
                        char newPath[200];
                        for(int j=0; j<200;j++){
                            newPath[j]='\0';
                        }
                        strcat(newPath,"/");
                        strcat(newPath,nombreAC);

                        return nuevoArchivo(auxindex,newPath);
                    }
                    else{
                        list<string>::iterator it = lista.begin();
                        for(int j=0; j<i; j++){
                            it++;
                        }
                        auxStr1 += "/"+*it;
                        auxStr2 += "/"+*it;
                        char dir[500];
                        strcpy(dir,auxStr1.c_str());
                        int carpeta = buscarCarpetaArchivo(file,dir);
                        if(carpeta == -1){
                            strcpy(dir,auxStr1.c_str());
                            nuevaCarpeta(file,dir,auxindex);
                            strcpy(dir,auxStr2.c_str());
                            auxindex = buscarCarpetaArchivo(file,dir);
                        }
                        else{
                            auxindex = carpeta;
                        }   
                    }
                }   
            }
            else{
                return badPath;
            }
        }
        else{
            char newPath[150];
            for(int j=0;j<150;j++){
                newPath[j]='\0';
            }
            strcat(newPath,"/");
            strcat(newPath,nombreAC);

            return nuevoArchivo(auxindex,newPath);
        }
    }

    return fileExist;
}

bool MKFILE_::getBloqueArchivo(FILE* file,int indexInodo,int &content,int &bloque,int &pointer,InodeTable &inodo,BloqueCarpetas &carpeta, BloqueApuntadores &apuntadores){
    
    //Se lee superbloque
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);
    
    //Se lee inodo
    fseek(file,super.s_inode_start + sizeof(InodeTable)*indexInodo,SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,file);
    //Se busca un espacio libre 
    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            
            if(i<12){
                fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[i],SEEK_SET);
                fread(&carpeta,sizeof(BloqueCarpetas),1,file);
                for(int j = 0; j < 4; j++){
                    //Si se encontro un espacio libre
                    if(carpeta.b_content[j].b_inodo == -1){
                        bloque = i;
                        content = j;
                        return true;
                    }
                }
            }
            else if(i == 12){
                fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[i],SEEK_SET);
                fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                for(int j = 0; j < 16; j++){
                    if(apuntadores.b_pointers[j] != -1){
                        fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*apuntadores.b_pointers[j],SEEK_SET);
                        fread(&carpeta,sizeof(BloqueCarpetas),1,file);
                        for(int m = 0; m < 4; m++){
                            if(carpeta.b_content[m].b_inodo == -1){
                                bloque = i;
                                pointer = j;
                                content = m;
                                return true;
                            }
                        }
                    }
                }
            }
        }

    }
    return false;
}

bool MKFILE_::permisoDeEscritura(int permisos, bool flagUser, bool flagGroup){
    string aux = to_string(permisos);
    char propietario = aux[0];
    char grupo = aux[1];
    char otros = aux[2];

    if(flagUser && (propietario == '2' || propietario == '3' || propietario == '6' || propietario || '7') ){
        return true;
    }
    else if((grupo == '2' || grupo == '3' || grupo == '6' || grupo == '7') && flagGroup){
        return true;
    }
    else if(otros == '2' || otros == '3' || otros == '6' || otros == '7'){
        return true;
    }

    return false;
}

int MKFILE_::buscarBit(FILE *file,char fit,char tipo){

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

returnType MKFILE_::nuevaCarpeta(FILE *file, char *tempPath, int index){
    list<string> lista = list<string>();

    char auxPath[500];
    strcpy(auxPath,tempPath);

    char dirName[500];
    strcpy(dirName,dirname(auxPath));

    char nombreAC[100];
    strcpy(auxPath,tempPath);
    strcpy(nombreAC,basename(auxPath));

    //Se separan los nombres de las directorios si se necesitara crearlos
    strcpy(auxPath,tempPath);
    char *token = strtok(tempPath,"/");
    int auxCont = 0;
    
    while(token != nullptr){
        auxCont++;
        lista.push_back(token);
        token = strtok(nullptr,"/");
    }
    strcpy(auxPath,tempPath);
    
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);
   
    InodeTable inodo,newInodo;
    BloqueCarpetas carpetas, carpetaAux, newCarpetas ;
    BloqueApuntadores apuntadores;

    //Una carpeta
    if(auxCont == 1){
        int contentP = 0;
        int apuntadorLibre = 0;
        int pointer = 0;
        char myChar;
        if(getBloqueArchivo(file,index,contentP,apuntadorLibre,pointer,inodo,carpetas,apuntadores)){
            bool auxBool1=inodo.i_uid == sesion.user;
            bool auxBool2=inodo.i_gid == sesion.group;
            bool permisos = permisoDeEscritura(inodo.i_perm,auxBool1,auxBool2);

            if((sesion.user == 1 && sesion.group == 1) || permisos ){
                
                //Apuntadores Directos
                if (apuntadorLibre<12){
                    int bitInodo = buscarBit(file,sesion.fit,'I');

                    //Se agrega la carpeta en el bloque de carpetas
                    carpetas.b_content[contentP].b_inodo = bitInodo;
                    strcpy(carpetas.b_content[contentP].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[apuntadorLibre],SEEK_SET);
                    fwrite(&carpetas,sizeof(BloqueCarpetas),1,file);

                    //Se crea el nuevo inodo
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '0';
                    newInodo.i_perm = 664;

                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    //Se marca inodo en el bitmap
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el bloque de carpeta
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    //Se configura padre y actual
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    strcpy(newCarpetas.b_content[0].b_name,".");
                    newCarpetas.b_content[1].b_inodo = index;            
                    strcpy(newCarpetas.b_content[1].b_name,"..");
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    //Se marca bloque en el bitmap
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se reescribe el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 1;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return folderCreated;
                }
                //Apuntador indirecto simple
                if(apuntadorLibre == 12){
                    

                    int bitInodo = buscarBit(file,sesion.fit,'I');

                    //Se marca inodo en el bitmap
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se agrega la carpeta en el bloque de carpetas
                    carpetas.b_content[contentP].b_inodo = bitInodo;
                    strcpy(carpetas.b_content[contentP].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*inodo.i_block[pointer],SEEK_SET);
                    fwrite(&carpetas,sizeof(BloqueCarpetas),1,file);


                    //Se crea el nuevo inodo
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '0';
                    newInodo.i_perm = 664;

                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    //Se marca bloque en el bitmap
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    

                    //Se crea el bloque de carpeta
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    //Se configura padre y actual
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    strcpy(newCarpetas.b_content[0].b_name,".");
                    newCarpetas.b_content[1].b_inodo = index;            
                    strcpy(newCarpetas.b_content[1].b_name,"..");
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    

                    //Se reescribe el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 1;
                    super.s_free_blocks_count = super.s_free_blocks_count - 1;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return folderCreated;

                }

            }
            else{
                return badPermissions;
            }
        }
        //Todos bloques estan llenos
        else{
            pointer = -1;
            fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
            fread(&inodo,sizeof(InodeTable),1,file);
            for (int i = 0; i < 15; i++) {
                if(i<12){
                    if(inodo.i_block[i] == -1){
                        apuntadorLibre = i;
                        break;
                    }
                }
                else if(i == 12){
                    if(inodo.i_block[i] == -1){
                        apuntadorLibre = 12;
                        break;
                    }
                    else{
                        fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                        fread(&apuntadores,sizeof(BloqueApuntadores),1,file);
                        for(int j = 0; j < 16; j++){
                            if(apuntadores.b_pointers[j] == -1){
                                apuntadorLibre = 12;
                                pointer = j;
                                break;
                            }
                        }
                        if(pointer!= -1){
                            break;
                        }
                    }
                }
            }
            bool auxBool1=inodo.i_uid == sesion.user;
            bool auxBool2=inodo.i_gid == sesion.group;
            bool permisos = permisoDeEscritura(inodo.i_perm,auxBool1,auxBool2);
            if((sesion.user == 1 && sesion.group == 1) || permisos ){

                //apuntador Directo
                if(apuntadorLibre<12){
                    
                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    //Se agrega bloque a inodo y se reescribe
                    inodo.i_block[apuntadorLibre] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                    fwrite(&inodo,sizeof(InodeTable),1,file);

                    //Ser crea el bloque de carpetas
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    for(int i = 0; i < 4; i++){
                        strcpy(carpetaAux.b_content[i].b_name,"");
                        carpetaAux.b_content[i].b_inodo = -1;
                    }
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BloqueCarpetas),1,file);
                    //Se marca bit en bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el nuevo inodo
                    bitBloque = buscarBit(file,sesion.fit,'B');
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '1';
                    newInodo.i_perm = 664;
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    //Se marca bit en el bitmap de inodos
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el nuevo bloque de carpetas
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    strcpy(newCarpetas.b_content[0].b_name,".");
                    newCarpetas.b_content[1].b_inodo = index;
                    strcpy(newCarpetas.b_content[1].b_name,"..");
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se reescribe el super bloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 2;
                    super.s_free_blocks_count = super.s_free_blocks_count - 2;
                    
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return folderCreated;


                }
                //Apuntador Indirecto Simple Primer Vez
                if(apuntadorLibre == 12 && pointer==-1){

                    //Se guarda bloque en el inodo
                    int bitBloque = buscarBit(file,sesion.fit,'B');//Apuntador
                    inodo.i_block[apuntadorLibre] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*index,SEEK_SET);
                    fwrite(&inodo,sizeof(InodeTable),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='3';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el bloque de apuntadores
                    int bitBloqueCarpeta = buscarBit(file,sesion.fit,'B');
                    apuntadores.b_pointers[0] = bitBloqueCarpeta;
                    for(int i = 1; i < 16; i++){
                        apuntadores.b_pointers[i] = -1;
                    }
                    fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*bitBloque,SEEK_SET);
                    fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                    //Se crea la el bloque carpeta del apuntador
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    for(int i = 0; i < 4; i++){
                        strcpy(carpetaAux.b_content[i].b_name,"");
                        carpetaAux.b_content[i].b_inodo = -1;
                    }
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloqueCarpeta,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BloqueCarpetas),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloqueCarpeta,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el nuevo inodo de carpeta
                    bitBloque = buscarBit(file,sesion.fit,'B');
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '1';
                    newInodo.i_perm = 664;
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    //Se marca en el bitmap de inodos
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Creamos el nuevo bloque carpeta para el nuevo inodo
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    newCarpetas.b_content[1].b_inodo = index;
                    strcpy(newCarpetas.b_content[0].b_name,".");
                    strcpy(newCarpetas.b_content[1].b_name,"..");
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se reescribe el superbloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 3;
                    super.s_free_blocks_count = super.s_free_blocks_count - 3;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return folderCreated;

                }
                //Apuntador Indirecto Simple n-vez
                else if(apuntadorLibre == 12 && pointer!=-1){

                    //Se registra bloque en el bloque de apuntadores
                    int bitBloque = buscarBit(file,sesion.fit,'B');
                    apuntadores.b_pointers[pointer] = bitBloque;
                    fseek(file,super.s_block_start + sizeof(BloqueApuntadores)*inodo.i_block[12],SEEK_SET);
                    fwrite(&apuntadores,sizeof(BloqueApuntadores),1,file);

                    //Se crea el bloque de carpetas
                    int bitInodo = buscarBit(file,sesion.fit,'I');
                    for(int i = 0; i < 4; i++){
                        strcpy(carpetaAux.b_content[i].b_name,"");
                        carpetaAux.b_content[i].b_inodo = -1;
                    }
                    carpetaAux.b_content[0].b_inodo = bitInodo;
                    strcpy(carpetaAux.b_content[0].b_name,nombreAC);
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&carpetaAux,sizeof(BloqueCarpetas),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Se crea el nuevo Inodo
                    bitBloque = buscarBit(file,sesion.fit,'B');//Carpeta
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_size = 0;
                    newInodo.i_atime = time(nullptr);
                    newInodo.i_ctime = time(nullptr);
                    newInodo.i_mtime = time(nullptr);
                    for(int i = 0; i < 15; i++){
                        newInodo.i_block[i] = -1;
                    }  
                    newInodo.i_type = '1';
                    newInodo.i_perm = 664;
                    newInodo.i_uid = sesion.user;
                    newInodo.i_gid = sesion.group;
                    newInodo.i_block[0] = bitBloque;
                    fseek(file,super.s_inode_start + sizeof(InodeTable)*bitInodo,SEEK_SET);
                    fwrite(&newInodo,sizeof(InodeTable),1,file);
                    //Se marca en el bitmap de inodos
                    fseek(file,super.s_bm_inode_start + bitInodo,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);

                    //Creamos el nuevo bloque carpeta
                    for(int i = 0; i < 4; i++){
                        strcpy(newCarpetas.b_content[i].b_name,"");
                        newCarpetas.b_content[i].b_inodo = -1;
                    }
                    newCarpetas.b_content[0].b_inodo = bitInodo;
                    strcpy(newCarpetas.b_content[0].b_name,".");
                    newCarpetas.b_content[1].b_inodo = index;
                    strcpy(newCarpetas.b_content[1].b_name,"..");
                    fseek(file,super.s_block_start + sizeof(BloqueCarpetas)*bitBloque,SEEK_SET);
                    fwrite(&newCarpetas,sizeof(BloqueCarpetas),1,file);
                    //Se marca en el bitmap de bloques
                    fseek(file,super.s_bm_block_start + bitBloque,SEEK_SET);
                    myChar='1';
                    fwrite(&myChar,sizeof(char),1,file);
                    //Se reescribe el superbloque
                    super.s_free_inodes_count = super.s_free_inodes_count - 1;
                    super.s_first_ino = super.s_first_ino + 1;
                    super.s_first_blo = super.s_first_blo + 2;
                    super.s_free_blocks_count = super.s_free_blocks_count - 2;
                    fseek(file,sesion.superStart,SEEK_SET);
                    fwrite(&super,sizeof(SuperBloque),1,file);
                    return folderCreated;
                }

            }
        }
    }
    //Hay que crear las carpetas
    else{
        returnType res;
        int auxIndex = buscarCarpetaArchivo(file,dirName);
        if(auxIndex == -1){
            if(this->rParam){
                auxIndex = 0;
                string auxStr1 = "";
                for(int i = 0; i < auxCont; i++){
                    list<string>::iterator it = lista.begin();
                    for(int j=0; j<i; j++){
                        it++;
                    }
                    auxStr1 += "/"+*it;
                    char dir[500];
                    strcpy(dir,auxStr1.c_str());
                    int carpeta = buscarCarpetaArchivo(file,dir);
                    if(carpeta == -1){
                        strcpy(dir,auxStr1.c_str());
                        res = nuevaCarpeta(file,dir,auxIndex);
                        if(res == badPermissions){
                            return badPermissions;
                        }
                        strcpy(dir,auxStr1.c_str());
                        auxIndex = buscarCarpetaArchivo(file,dir);
                    }
                    else{
                        auxIndex = carpeta;
                    }
                        
                }
            }
            else{
                return badPath;
            }
        }
        else{
            char newPath[150];
            for(int j=0;j<150;j++){
                newPath[j]='\0';
            }
            strcat(newPath,"/");
            strcat(newPath,nombreAC);
            return nuevaCarpeta(file,newPath,auxIndex);
        }
        return res;
    }

    return badPath;
    
}

void MKFILE_::guardarJournal(char* operacion,char *path,char *content){
    
    
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
