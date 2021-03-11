#ifndef LOSS_H
#define LOSS_H

#endif // LOSS_H
#include <string.h>
#include <stdio.h>
#include <MOUNT.h>
#include <iostream>
#include <list>
#include <iterator>
#include <structs.h>


using namespace std;

class LOSS_{

private:
    string id; //ID de la particion
    bool statusFlag; //Indica si hay errores en el comando
public:

    LOSS_(){
        this->id="";
        this->statusFlag=false;
    }

    /**
     * Setter del ID
     * @param id: ID de la particion a formatear
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
     * Inicia el proceso de formateo
    */
    void init();

};

extern list<MOUNT_> *mounted;

void LOSS_::setId(char * id){
    this->id = id;
    if(this->id[0] == '\"'){
        this->id = this->id.substr(1,this->id.size()-2);
    }
}

string LOSS_::getId(){
    return this->id;
}

void LOSS_::setStatus(){
    if(this->id != ""){
        statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] Se tiene que especificar ID de la particion" << "\x1B[0m" << endl;
    }
}

void LOSS_::init(){
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
       
        FILE *file = fopen(i->getPath().c_str(),"rb+");

        if(disk!=NULL){

            SuperBloque super;
            partIndex = disk->findPartitionIndex();

            if(partIndex != -1){
                MBR master;
                fread(&master,sizeof(MBR),1,file);
                
                Partition *mbr_partitions[4];
                mbr_partitions[0]=&master.mbr_partition_1;
                mbr_partitions[1]=&master.mbr_partition_2;
                mbr_partitions[2]=&master.mbr_partition_3;
                mbr_partitions[3]=&master.mbr_partition_4;

                fseek(file,mbr_partitions[partIndex]->part_start,SEEK_SET);
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
            if(super.s_filesystem_type == 3){
                char myChar = '\0';
                fseek(file,super.s_bm_inode_start,SEEK_SET);
                for(int i = super.s_bm_inode_start; i < super.s_block_start + super.s_block_size*(super.s_blocks_count); i++){
                    fputc(myChar,file);
                }       
                fclose(file);
                cout<< "\u001B[33m" << "[WARNING] Se ha simulado la perdidda de informacion en la particion "<<this->id<< "\x1B[0m" << endl;

            }
            else{
                cout<< "\u001B[31m" << "[BAD PARAM] La simulacion de perdida de informacion no se puede llevar en un sistema ext2" << "\x1B[0m" << endl;
            }
        }
        else{
            cout<< "\u001B[31m" << "[BAD PARAM] La particion no esta montada" << "\x1B[0m" << endl;
        }                 
    }

}

