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

#include <neu/NProcTask.h>

#include <vector>
#include <queue>

#include <neu/NVSemaphore.h>
#include <neu/NBasicMutex.h>
#include <neu/NThread.h>
#include <neu/NProc.h>

using namespace std;
using namespace neu;

namespace neu{
  
  class NProc;
  
  class NProcTask_{
  public:
    class Item{
    public:
      Item(double p, nvar& r, NProc* np)
      : p(p),
      r(move(r)),
      np(np){
        
      }
      
      double p;
      nvar r;
      NProc* np;
    };
    
    class Queue{
    public:
      Queue()
      : sem_(0){
        
      }
 
      void put(Item* item){
        mutex_.lock();
        queue_.push(item);
        mutex_.unlock();

        sem_.release();
      }
      
      Item* get(){
        sem_.acquire();
        
        mutex_.lock();
        Item* item = queue_.top();
        queue_.pop();
        mutex_.unlock();
        
        return item;
      }
      
    private:
      struct Compare_{
        bool operator()(const Item* i1, const Item* i2) const{
          return i1->p < i2->p;
        }
      };
      
      typedef priority_queue<Item*, vector<Item*>, Compare_> Queue_;
      
      Queue_ queue_;
      NVSemaphore sem_;
      NBasicMutex mutex_;
    };

    class Thread : public NThread{
    public:
      Thread(Queue& queue)
      : queue_(queue){
        
      }
      
      void run(){
        for(;;){
          Item* item = queue_.get();
          item->np->run(item->r);
          delete item;
        }
      }
      
    private:
      Queue& queue_;
    };
    
    NProcTask_(NProcTask* o, size_t threads)
    : o_(o){
    
      for(size_t i = 0; i < threads; ++i){
        Thread* thread = new Thread(q_);
        threadVec_.push_back(thread);
        thread->start();
      }
    }
    
    ~NProcTask_(){
      
    }
    
    void queue(NProc* proc, nvar& r, double priority){
      Item* item = new Item(priority, r, proc);
      q_.put(item);
    }
    
  private:
    typedef NVector<Thread*> ThreadVec_;
    
    NProcTask* o_;
    Queue q_;
    ThreadVec_ threadVec_;
  };
  
} // end namespace neu

NProcTask::NProcTask(size_t threads){
  x_ = new NProcTask_(this, threads);
}

NProcTask::~NProcTask(){
  delete x_;
}

void NProcTask::queue(NProc* proc, nvar& r, double priority){
  x_->queue(proc, r, priority);
}
