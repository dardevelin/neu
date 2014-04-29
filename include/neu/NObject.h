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

#ifndef NEU_N_OBJECT_H
#define NEU_N_OBJECT_H

#include <neu/NObjectBase.h>
#include <neu/nvar.h>

namespace neu{
  
  class NScope;
  class NBroker;
  
  class NObject : public NObjectBase{
  public:
    static const uint32_t classId;
    
    NObject();
    
    NObject(NScope* sharedScope);
    
    NObject(NBroker* broker);
    
    ~NObject();
    
    static const uint32_t Delegated = 0x00000001;
    
    nvar remoteProcess(const nvar& v);
    
    nvar process(const nvar& v, uint32_t flags=0);
    
    virtual NFunc handle(const nvar& v, uint32_t flags=0);
    
    virtual bool instanceOf(uint32_t classId) const{
      return classId == NObject::classId;
    }
    
    void enableThreading();
    
    void setStrict(bool flag);
    
    void setExact(bool flag);
    
    bool isRemote();
    
    void foo(nvar& x);
    
    static int precedence(const nvar& f);
    
    NScope* currentScope();
    
    NScope* objectScope();
    
    nvar Reset();
    
    nvar Throw(const nvar& v1, const nvar& v2);
    
    nvar Add(const nvar& v1, const nvar& v2);
    
    nvar Sub(const nvar& v1, const nvar& v2);
    
    nvar Mul(const nvar& v1, const nvar& v2);
    
    nvar Div(const nvar& v1, const nvar& v2);
    
    nvar Mod(const nvar& v1, const nvar& v2);
    
    nvar Neg(const nvar& v);
    
    nvar AddBy(const nvar& v1, const nvar& v2);
    
    nvar SubBy(const nvar& v1, const nvar& v2);
    
    nvar MulBy(const nvar& v1, const nvar& v2);
    
    nvar DivBy(const nvar& v1, const nvar& v2);
    
    nvar ModBy(const nvar& v1, const nvar& v2);
    
    nvar Inc(const nvar& v);
    
    nvar PostInc(const nvar& v);
    
    nvar Dec(const nvar& v);
    
    nvar PostDec(const nvar& v);
    
    nvar LT(const nvar& v1, const nvar& v2);
    
    nvar LE(const nvar& v1, const nvar& v2);
    
    nvar GT(const nvar& v1, const nvar& v2);
    
    nvar GE(const nvar& v1, const nvar& v2);
    
    nvar EQ(const nvar& v1, const nvar& v2);
    
    nvar NE(const nvar& v1, const nvar& v2);
    
    nvar And(const nvar& v1, const nvar& v2);
    
    nvar Or(const nvar& v1, const nvar& v2);
    
    nvar Not(const nvar& v);
    
    nvar Sqrt(const nvar& v);
    
    nvar Exp(const nvar& v);
    
    nvar Abs(const nvar& v);
    
    nvar Floor(const nvar& v);
    
    nvar Ceil(const nvar& v);
    
    nvar Log10(const nvar& v);
    
    nvar Log(const nvar& v);
    
    nvar Cos(const nvar& v);
    
    nvar Acos(const nvar& v);
    
    nvar Cosh(const nvar& v);
    
    nvar Sin(const nvar& v);
    
    nvar Asin(const nvar& v);
    
    nvar Sinh(const nvar& v);
    
    nvar Tan(const nvar& v);
    
    nvar Atan(const nvar& v);
    
    nvar Tanh(const nvar& v);
    
    nvar Var(const nvar& v);
    
    nvar Var(const nvar& v1, const nvar& v2);
    
    nvar Var(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Set(const nvar& v1, const nvar& v2);
    
    nvar VarSet(const nvar& v1, const nvar& v2);
    
    nvar Get(const nvar& v);
    
    nvar Get(const nvar& v1, const nvar& v2);
    
    nvar Idx(const nvar& v1, const nvar& v2);
    
    nvar Dot(const nvar& v1, const nvar& v2);
    
    nvar Put(const nvar& v1, const nvar& v2);
    
    nvar DotPut(const nvar& v1, const nvar& v2);
    
    nvar Cs(const nvar& v);
    
    nvar In(const nvar& v1, const nvar& v2);
    
    nvar Call(const nvar& v);
    
    nvar Call(const nvar& v1, const nvar& v2);
    
    nvar Def(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Def(const nvar& v1, const nvar& v2);
    
    nvar New(const nvar& v);
    
    nvar New(const nvar& v1, const nvar& v2);
    
    nvar PushScope(const nvar& v);
    
    nvar PopScope();
    
    nvar PushBack(const nvar& v1, const nvar& v2);
    
    nvar TouchMultimap(const nvar& v1);
    
    nvar TouchList(const nvar& v1);
    
    nvar Keys(const nvar& v1);
    
    nvar PushFront(const nvar& v1, const nvar& v2);
    
    nvar PopBack(const nvar& v1);
    
    nvar HasKey(const nvar& v1, const nvar& v2);
    
    nvar Insert(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar Clear(const nvar& v1);
    
    nvar Empty(const nvar& v1);
    
    nvar Back(const nvar& v1);
    
    nvar Erase(const nvar& v1, const nvar& v2);
    
    nvar Merge(const nvar& v1, const nvar& v2);
    
    nvar OuterMerge(const nvar& v1, const nvar& v2);
    
    nvar Class(const nvar& v1);
    
    nvar If(const nvar& v1, const nvar& v2);
    
    nvar If(const nvar& v1, const nvar& v2, const nvar& v3);
    
    nvar For(const nvar& v1, const nvar& v2, const nvar& v3, const nvar& v4);
    
    nvar While(const nvar& v1, const nvar& v2);
    
    void dumpScopes();
    
    friend class NObject_;
    
  private:
    class NObject_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_OBJECT_H
