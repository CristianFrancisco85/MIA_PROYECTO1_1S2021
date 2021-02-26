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
     *Setter del ID 
      * @param size: ID de la particion montada
     */
    void setId(char* value);

    /**
     *Verifica que no haya error en los parametros 
     */
    void setStatus();

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
            int aux = makeLog();
            if(aux=0){
                cout<< "\u001B[31m" << "[Error] Usuario Invalido"<< "\x1B[0m" << endl;
            }
            else if(aux=1){
                loged=true;
                cout<< "\u001B[32m" << "[OK] Sesion iniciada exitosamente "<< "\x1B[0m" << endl;
            }
            else if(aux=2){
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

    char auxArr[500] = "\0";
    SuperBloque super;
    InodeTable inodo;

    fseek(file,sesion.superStart,SEEK_SET);
    fread(&super,sizeof(SuperBloque),1,file);

    //Se lee inodo de users.txt
    fseek(file,super.s_inode_start + sizeof(InodeTable),SEEK_SET);
    fread(&inodo,sizeof(InodeTable),1,file);

    //Se lee el archivo users.txt
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
        char *auxToken = strtok_r(lineToken,",",&endToken);
        strcpy(id,auxToken);
        if(strcmp(id,"0") != 0){
            auxToken=strtok_r(NULL,",",&endToken);
            strcpy(tipo,auxToken);
            if(strcmp(tipo,"U") == 0){
                auxToken = strtok_r(NULL,",",&endToken);
                group = auxToken;
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(userArr,auxToken);
                auxToken = strtok_r(NULL,",",&endToken);
                strcpy(passwdArr,auxToken);

                if(strcmp(userArr,this->user.data()) == 0){
                    if(strcmp(passwdArr,this->password.data()) == 0){
                        sesion.user = atoi(id);
                        sesion.direccion = diskPath;
                        sesion.group = buscarGrupo(group);
                        return 1;
                    }
                    else{
                        return 2;
                    }
                }
            }
        }
        lineToken = strtok_r(nullptr,"\n",&endString);
    }

    return 0;

}

char LOGIN_::getLogicPartFit(string path){
    FILE *file;
    file=fopen(path.data(),"r+b");
    if(file != NULL){
        int extendedIndex = -1;
        MBR master;
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
            EBR ebr;
            fseek(file, mbr_partitions[extendedIndex]->part_start,SEEK_SET);
            while(fread(&ebr,sizeof(EBR),1,file)!=0 ){
                if(strcmp(id.data(),ebr.part_name) == 0){
                    fclose(file);
                    return ebr.part_fit;
                }
                else if(ebr.part_next==-1){
                    break;
                }
                else{
                    fseek(file,ebr.part_next,SEEK_SET);
                }
            }
                
        }
        fclose(file);
    }
    return -1;
}

int LOGIN_::buscarGrupo(string name){
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

void LOGIN_::logout(){
    if(loged){
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