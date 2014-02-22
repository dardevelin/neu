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

#include <neu/NResourceManager.h>

#include <neu/NList.h>
#include <neu/NMutex.h>
#include <neu/NRecMutex.h>
#include <neu/NCommand.h>

using namespace std;
using namespace neu;

namespace neu{

  class NResourceManager_{
  public:
    NResourceManager_(NResourceManager* o)
    : o_(o){

    }

    ~NResourceManager_(){

    }

    void release(){
      releaseMutex_.lock();
      mutex_.lock();

      for(auto& itr : commandList_){
        itr->close();
      }
  
      commandList_.clear();

      mutex_.unlock();
      releaseMutex_.unlock();
    }

    void add(NCommand* command){
      mutex_.lock();
      commandList_.push_back(command);
      mutex_.unlock();
    }
  
    void remove(NCommand* command){
      mutex_.lock();
      commandList_.remove(command);
      mutex_.unlock();
    }

  private:
    typedef NList<NCommand*> CommandList_;

    NResourceManager* o_;

    CommandList_ commandList_;
    
    NRecMutex mutex_;
    NMutex releaseMutex_;
  };

} // end namespace neu

NResourceManager::NResourceManager(){
  x_ = new NResourceManager_(this);
}

NResourceManager::~NResourceManager(){
  delete x_;
}

void NResourceManager::release(){
  x_->release();
}

void NResourceManager::add(NCommand* command){
  x_->add(command);
}

void NResourceManager::remove(NCommand* command){
  x_->remove(command);
}
