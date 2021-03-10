#ifndef UNMOUNT_H
#define UNMOUNT_H

#endif // UNMOUNT_H

#include <MOUNT.h>
#include <list>
#include <iterator>
#include <structs.h>

class UNMOUNT_{
private:
    string id; //ID de la particion
    bool statusFlag; //Indica si hay errores en el comando
public:

    UNMOUNT_(){
        this->id="";
        this->statusFlag=false;
    }

    /**
     * Setter del ID
     * @param id: ID de la particion a desmontar
    */
    void setId(char* id);

    /**
     * Getter del ID
    */
    string getId();

    /*
     * Inicia el proceso de desmontaje
    */
    void beginToUnmount();

    /**
     * Verifica si hay errores en el comando
    */
    void setStatus();
    
};

extern list<MOUNT_> *mounted;

void UNMOUNT_::setId(char * id){
    this->id = id;
    if(this->id[0] == '\"'){
        this->id = this->id.substr(1,this->id.size()-2);
    }
}

string UNMOUNT_::getId(){
    return this->id;
}

void UNMOUNT_::setStatus(){
    if(this->id != ""){
        statusFlag = true;
    }
    else{
        cout<< "\u001B[31m" << "[BAD PARAM] Se tiene que especificar ID de la particion" << "\x1B[0m" << endl;
    }
}

void UNMOUNT_::beginToUnmount(){
    setStatus();
    if(this->statusFlag){
        list<MOUNT_>::iterator i;
        for(i = mounted->begin(); i != mounted->end();++i){
            if(i->getId() == this->id){
                cout<< "\u001B[32m" << "[OK] La particion " <<this->id<<" ha sido desmontada"<< "\x1B[0m" << endl;
                mounted->erase(i);
                return;
            }
        }
        cout<< "\u001B[31m" << "[BAD PARAM] La particion "<<this->id<<" no esta montada" << "\x1B[0m" << endl;
    }
}
