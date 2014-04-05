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

#include <neu/NCommunicator.h>

#include <deque>

#include <neu/NProcTask.h>
#include <neu/NProc.h>
#include <neu/NBasicMutex.h>
#include <neu/NVSemaphore.h>
#include <neu/NError.h>
#include <neu/NSocket.h>

using namespace std;
using namespace neu;

namespace{
  
  class ReceiveProc : public NProc{
  public:
    ReceiveProc(NCommunicator_* c);
    
    bool handle(nvar& v, nvar& r){
      return true;
    }
    
    void run(nvar& r);
    
  private:
    NCommunicator_* c_;
    NSocket* s_;
  };
  
  class SendProc : public NProc{
  public:
    SendProc(NCommunicator_* c);
    
    bool handle(nvar& v, nvar& r){
      return true;
    }
    
    void run(nvar& r);
    
  private:
    NCommunicator_* c_;
    NSocket* s_;
  };
  
} // end namespace

namespace neu{
  
  class NCommunicator_{
  public:
    NCommunicator_(NCommunicator* o, NProcTask* task, NSocket* socket)
    : o_(o),
    task_(task),
    socket_(socket),
    sendSem_(0),
    sendProc_(0),
    receiveSem_(0),
    receiveProc_(0){
      
      if(socket_){
        start_();
      }
      
    }
    
    ~NCommunicator_(){
      if(socket_){
        delete socket_;
      }
    }
    
    void start_(){
      
    }
    
    bool connect(const nstr& host, int port){
      assert(!socket_);
      
      socket_ = new NSocket;
      if(!socket_->connect(host, port)){
        delete socket_;
        return false;
      }
      
      return true;
    }
    
    NProcTask* task(){
      return task_;
    }
    
    void close(){
      
    }
    
    void close_(){
      
    }
    
    bool isConnected() const{
      return false;
    }
    
    void send(const nvar& msg){
      sendMutex_.lock();
      sendQueue_.emplace_back(move(msg));
      sendMutex_.unlock();
      sendSem_.release();
    }
    
    bool receive(nvar& msg, double timeout){
      if(!receiveSem_.acquire(timeout)){
        return false;
      }
      
      receiveMutex_.lock();
      msg = move(receiveQueue_.front());
      receiveQueue_.pop_front();
      receiveMutex_.unlock();
      return true;
    }
    
    bool receive(nvar& msg){
      receiveSem_.acquire();
      
      receiveMutex_.lock();
      msg = move(receiveQueue_.front());
      receiveQueue_.pop_front();
      receiveMutex_.unlock();
      return true;
    }
    
    bool get(nvar& msg, double timeout){
      if(!sendSem_.acquire(timeout)){
        return false;
      }

      sendMutex_.lock();
      msg = move(sendQueue_.front());
      sendQueue_.pop_front();
      sendMutex_.unlock();
      return true;
    }
    
    void put(nvar& msg){
      receiveMutex_.lock();
      receiveQueue_.emplace_back(move(msg));
      receiveMutex_.unlock();
      receiveSem_.release();
    }
    
    NSocket* socket(){
      return socket_;
    }
    
  private:
    typedef deque<nvar> Queue_;
    
    NCommunicator* o_;
    NProcTask* task_;
    NSocket* socket_;
    Queue_ sendQueue_;
    NBasicMutex sendMutex_;
    NVSemaphore sendSem_;
    SendProc* sendProc_;
    Queue_ receiveQueue_;
    NBasicMutex receiveMutex_;
    NVSemaphore receiveSem_;
    ReceiveProc* receiveProc_;
  };
  
} // end namespace neu

ReceiveProc::ReceiveProc(NCommunicator_* c)
: c_(c),
s_(c_->socket()){
  
}

void ReceiveProc::run(nvar& r){
  
}

SendProc::SendProc(NCommunicator_* c)
: c_(c),
s_(c_->socket()){
  
}

void SendProc::run(nvar& r){
  
}

NCommunicator::NCommunicator(NProcTask* task, NSocket* socket){
  x_ = new NCommunicator_(this, task, socket);
}

NCommunicator::~NCommunicator(){
  delete x_;
}

bool NCommunicator::connect(const nstr& host, int port){
  return x_->connect(host, port);
}

NProcTask* NCommunicator::task(){
  return x_->task();
}

void NCommunicator::close(){
  x_->close();
}

void NCommunicator::close_(){
  x_->close_();
}

bool NCommunicator::isConnected() const{
  return x_->isConnected();
}

void NCommunicator::send(const nvar& msg){
  x_->send(msg);
}

bool NCommunicator::receive(nvar& msg){
  return x_->receive(msg);
}

bool NCommunicator::receive(nvar& msg, double timeout){
  return x_->receive(msg, timeout);
}
