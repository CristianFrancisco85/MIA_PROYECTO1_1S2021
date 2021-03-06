#ifndef LOGIN_H
#define LOGIN_H

#endif // LOGIN_H
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <parser.h>
#include <list>
#include <iterator>
#include <MOUNT.h>
#include <REP.h>

using namespace  std;

class LOGIN_{
private:
    string user;
    string password;
    string id;
    bool statusFlag;

public:
    LOGIN_(){
        this->user="";
        this->password="";
        this->id="";
        this->statusFlag=true;
    }

    /**
     *Setter del ID 
      * @param size: ID de la particion montada
     */
    void setId(char* value);

    /**
      *Setter del User 
      * @param Value: Nombre de usuario
     */
    void setUser(char* value);

    /**
     *Setter del Password 
      * @param size: Contraseña
     */
    void setPassword(char* value);

    /**
     *Inicia el Logeo 
     */
    void initLog();

    /**
     *Realiza el logueo
     */
    int makeLog();

     /**
      *Obtiene el fit de una particion logica
      * @param size: Path del disco
     */
    char getLogicPartFit(string path);

    /**
     * Busca el grupo
     * @param name: Nombre del grupo
     */
    int buscarGrupo(string name);

    /**
     * Realiza el logout
     */
    void logout();

    /**
    *  Verifica que no haya error en los parametros 
    */
    void setStatus();

};

extern list<MOUNT_> *mounted;
extern bool loged;
extern Sesion sesion;

void LOGIN_::setUser(char *value){
    this->user = value;
    if(this->user[0] == '\"'){
        this->user = this->user.substr(1,this->user.length()-2);
    }
}

void LOGIN_::setPassword(char *value){
    this->password = value;
    if(this->password[0] == '\"'){
        this->password = this->password.substr(1,this->password.length()-2);
    }
}

void LOGIN_::setId(char *value){
    this->id = value;
    if(this->id[0] == '\"'){
        this->id = this->id.substr(1, this->user.length()-2);
    }
}

void LOGIN_::setStatus(){
    if(this->user == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] User invalido "<< "\x1B[0m" << endl;
        statusFlag = false;
    }
    if(this->password == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] Password Invalido "<< "\x1B[0m" << endl;
        statusFlag = false;
    }
    if(this->id == ""){
        cout<< "\u001B[31m" << "[BAD PARAM] ID Invalido "<< "\x1B[0m" << endl;
        statusFlag = false;
    }
}

void LOGIN_::initLog(){
    setStatus();
    if(this->statusFlag){
        if(!loged){
            int aux = 0;
            aux = makeLog();
            if(aux==0){
                cout<< "\u001B[31m" << "[Error] Usuario Invalido"<< "\x1B[0m" << endl;
            }
            else if(aux==1){
                loged=true;
                cout<< "\u001B[32m" << "[OK] Sesion iniciada exitosamente como "<<this->user<< "\x1B[0m" << endl;
            }
            else if(aux==2){
                cout<< "\u001B[31m" << "[Error] Contraseña invalida "<< "\x1B[0m" << endl;
            }
            else{
                cout<< "\u001B[31m" << "[Error] No se puede iniciar sesion"<< "\x1B[0m" << endl;
            }
        }
        else{
            cout<< "\u001B[31m" << "[Error] Ya hay una sesion activa, haga logout "<< "\x1B[0m" << endl;
        }
    }
}

int LOGIN_::makeLog(){

    string diskPath;
    //Se busca en las particiones montadas
    int partIndex = -1;
    MOUNT_ *disk;
    list<MOUNT_>::iterator i;
    for(i=mounted->begin(); i != mounted->end(); i++){
        if(i->getId() == this->id){
            disk = &*i;
            break;
        }
    }

    partIndex = disk->findPartitionIndex();

    if(partIndex!= -1){
        
        FILE *file = fopen(disk->getPath().data(),"r+b");

        //Se lee MBR
        MBR master;
        fread(&master,sizeof(MBR),1,file);
        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;
        
        //Se lee SuperBloque
        SuperBloque super;
        fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);

        //Se lee Tabla de Inodos
        InodeTable inodo;
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fread(&inodo,sizeof(InodeTable),1,file);

        inodo.i_atime = time(nullptr);
        fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
        fwrite(&inodo,sizeof(InodeTable),1,file);
        fclose(file);

        sesion.superStart = mbr_partitions[partIndex]->part_start;
        sesion.journalStart = mbr_partitions[partIndex]->part_start + static_cast<int>(sizeof(SuperBloque));
        sesion.sistemaType = super.s_filesystem_type;
        sesion.fit = mbr_partitions[partIndex]->part_fit;

        diskPath = disk->getPath();

        
    }
    else{
        partIndex = disk->findLogicPartitionStart();
        if(partIndex != -1){

            FILE *file = fopen(disk->getPath().data(),"r+b");

            //Se lee Superbloque
            SuperBloque super;
            fseek(file,partIndex+ sizeof(EBR),SEEK_SET);
            fread(&super,sizeof(SuperBloque),1,file);

            //Se lee tabla de Inodos
            InodeTable inodo;
            fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
            fread(&inodo,sizeof(InodeTable),1,file);

            inodo.i_atime = time(nullptr);
            fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
            fwrite(&inodo,sizeof(InodeTable),1,file);
            fclose(file);

            sesion.superStart = partIndex + sizeof(EBR);
            sesion.sistemaType = super.s_filesystem_type;
            sesion.fit = getLogicPartFit(disk->getPath());

            diskPath = disk->getPath();       
        }
        else{
            return 0;
        }
    }

    FILE *file = fopen(diskPath.data(),"r+b");

    //Se lee superbloque
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    //Se lee inodo de users.txt
    InodeTable inodo;
    fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,file);

    char auxArr[500] = "\0";
    BloqueArchivos archivo;
    //Se lee el archivo users.txt
    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            fseek(file,super.s_block_start + sizeof(BloqueArchivos)* inodo.i_block[i],SEEK_SET);
            fread(&archivo,sizeof(BloqueArchivos),1,file);
            strcat(auxArr,archivo.b_content);
        }
    }

    fclose(file);

    //Se parsea el archivo
    char *endString;
    char *lineToken = strtok_r(auxArr,"\n",&endString);
    

    while(lineToken != NULL){
        //Arreglos para guardar los datos
        char id[2];
        char tipo[2];
        string group;
        char userArr[15];
        char passwdArr[15];

        char *endToken;
        char *auxToken = new char[100]; 
            
        auxToken = strtok_r(lineToken,",",&endToken);
        strcpy(id,auxToken);
        if(strcmp(id,"0") != 0){
            auxToken=strtok_r(NULL,",",&endToken);
            for(int j=0;j<100;j++){
                if(auxToken[j]=='U' || auxToken[j]=='G'){
                    tipo[0]=auxToken[j];
                    tipo[1]='\0';
                    break;
                }
            }          
            //strcpy(tipo,auxToken);
            if(tipo[0] == 'U'||tipo[1]=='U'){
                auxToken = strtok_r(NULL,",",&endToken);
                group = auxToken;
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(userArr,auxToken);
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(passwdArr,auxToken);
                if(strcmp(userArr,this->user.data()) == 0){          
                    if( strcmp(passwdArr,this->password.data()) == 0){                       
                        sesion.user = atoi(id);
                        sesion.direccion = diskPath;
                        sesion.group = buscarGrupo(group);

                        FILE *file = fopen(diskPath.data(),"r+b");
                        fseek(file,sesion.superStart,SEEK_SET);
                        fread(&super,sizeof(SuperBloque),1,file);
                        super.s_mtime=time(nullptr);
                        super.s_mnt_count++;
                        fseek(file,sesion.superStart,SEEK_SET);
                        fwrite(&super,sizeof(SuperBloque),1,file);
                        fclose(file);

                        return 1;
                    }
                    else{
                        return 2;
                    }
                }
            }
        }
        *auxToken='\0';
        lineToken = strtok_r(NULL,"\n",&endString);
    }

    return 0;

}

char LOGIN_::getLogicPartFit(string path){
    FILE *file;
    file=fopen(path.data(),"r+b");
    if(file != NULL){
        int extendedIndex = -1;
        MBR master;
        EBR ebr;
        fseek(file,0,SEEK_SET);
        fread(&master,sizeof(MBR),1,file);

        Partition *mbr_partitions[4];
        mbr_partitions[0]=&master.mbr_partition_1;
        mbr_partitions[1]=&master.mbr_partition_2;
        mbr_partitions[2]=&master.mbr_partition_3;
        mbr_partitions[3]=&master.mbr_partition_4;

        for(int i = 0; i < 4; i++){
            if(mbr_partitions[i]->part_type == 'E'){
                extendedIndex = i;
                break;
            }
        }
        if(extendedIndex != -1){
            
            fseek(file, mbr_partitions[extendedIndex]->part_start,SEEK_SET);
            while(fread(&ebr,sizeof(EBR),1,file)!=0 ){

                fseek(file,ebr.part_next,SEEK_SET);               
                if(strcmp(id.data(),ebr.part_name) == 0){
                    fclose(file);
                    return ebr.part_fit;
                }
                if(ebr.part_next==-1){
                    break;
                }

            }
                
        }
        fclose(file);
    }
    return -1;
}

int LOGIN_::buscarGrupo(string name){
    FILE *file = fopen(sesion.direccion.data(),"r+b");

    //Se lee superbloque
    SuperBloque super;
    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    //Se lee inodo de users.txt
    InodeTable inodo;
    fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,file);

    char auxArr[500] = "\0";
    BloqueArchivos archivo;
    //Se lee el archivo users.txt
    for(int i = 0; i < 15; i++){
        if(inodo.i_block[i] != -1){
            fseek(file,super.s_block_start + sizeof(BloqueArchivos)* inodo.i_block[i],SEEK_SET);
            fread(&archivo,sizeof(BloqueArchivos),1,file);
            strcat(auxArr,archivo.b_content);
        }
    }

    fclose(file);

    char *endString;
    char *token = strtok_r(auxArr,"\n",&endString);
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

void LOGIN_::logout(){
    if(loged){
        SuperBloque super;
        FILE *file = fopen(sesion.direccion.c_str(),"r+b");
        fseek(file,sesion.superStart,SEEK_SET);
        fread(&super,sizeof(SuperBloque),1,file);
        super.s_umtime=time(nullptr);
        fseek(file,sesion.superStart,SEEK_SET);
        fwrite(&super,sizeof(SuperBloque),1,file);
        fclose(file);

        loged = false;
        sesion.direccion = "";
        sesion.user = -1;
        sesion.superStart = -1;
        cout<< "\u001B[32m" << "[OK] Sesion cerrada correctamente "<< "\x1B[0m" << endl;
    }
    else{
        cout<< "\u001B[31m" << "[BAD COMMAND] No hay session activa "<< "\x1B[0m" << endl;
    }
}