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

#include <neu/NServer.h>

#include <atomic>

#include <neu/NProcTask.h>
#include <neu/NProc.h>
#include <neu/NListener.h>
#include <neu/NVSemaphore.h>
#include <neu/NCommunicator.h>

using namespace std;
using namespace neu;

namespace{

  class AuthProc : public NProc{
  public:
    AuthProc(NServer* server)
    : server_(server),
    active_(true){
      
    }
    
    void run(nvar& r){
      if(!active_){
        finished();
        return;
      }
      
      NSocket* socket = r["s"].ptr<NSocket>();
      NCommunicator* comm = server_->create(socket);
      nvar auth;
      comm->receive(auth);
      server_->authenticate(comm, auth);
    }
    
    void finished(){
      f_.release();
    }
    
    void await(){
      f_.acquire();
    }
    
    void finish(){
      active_ = false;
    }
    
  private:
    NServer* server_;
    NVSemaphore f_;
    atomic_bool active_;
  };
  
  class AcceptProc : public NProc{
  public:
    AcceptProc(AuthProc* authProc, NListener& listener, int port)
    : authProc_(authProc),
    listener_(listener),
    active_(true){
      
    }

    void run(nvar& r){
      if(!active_){
        finished();
        return;
      }
      
      NSocket* socket = listener_.accept(_timeout);
      if(socket){
        nvar r;
        r("s") = socket;
        task()->queue(authProc_, r);
      }
      
      signal(this);
    }
    
    void finished(){
      f_.release();
    }
    
    void await(){
      f_.acquire();
    }
    
    void finish(){
      active_ = false;
    }
    
  private:
    AuthProc* authProc_;
    NListener& listener_;
    NVSemaphore f_;
    atomic_bool active_;
  };
  
} // end namespace

namespace neu{
  
  class NServer_{
  public:
    NServer_(NServer* o, NProcTask* task)
    : o_(o),
    task_(task),
    acceptProc_(0),
    authProc_(0){
      
    }
    
    ~NServer_(){
      if(acceptProc_){
        acceptProc_->finish();
        acceptProc_->await();
        delete acceptProc_;
      }
      
      if(authProc_){
        authProc_->finish();
        authProc_->await();
        delete authProc_;
      }
    }
    
    NProcTask* task(){
      return task_;
    }
    
    bool listen(int port){
      if(!listener_.listen(port)){
        return false;
      }
      
      acceptProc_ = new AcceptProc(authProc_, listener_, port);
      acceptProc_->setTask(task_);
      
      authProc_ = new AuthProc(o_);
      authProc_->setTask(task_);
      
      task_->queue(acceptProc_);
      return true;
    }
    
  private:
    NServer* o_;
    NProcTask* task_;
    AcceptProc* acceptProc_;
    AuthProc* authProc_;
    NListener listener_;
  };
  
} // end namespace neu

NServer::NServer(NProcTask* task){
  x_ = new NServer_(this, task);
}

NServer::~NServer(){
  delete x_;
}

NProcTask* NServer::task(){
  return x_->task();
}

bool NServer::listen(int port){
  return x_->listen(port);
}
