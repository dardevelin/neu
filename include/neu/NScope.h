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

#ifndef NEU_N_SCOPE_H
#define NEU_N_SCOPE_H

#include <neu/NObjectBase.h>
#include <neu/NVar.h>
#include <neu/NRWMutex.h>

namespace neu{

class NScope : public NObjectBase{
public:
  static const uint32_t classId;

  NScope(bool limiting=false, bool shared=false)
    : limiting_(limiting),
      shared_(shared ? new Shared_ : 0){
    
  }

  virtual bool instanceOf(uint32_t classId) const{
    return classId == NScope::classId;
  }

  bool isLimiting() const{
    return limiting_;
  }

  // warning: does not protect for shared scope
  void setSymbolFast(const nstr& s, const nvar& v){
    symbolMap_[s] = v;
  }

  void setSymbol(const nstr& s, const nvar& v){
    if(shared_){
      shared_->symbolMutex_.writeLock();
      symbolMap_[s] = v;
      shared_->symbolMutex_.unlock();
    }
    else{
      symbolMap_[s] = v;
    }
  }

  bool setNewSymbol(const nstr& s, const nvar& v){
    if(shared_){
      shared_->symbolMutex_.writeLock();

      auto itr = symbolMap_.find(s);
      if(itr != symbolMap_.end()){
        shared_->symbolMutex_.unlock();
        return false;
      }
      
      symbolMap_[s] = v;
      
      shared_->symbolMutex_.unlock();
      
      return true;
    }

    auto itr = symbolMap_.find(s);
    if(itr != symbolMap_.end()){
      return false;
    }
    
    symbolMap_[s] = v;
    
    return true;
  }

  bool getSymbol(const nstr& s, nvar& v){
    if(shared_){
      shared_->symbolMutex_.readLock();

      auto itr = symbolMap_.find(s);
      if(itr == symbolMap_.end()){
        shared_->symbolMutex_.unlock();
        return false;
      }

      v = itr->second;
      shared_->symbolMutex_.unlock();
      return true;
    }

    auto itr = symbolMap_.find(s);
    if(itr == symbolMap_.end()){
      return false;
    }
    
    v = itr->second;
    return true;
  }

  void setFunction(const nvar& s, const nvar& b){
    if(shared_){
      shared_->functionMutex_.writeLock();
      functionMap_.insert(std::make_pair(std::make_pair(s.str(), s.size()),
                                         std::make_pair(s, b)));
      shared_->functionMutex_.unlock();
      return;
    }

    functionMap_.insert(std::make_pair(std::make_pair(s.str(), s.size()),
                                       std::make_pair(s, b)));
  }

  bool getFunction(const nstr& f, size_t arity, nvar& s, nvar& b){
    if(shared_){
      shared_->functionMutex_.readLock();
      auto itr = functionMap_.find(std::make_pair(f, arity));
      if(itr == functionMap_.end()){
        shared_->functionMutex_.unlock();
        return false;
      }

      shared_->functionMutex_.unlock();
      s = itr->second.first;
      b = itr->second.second;
      return true;
    }

    auto itr = functionMap_.find(std::make_pair(f, arity));
    if(itr == functionMap_.end()){
      return false;
    }

    s = itr->second.first;
    b = itr->second.second;
    return true;
  }

private:
  bool limiting_ : 1;

  typedef NMap<nstr, nvar> SymbolMap_;
  typedef NMap<std::pair<nstr, size_t>, std::pair<nvar, nvar>> FunctionMap_;
  
  SymbolMap_ symbolMap_;
  FunctionMap_ functionMap_;

  struct Shared_{
    mutable NRWMutex symbolMutex_;
    mutable NRWMutex functionMutex_;
  };

  mutable Shared_* shared_;
};

} // end namespace neu

#endif // NEU_N_SCOPE_H
