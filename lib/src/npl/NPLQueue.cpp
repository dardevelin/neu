/*================================= Neu =================================
 
 Copyright (c) 2013-2014, Andrometa LLC
 All rights reserved.
 
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
 
 =======================================================================*/

#include <neu/NPLQueue.h>

#include <neu/NVSemaphore.h>
#include <neu/NThread.h>
#include <neu/NBasicMutex.h>
#include <neu/NList.h>

using namespace std;
using namespace neu;

namespace{
  
  class Queue{
  public:
    
    void add(size_t index){
      queue_.push_back(index);
    }
    
    bool get(size_t& index){
      mutex_.lock();
      
      if(queue_.empty()){
        mutex_.unlock();
        return false;
      }
      
      index = queue_.front();
      queue_.pop_front();
      mutex_.unlock();
      
      return true;
    }
    
  private:
    typedef NList<size_t> Queue_;
    
    Queue_ queue_;
    NBasicMutex mutex_;
  };
  
  class Node{
  public:
    Node(NPLFunc fp, NPLObject* o, NPLObject* o2)
    : fp(fp),
    o(o),
    o2(o2){
      
    }
    
    NPLFunc fp;
    NPLObject* o;
    NPLObject* o2;
  };
  
} // end namespace

namespace neu{
  
class NPLQueue_{
public:
  
  NPLQueue_(NPLQueue* o, size_t threads)
  : o_(o),
    threads_(threads),
    chunk_(-1){

    for(size_t i = 0; i < threads_; ++i){
      Thread_* thread = new Thread_(nodeVec_, queue_, chunk_);
      threadVec_.push_back(thread);
      thread->start();
    }
  }
  
  ~NPLQueue_(){
    for(size_t i = 0; i < threads_; ++i){
      Thread_* thread = threadVec_[i];
      thread->exit();
      thread->join();
      delete thread;
    }
  }
  
  void add(NPLFunc f, NPLObject* o, NPLObject* o2){
    chunk_ = -1;
    nodeVec_.push_back(new Node(f, o, o2));
  }

  void clear(){
    for(auto& n : nodeVec_){
      delete n;
    }
    
    nodeVec_.clear();
  }
  
  void start(){
    size_t end = nodeVec_.size();

    if(chunk_ < 0){
      resetChunk();
    }

    for(size_t i = 0; i < end; i += chunk_){
      queue_.add(i);
    }

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->begin();
    }
  }
  
  void await(){
    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->finish();
    }
  }

  void resetChunk(){
    chunk_ = nodeVec_.size() / (threads_ * 2) + 1;
    
    while(chunk_ % 32 != 0){
      ++chunk_;
    }
  }

  bool run(){
    if(nodeVec_.empty()){
      return false;
    }

    size_t end = nodeVec_.size();

    if(chunk_ < 0){
      resetChunk();
    }

    for(size_t i = 0; i < end; i += chunk_){
      queue_.add(i);
    }

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->begin();
    }

    // run

    for(size_t i = 0; i < threads_; ++i){
      threadVec_[i]->finish();
    }
    
    return true;
  }
  
private:
  typedef NVector<Node*> NodeVec_;

  NPLQueue* o_;
  size_t threads_;
  NodeVec_ nodeVec_;
  Queue queue_;
  int chunk_;

  class Thread_ : public NThread{
  public:
    Thread_(NodeVec_& nodeVec, Queue& queue, int& chunk)
      : nodeVec_(nodeVec),
        queue_(queue),
        beginSem_(0),
        finishSem_(0),
        exit_(false),
        chunk_(chunk){
      
    }
    
    void begin(){
      beginSem_.release();
    }

    void finish(){
      finishSem_.acquire();
    }

    void exit(){
      exit_ = true;
      beginSem_.release();
    }
    
    void run(){
      size_t i;
      Node* n;

      for(;;){
        beginSem_.acquire();

        if(exit_){
          return;
        }

        for(;;){
          if(!queue_.get(i)){
            break;
          }

          size_t end = i + chunk_;
          size_t size = nodeVec_.size();

          if(end > size){
            end = size;

            while(i < end){
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
            }
          }
          else{
            while(i < end){
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
              
              n = nodeVec_[i++];
              n->fp(n->o, n->o2);

              n = nodeVec_[i++];
              n->fp(n->o, n->o2);
            }
          }
        }
    
        finishSem_.release();
      }
    }
    
  private:
    NodeVec_& nodeVec_;
    Queue& queue_;
    NVSemaphore beginSem_;
    NVSemaphore finishSem_;
    bool exit_;
    int& chunk_;
  };
  
  typedef NVector<Thread_*> ThreadVec_;
  
  ThreadVec_ threadVec_;
};

} // end namespace Meta

NPLQueue::NPLQueue(size_t threads){
  x_ = new NPLQueue_(this, threads);
}

NPLQueue::~NPLQueue(){
  delete x_; 
}

void NPLQueue::add(NPLFunc f, NPLObject* o, NPLObject* o2){
  x_->add(f, o, o2);
}

void NPLQueue::clear(){
  x_->clear();
}

void NPLQueue::start(){
  x_->start();
}

bool NPLQueue::run(){
  return x_->run();
}

void NPLQueue::await(){
  x_->await();
}
