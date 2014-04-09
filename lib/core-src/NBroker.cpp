/*
 
      ___           ___           ___     
     /\__\         /\  \         /\__\    
    /::|  |       /::\  \       /:/  /    
   /:|:|  |      /:/\:\  \     /:/  /     
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___ 
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  / 
     |::/  /     \:\ \/__/     \:\/:/  /  
     /:/  /       \:\__\        \::/  /   
     \/__/         \/__/         \/__/    
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#include <neu/NBroker.h>

#include <neu/NProc.h>
#include <neu/NServer.h>
#include <neu/NBasicMutex.h>
#include <neu/NObject.h>
#include <neu/NClass.h>

using namespace std;
using namespace neu;

namespace{
  
  class Communicator : public NCommunicator{
  public:
    Communicator(NProcTask* task, NBroker_* broker);
    
    void onClose(bool manual);
    
  private:
    NBroker_* broker_;
  };
  
  class Server : public NServer{
  public:
    Server(NProcTask* task, NBroker_* broker)
    : NServer(task),
    broker_(broker){
      
    }
    
    NCommunicator* create(){
      return new Communicator(task(), broker_);
    }
    
    bool authenticate(NCommunicator* comm, const nvar& auth);

  private:
    NBroker_* broker_;
  };
  
  class ClientProc : public NProc{
  public:
    ClientProc(NBroker_* broker)
    : broker_(broker){
      
    }
    
    void run(nvar& r);
    
  private:
    NBroker_* broker_;
  };
  
  class ServerProc : public NProc{
  public:
    ServerProc(NBroker_* broker, Communicator* comm)
    : broker_(broker),
    comm_(comm){
      
    }
    
    void run(nvar& r);
    
  private:
    NBroker_* broker_;
    Communicator* comm_;
  };
  
  class Object{
  public:
    nstr className;
    NObject* obj;
  };
  
} // end namespace

namespace neu{
  
  class NBroker_{
  public:
    
    NBroker_(NBroker* o, NProcTask* task)
    : o_(o),
    task_(task),
    server_(0),
    encoder_(0){
      
    }
    
    ~NBroker_(){
      if(server_){
        delete server_;
      }
      
      for(auto& itr : serverProcMap_){
        ServerProc* p = itr.first;
        task_->terminate(p);
      }
    }
    
    NProcTask* task(){
      return task_;
    }
    
    bool listen(int port){
      server_ = new Server(task_, this);
      if(server_->listen(port)){
        return true;
      }
      
      delete server_;
      server_ = 0;
      return false;
    }
    
    void distribute(NObject* object,
                    const nstr& className,
                    const nstr& objectName){
      
      Object* o = new Object;
      o->className = className;
      o->obj = object;
      
      objectMapMutex_.lock();
      objectMap_[objectName] = o;
      objectMapMutex_.unlock();
    }
    
    bool revoke(NObject* object){
      return false;
    }
    
    NObject* obtain(const nstr& host,
                    int port,
                    const nstr& objectName,
                    const nvar& auth){
      
      Communicator* comm = new Communicator(task_, this);
      if(!comm->connect(host, port)){
        delete comm;
        return 0;
      }
      
      comm->send(auth);
      nvar msg;
      comm->receive(msg);
      if(!msg){
        delete comm;
        return 0;
      }
      
      nvar req;
      req("t") = "obt";
      req("o") = objectName;
      
      comm->send(req);
      nvar resp;
      comm->receive(resp);
      
      if(resp["t"] == "obd" && resp["r"]){
        nvar f = nfunc(resp["c"]) << o_;
        
        NObjectBase* ob = NClass::create(f);
        NObject* obj = static_cast<NObject*>(ob);
        objClientMap_[obj] = comm;
        clientObjMap_[comm] = obj;
        
        return obj;
      }
      
      return 0;
    }
    
    bool release(NObject* object, bool disconnect){
      return false;
    }
    
    void setLogStream(std::ostream& ostr){
      
    }
    
    NBroker* outer(){
      return o_;
    }
    
    void setEncoder(NCommunicator::Encoder* encoder){
      encoder_ = encoder;
    }
    
    NCommunicator::Encoder* encoder(){
      return encoder_;
    }
    
    void createServerProc(Communicator* c){
      ServerProc* p = new ServerProc(this, c);
      p->setTask(task_);
    
      serverProcMapMutex_.lock();
      serverProcMap_[p] = true;
      serverProcMapMutex_.unlock();
      
      task_->queue(p);
    }
    
    nvar process_(NObject* obj, const nvar& n){
      auto itr = objClientMap_.find(obj);
      assert(itr != objClientMap_.end());
      Communicator* comm = itr->second;
      nvar req;
      req("t") = "cal";
      req("f") = n;
      comm->send(req);
      
      nvar resp;
      comm->receive(resp);
      if(resp["t"] == "cld"){
        return resp["r"];
      }
      
      NERROR("invalid call");
    }
    
  private:
    typedef NMap<ServerProc*, bool> ServerProcMap_;
    typedef NMap<NObject*, Communicator*> ObjClientMap_;
    typedef NMap<Communicator*, NObject*> ClientObjMap_;
    typedef NMap<nstr, Object*> ObjectMap_;
    
    NBroker* o_;
    NProcTask* task_;
    Server* server_;
    ServerProc* serverProc_;
    ClientProc* clientProc_;
    NCommunicator::Encoder* encoder_;
    ServerProcMap_ serverProcMap_;
    NBasicMutex serverProcMapMutex_;
    ObjectMap_ objectMap_;
    NBasicMutex objectMapMutex_;
    ClientObjMap_ clientObjMap_;
    ObjClientMap_ objClientMap_;
  };
  
} // end namespace neu

void ServerProc::run(nvar& r){
  
}

void ClientProc::run(nvar& r){
  
}

Communicator::Communicator(NProcTask* task, NBroker_* broker)
: NCommunicator(task),
broker_(broker){
  setEncoder(broker_->encoder());
}

void Communicator::onClose(bool manual){
  
}

bool Server::authenticate(NCommunicator* comm, const nvar& auth){
  if(!broker_->outer()->authenticate(auth)){
    return false;
  }

  Communicator* c = static_cast<Communicator*>(comm);
  broker_->createServerProc(c);
  
  return true;
}

NBroker::NBroker(NProcTask* task){
  x_ = new NBroker_(this, task);
}

NBroker::~NBroker(){
  delete x_;
}

NProcTask* NBroker::task(){
  return x_->task();
}

bool NBroker::listen(int port){
  return x_->listen(port);
}

void NBroker::distribute(NObject* object,
                         const nstr& className,
                         const nstr& objectName){
  x_->distribute(object, className, objectName);
}

bool NBroker::revoke(NObject* object){
  return x_->revoke(object);
}

NObject* NBroker::obtain(const nstr& host,
                         int port,
                         const nstr& objectName,
                         const nvar& auth){
  return x_->obtain(host, port, objectName, auth);
}

bool NBroker::release(NObject* object, bool disconnect){
  return x_->release(object, disconnect);
}

void NBroker::setLogStream(std::ostream& ostr){
  x_->setLogStream(ostr);
}

void NBroker::setEncoder(NCommunicator::Encoder* encoder){
  x_->setEncoder(encoder);
}

nvar NBroker::process_(NObject* obj, const nvar& n){
  return x_->process_(obj, n);
}
