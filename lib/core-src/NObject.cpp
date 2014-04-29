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

#include <neu/NObject.h>

#include <neu/NFuncMap.h>
#include <neu/NScope.h>
#include <neu/NClass.h>
#include <neu/NThread.h>
#include <neu/NRWMutex.h>
#include <neu/NBroker.h>

using namespace std;
using namespace neu;

namespace{
  
  class FuncMap : public NFuncMap{
  public:
    FuncMap();
  };
  
  FuncMap _funcMap;
  
  class Global{
  public:
    Global()
    : globalScope_(new NScope){
      
      precedenceMap_("Set") = 17;
      precedenceMap_("Var") = 17;
      precedenceMap_("Def") = 17;
      precedenceMap_("AddBy") = 17;
      precedenceMap_("SubBy") = 17;
      precedenceMap_("MulBy") = 17;
      precedenceMap_("DivBy") = 17;
      precedenceMap_("AltBy") = 17;
      precedenceMap_("ModBy") = 17;
      precedenceMap_("Or") = 15;
      precedenceMap_("And") = 15;
      precedenceMap_("EQ") = 11;
      precedenceMap_("NE") = 11;
      precedenceMap_("LT") = 10;
      precedenceMap_("GT") = 10;
      precedenceMap_("LE") = 10;
      precedenceMap_("GE") = 10;
      precedenceMap_("Push") = 9;
      precedenceMap_("Sub") = 8;
      precedenceMap_("Add") = 8;
      precedenceMap_("Mod") = 7;
      precedenceMap_("Div") = 7;
      precedenceMap_("Mul") = 7;
      precedenceMap_("Not") = 5;
      precedenceMap_("Neg") = 5;
      precedenceMap_("Pow") = 4;
      precedenceMap_("PostDec") = 3;
      precedenceMap_("PostInc") = 3;
      precedenceMap_("Dec") = 2;
      precedenceMap_("Inc") = 2;
      
    }
    
    NScope* globalScope(){
      return globalScope_;
    }
    
    const int precedence(const nstr& op){
      return precedenceMap_.get(op, -1);
    }
    
  private:
    NScope* globalScope_;
    nvar precedenceMap_;
  };
  
  Global _global;
  
  class Class : public NClass{
  public:
    Class() : NClass("neu::NObject"){
      
    }
    
    NObjectBase* construct(const nvar& f){
      switch(f.size()){
        case 0:
          return new NObject;
        case 1:
          return new NObject(static_cast<NScope*>(f[0].obj()));
        default:
          return 0;
      }
    }
  };
  
  Class _class;
  
  NFunc _ret0Func;
  NFunc _ret1Func;
  NFunc _breakFunc;
  NFunc _continueFunc;
  
} // end namespace

const uint32_t NObject::classId = NObjectBase::getClassId();

const uint32_t NScope::classId = NObjectBase::getClassId();

namespace neu{
  
  class NObject_{
  public:
    typedef NVector<NScope*> ScopeStack;
    
    class ThreadContext{
    public:
      
      void pushScope(NScope* scope){
        scopeStack.push_back(scope);
      }
      
      NScope* getScope(size_t i){
        return scopeStack[i];
      }
      
      NScope* topScope(){
        return scopeStack.back();
      }
      
      void popScope(){
        scopeStack.pop_back();
      }
      
      void dumpScopes(){
        for(size_t i = 0; i < scopeStack.size(); ++i){
          cout << "---------------------" << endl;
          scopeStack[i]->dump();
        }
        cout << "======================" << endl;
      }
      
      ScopeStack scopeStack;
    };
    
    class ThreadData{
    public:
      
      ~ThreadData(){
        for(auto& itr : contextMap_){
          delete itr.second;
        }
      }
      
      ThreadContext*
      getContext(NObject_* obj, const NThread::id& threadId){
        
        contextMutex_.readLock();
        auto itr = contextMap_.find(threadId);
        if(itr != contextMap_.end()){
          ThreadContext* context = itr->second;
          contextMutex_.unlock();
          return context;
        }
        contextMutex_.unlock();
        
        ThreadContext* context = new ThreadContext;
        
        obj->initContext_(context);
        
        contextMutex_.writeLock();
        contextMap_.insert({threadId, context});
        contextMutex_.unlock();
        
        return context;
      }
      
    private:
      typedef NMap<NThread::id, ThreadContext*> ContextMap_;
      
      ContextMap_ contextMap_;
      NRWMutex contextMutex_;
    };
    
    NObject_(NObject* o)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(false),
    threadData_(0),
    broker_(0){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", gs);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    NObject_(NObject* o, NScope* sharedScope)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(true),
    threadData_(0),
    broker_(0){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      mainContext_.pushScope(sharedScope);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", sharedScope);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    NObject_(NObject* o, NBroker* broker)
    : o_(o),
    exact_(false),
    strict_(true),
    sharedScope_(false),
    threadData_(0),
    broker_(broker){
      
      NScope* gs = _global.globalScope();
      mainContext_.pushScope(gs);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", gs);
      s->setSymbolFast("This", o_);
      mainContext_.pushScope(s);
    }
    
    ~NObject_(){
      if(sharedScope_){
        delete mainContext_.getScope(2);
      }
      else{
        delete mainContext_.getScope(1);
      }
      
      if(threadData_){
        delete threadData_;
      }
    }
    
    static NObject_* inner(NObject* o){
      return o->x_;
    }
    
    void setStrict(bool flag){
      strict_ = flag;
    }
    
    void setExact(bool flag){
      exact_ = flag;
    }
    
    bool isRemote(){
      return broker_;
    }
    
    void enableThreading(){
      if(threadData_){
        return;
      }
      
      threadData_ = new ThreadData;
    }
    
    void initContext_(ThreadContext* context){
      context->pushScope(mainContext_.getScope(0));
      context->pushScope(mainContext_.getScope(1));
      
      if(sharedScope_){
        context->pushScope(mainContext_.getScope(2));
      }
    }
    
    ThreadContext* getContext(){
      if(threadData_){
        NThread::id threadId = NThread::thisThreadId();
        
        if(threadId == NThread::mainThreadId){
          return &mainContext_;
        }
        
        return threadData_->getContext(this, threadId);
      }
      
      return &mainContext_;
    }
    
    void getSymbol(ThreadContext* context, const nstr& s, nvar& v){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getSymbol(s, v)){
          return;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 3 : 2;
        }
      }
      
      if(strict_){
        NERROR("symbol not in scope: " + s);
      }
      
      v = nsym(s);
    }
    
    void getSymbolNone(ThreadContext* context, const nstr& s, nvar& v){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getSymbol(s, v)){
          return;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 3 : 2;
        }
      }
      
      v = none;
    }
    
    bool getFunction(ThreadContext* context,
                     const nstr& f,
                     size_t arity,
                     nvar& s,
                     nvar& b){
      for(int i = context->scopeStack.size() - 1; i >= 0; --i){
        NScope* scope = context->getScope(i);
        
        if(scope->getFunction(f, arity, s, b)){
          return true;
        }
        
        if(scope->isLimiting()){
          i = sharedScope_ ? 2 : 1;
        }
      }
      
      return false;
    }
    
    nvar process(const nvar& v, uint32_t flags=0){
      //cout << "processing: " << v << endl;
      
      const nvar& vd = *v;
      
      switch(vd.type()){
        case nvar::Function:{
          NFunc fp = vd.func();
          
          if(fp){
            return (*fp)(o_, v);
          }
          
          fp = o_->handle(vd, flags);
          
          if(fp){
            return (*fp)(o_, v);
          }
          
          ThreadContext* context = getContext();
          
          nvar s;
          nvar b;
          if(getFunction(context, vd.str(), vd.size(), s, b)){
            NScope scope(true);
            context->pushScope(&scope);
            
            size_t size = vd.size();
            for(size_t i = 0; i < size; ++i){
              const nvar& si = s[i];
              const nvar& pi = vd[i];
              
              scope.setSymbolFast(si, pi);
            }
            
            try{
              nvar r = process(b);
              context->popScope();

              if(r.isFunction()){
                NFunc f = (*r).func();
                if(f == _ret0Func){
                  return none;
                }
                else if(f == _ret1Func){
                  return r[0];
                }
              }
              
              return r;
            }
            catch(NError& e){
              context->popScope();
              throw e;
            }
          }
          
          return Throw(v, "failed to process function");
        }
        case nvar::Symbol:{
          nvar p;
          getSymbol(getContext(), vd, p);
          return p;
        }
        default:
          return v;
      }
      
      return v;
    }
    
    nvar Throw(const nvar& v1, const nvar& v2){
      nstr msg = v1.toStr() + ": ";
      
      nstr loc = v1.getLocation();
      if(!loc.empty()){
        msg += loc + ": ";
      }
      
      msg += process(v2).str();
      
      NERROR(msg);
      
      return none;
    }
    
    nvar Reset(){
      if(sharedScope_){
        mainContext_.getScope(2)->clear();
      }
      else{
        mainContext_.getScope(1)->clear();
      }
      
      return none;
    }
    
    nvar Add(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 + p2;
    }
    
    nvar Sub(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 - p2;
    }
    
    nvar Mul(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 * p2;
    }
    
    nvar Div(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 / p2;
    }
    
    nvar Mod(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 % p2;
    }
    
    nvar Neg(const nvar& v){
      nvar p = process(v);
      
      return -p;
    }
    
    nvar AddBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 += p2;
    }
    
    nvar SubBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 -= p2;
    }
    
    nvar MulBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 *= p2;
    }
    
    nvar DivBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 /= p2;
    }
    
    nvar ModBy(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 %= p2;
    }
    
    nvar Inc(const nvar& v){
      nvar p = process(v);
      
      return ++p;
    }
    
    nvar PostInc(const nvar& v){
      nvar p = process(v);
      
      return p++;
    }
    
    nvar Dec(const nvar& v){
      nvar p = process(v);
      
      return --p;
    }
    
    nvar PostDec(const nvar& v){
      nvar p = process(v);
      
      return p--;
    }
    
    nvar LT(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 < p2;
    }
    
    nvar LE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 <= p2;
    }
    
    nvar GT(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 > p2;
    }
    
    nvar GE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 >= p2;
    }
    
    nvar EQ(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 == p2;
    }
    
    nvar NE(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 != p2;
    }
    
    nvar And(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 && p2;
    }
    
    nvar Or(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1 || p2;
    }
    
    nvar Not(const nvar& v){
      nvar p = process(v);
      
      return !p;
    }
    
    nvar Sqrt(const nvar& v){
      nvar p = process(v);
      
      return nvar::sqrt(p, exact_ ? o_ : 0);
    }
    
    nvar Exp(const nvar& v){
      nvar p = process(v);
      
      return nvar::exp(p, exact_ ? o_ : 0);
    }
    
    nvar Abs(const nvar& v){
      nvar p = process(v);
      
      return nvar::abs(p);
    }
    
    nvar Floor(const nvar& v){
      nvar p = process(v);
      
      return nvar::floor(p);
    }
    
    nvar Ceil(const nvar& v){
      nvar p = process(v);
      
      return nvar::ceil(p);
    }
    
    nvar Log10(const nvar& v){
      nvar p = process(v);
      
      return nvar::log10(p, exact_ ? o_ : 0);
    }
    
    nvar Log(const nvar& v){
      nvar p = process(v);
      
      return nvar::log(p, exact_ ? o_ : 0);
    }
    
    nvar Cos(const nvar& v){
      nvar p = process(v);
      
      return nvar::cos(p, exact_ ? o_ : 0);
    }
    
    nvar Acos(const nvar& v){
      nvar p = process(v);
      
      return nvar::acos(p, exact_ ? o_ : 0);
    }
    
    nvar Cosh(const nvar& v){
      nvar p = process(v);
      
      return nvar::cosh(p, exact_ ? o_ : 0);
    }
    
    nvar Sin(const nvar& v){
      nvar p = process(v);
      
      return nvar::sin(p, exact_ ? o_ : 0);
    }
    
    nvar Asin(const nvar& v){
      nvar p = process(v);
      
      return nvar::asin(p, exact_ ? o_ : 0);
    }
    
    nvar Sinh(const nvar& v){
      nvar p = process(v);
      
      return nvar::sinh(p, exact_ ? o_ : 0);
    }
    
    nvar Tan(const nvar& v){
      nvar p = process(v);
      
      return nvar::tan(p, exact_ ? o_ : 0);
    }
    
    nvar Atan(const nvar& v){
      nvar p = process(v);
      
      return nvar::atan(p, exact_ ? o_ : 0);
    }
    
    nvar Tanh(const nvar& v){
      nvar p = process(v);
      
      return nvar::tanh(p, exact_ ? o_ : 0);
    }
    
    nvar Var(const nvar& v){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
#ifndef NEU_FAST
      if(!v.isSymbol()){
        return Throw(v, "Var[0] is not a symbol");
      }
#endif
      
      nvar r = new nvar;
      
#ifndef NEU_FAST
      if(!currentScope->setNewSymbol(v, r)){
        Throw(v, "Symbol Var[0] exists in scope");
      }
#else
      currentScope->setSymbol(v, r);
#endif
      
      return r;
    }
    
    nvar Var(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
#ifndef NEU_FAST
      if(!v1.isSymbol()){
        return Throw(v1, "Var[0] is not a symbol");
      }
#endif
      
      nvar p1 = process(v2);
      
      nvar r = new nvar(p1);
      
#ifndef NEU_FAST
      if(!currentScope->setNewSymbol(v1, r)){
        Throw(v1, "Symbol Var[0] exists in scope");
      }
#else
      currentScope->setSymbol(v1, r);
#endif
      
      return r;
    }
    
    nvar Var(const nvar& v1, const nvar& v2, const nvar& v3){
      ThreadContext* context = getContext();
      NScope* currentScope = context->topScope();
      
#ifndef NEU_FAST
      if(!v1.isSymbol()){
        return Throw(v1, "Var[0] is not a symbol");
      }
#endif
      
      nvar p1 = process(v2);
      
      nvar r;
      if(v3.get("shared", false)){
        r = nvar(p1.obj(), nvar::SharedObject);
      }
      else if(v3.get("local", false)){
        r = nvar(p1.obj(), nvar::LocalObject);
      }
      else{
        r = new nvar(move(p1));
      }
      
#ifndef NEU_FAST
      if(!currentScope->setNewSymbol(v1, r)){
        Throw(v1, "Symbol Var[0] exists in scope");
      }
#else
      currentScope->setSymbol(v1, r);
#endif
      
      return r;
    }
    
    nvar Set(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.set(p2);
    }
    
    nvar VarSet(const nvar& v1, const nvar& v2){
#ifndef NEU_FAST
      if(!v1.isSymbol()){
        return Throw(v1, "VarSet[0] is not a symbol");
      }
#endif
      
      nvar p2 = process(v2);
      
      ThreadContext* context = getContext();
      
      nvar s;
      getSymbolNone(context, v1, s);
      
      if(s.some()){
        s.set(p2);
      }
      else{
        s = new nvar(p2);
        NScope* currentScope = context->topScope();
        
        currentScope->setSymbol(v1, s);
      }
      
      return s;
    }
    
    nvar Get(const nvar& v){
      ThreadContext* context = getContext();
      
      nvar r;
      getSymbolNone(context, v, r);
      
      return r;
    }

    nvar Get(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      nvar& v = *p1.get(p2);
      
      return nvar(&v, nvar::Ptr);
    }
    
    nvar Idx(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      nvar& v = *p1[p2];
      
      return nvar(&v, nvar::Ptr);
    }
    
    nvar Dot(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      nvar& v = *p1[v2];
      
      return nvar(&v, nvar::Ptr);
    }
    
    nvar Put(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      nvar& v = *p1(p2);
      
      return nvar(&v, nvar::Ptr);
    }
    
    nvar DotPut(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      nvar& v = *p1(v2);
      
      return nvar(&v, nvar::Ptr);
    }
    
    nvar Cs(const nvar& v){
      return v;
    }
    
    nvar Call(const nvar& v){
      const nvar& p = *v;
      
      nvar f(v.str(), nvar::Func);
      
      size_t size = p.size();
      for(size_t i = 0; i < size; ++i){
        f << process(p[i]);
      }
      
      return process(f);
    }
    
    nvar Call(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      const nvar& p2 = *v2;
      
      NObject* o = static_cast<NObject*>(p1.obj());
      
      nvar f(v2.str(), nvar::Func);
      
      size_t size = p2.size();
      for(size_t i = 0; i < size; ++i){
        f << process(p2[i]);
      }
      
      return o->process(f);
    }
    
    nvar Def(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      
      NScope* scope = context->topScope();
      
      switch(v1.type()){
        case nvar::Function:
          scope->setFunction(v1, v2);
          break;
        case nvar::Symbol:
          scope->setSymbol(v1, v2);
          break;
        default:
          return Throw(v2, "Def[1] is invalid");
      }
      
      return none;
    }
    
    nvar Def(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      
      NScope* scope;
      
      try{
        scope = toScope(p1);
      }
      catch(NError& e){
        return Throw(v1, "Def[0] is not a scope");
      }
      
      switch(v2.type()){
        case nvar::Function:
          scope->setFunction(v2, v3);
          break;
        case nvar::Symbol:
          scope->setSymbol(v2, v3);
          break;
        default:
          return Throw(v2, "Def[1] is invalid");
      }
      
      return none;
    }
    
    nvar In(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      NObject* o = static_cast<NObject*>(p1.obj());
      
      return o->process(v2);
    }
    
    nvar New(const nvar& v){
      NObjectBase* o = NClass::create(v);
      
      if(o){
        return o;
      }
      
      ThreadContext* context = getContext();

      nstr n = "__class_" + v.str();
      nvar s;
      
      getSymbolNone(context, n, s);
      if(s.some()){
        const nmap& m = s["ctors"];

        auto itr = m.find(v.size());
        if(itr == m.end()){
          return Throw(v, "New[0] invalid ctor");
        }

        const nvar& ctor = itr->second;
        const nvar& f = ctor[1];
        
        NScope scope(true);
        context->pushScope(&scope);

        for(size_t i = 0; i < f.size(); ++i){
          const nvar& si = f[i];
          const nvar& pi = v[i];
          
          scope.setSymbolFast(si, pi);
        }

        NObject* o;
        try{
          o = New(ctor[0]).ptr<NObject>();
        }
        catch(NError& e){
          context->popScope();
          return Throw(v, "New[0] failed to create base object");
        }

        context->popScope();
        
        o->process(s["stmts"]);
        
        o->PushScope(&scope);
        try{
          o->process(ctor[2]);
        }
        catch(NError& e){
          delete o;
          return Throw(v, "New[0] failed to construct object");
        }
        o->PopScope();

        return o;
      }
      
      return Throw(v, "New[0] failed to create: " + v);
    }
    
    nvar New(const nvar& v1, const nvar& v2){
      NObjectBase* o = NClass::create(v1);
      
      if(o){
        if(v2.get("shared", false)){
          return nvar(o, nvar::SharedObject);
        }
        else if(v2.get("local", false)){
          return nvar(o, nvar::LocalObject);
        }
        else{
          return o;
        }
      }
      
      return Throw(v1, "New[0] failed to create: " + v1);
    }

    nvar Ret(const nvar& v){
      nvar p = process(v);
      
      nvar r = nfunc("Ret") << move(p);
      (*r).setFunc(_ret1Func);
      
      return r;
    }
    
    nvar Block_n(const nvar& v){
      size_t size = v.size();
      nvar r = none;

      for(size_t i = 0; i < size; ++i){
        r = process(v[i]);
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func ||
             f == _ret1Func ||
             f == _breakFunc ||
             f == _continueFunc){
            return r;
          }
        }
      }
      
      return r;
    }
    
    nvar ScopedBlock_n(const nvar& v){
      ThreadContext* context = getContext();
      
      NScope scope;
      context->pushScope(&scope);
      
      size_t size = v.size();
      nvar r = none;

      for(size_t i = 0; i < size; ++i){
        try{
          r = process(v[i]);
        }
        catch(NError& e){
          context->popScope();
          throw e;
        }
        
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func ||
             f == _ret1Func ||
             f == _breakFunc ||
             f == _continueFunc){
            context->popScope();
            return r;
          }
        }
      }
      
      context->popScope();
      
      return r;
    }
    
    nvar Print_n(const nvar& v){
      nstr s = process(v[0]);
      
      size_t size = v.size();
      size_t pos = 0;

      for(size_t i = 1; i < size; ++i){
        size_t p = s.find("%v", pos);
      
        if(p == nstr::npos){
          return Throw(v, "Print[" + nvar(i) + "] missing token");
        }

        nstr ri = process(v[i]).toStr();
        
        s.replace(p, 2, ri);

        pos = p + ri.length();
      }
      
      cout << s << endl;
      
      return none;
    }
    
    nvar PushScope(const nvar& v){
      nvar p = process(v);
      
      NScope* scope = toScope(p);
      if(!scope){
        return Throw(v, "PushScope[0] is not a scope");
      }
      
      ThreadContext* context = getContext();
      context->pushScope(scope);

      return none;
    }
    
    nvar PopScope(){
      ThreadContext* context = getContext();
      context->popScope();
      
      return none;
    }
    
    nvar PushBack(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.pushBack(p2);
      
      return none;
    }
    
    nvar TouchMultimap(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.touchMultimap();
      
      return none;
    }
    
    nvar TouchList(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.touchList();
      
      return none;
    }
    
    nvar Keys(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.keys();
    }
    
    nvar PushFront(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.pushFront(p2);
      
      return none;
    }
    
    nvar PopBack(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.popBack();
    }
    
    nvar HasKey(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      return p1.hasKey(p2);
    }
    
    nvar Insert(const nvar& v1, const nvar& v2, const nvar& v3){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      nvar p3 = process(v3);
      
      p1.insert(v2, v3);
      
      return none;
    }
    
    nvar Clear(const nvar& v1){
      nvar p1 = process(v1);
      
      p1.clear();
      
      return none;
    }
    
    nvar Empty(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.empty();
    }
    
    nvar Back(const nvar& v1){
      nvar p1 = process(v1);
      
      return p1.back();
    }
    
    nvar Erase(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.erase(p2);
      
      return none;
    }
    
    nvar Merge(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.merge(p2);
      
      return none;
    }
    
    nvar OuterMerge(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      p1.outerMerge(p2);
      
      return none;
    }
    
    nvar Class(const nvar& v1){
      nstr n = "__class_" + v1["name"].str();
      
      ThreadContext* context = getContext();
      NScope* scope = context->topScope();
      scope->setSymbol(n, v1);
      
      return none;
    }
    
    nvar For(const nvar& v1, const nvar& v2, const nvar& v3, const nvar& v4){
      process(v1);
      
      for(;;){
        nvar c = process(v2);
        if(!c){
          return none;
        }

        nvar r = process(v4);

        bool should = true;
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func || f == _ret1Func){
            return r;
          }
          else if(f == _breakFunc){
            return none;
          }
          else if(f == _continueFunc){
            should = false;
          }
        }
        
        if(should){
          process(v3);
        }
      }
      
      return none;
    }
    
    nvar While(const nvar& v1, const nvar& v2){
      for(;;){
        nvar c = process(v1);
        if(!c){
          return none;
        }
        
        nvar r = process(v2);
        
        bool should = true;
        if(r.isFunction()){
          NFunc f = (*r).func();
          if(f == _ret0Func || f == _ret1Func){
            return r;
          }
          else if(f == _breakFunc){
            return none;
          }
        }
      }
    }
    
    nvar If(const nvar& v1, const nvar& v2){
      if(process(v1)){
        return process(v2);
      }
    }
    
    nvar If(const nvar& v1, const nvar& v2, const nvar& v3){
      if(process(v1)){
        return process(v2);
      }
      else{
        return process(v3);
      }
    }
    
    void foo(nvar& x){
      cout << "called foo" << endl;
      
      x = 29;
    }
    
    NScope* currentScope(){
      ThreadContext* context = getContext();
      
      return context->topScope();
    }
    
    NScope* objectScope(){
      ThreadContext* context = getContext();
      
      return sharedScope_ ? context->scopeStack[2] : context->scopeStack[1];
    }
    
    NScope* toScope(const nvar& v){
      NObjectBase* o = v.obj();
      if(o->instanceOf(NScope::classId)){
        return static_cast<NScope*>(o);
      }
      
      return static_cast<NObject*>(o)->objectScope();
    }
    
    nvar remoteProcess(const nvar& n){
      assert(broker_);
      
      return broker_->process_(o_, n);
    }
    
    void dumpScopes(){
      ThreadContext* context = getContext();
      context->dumpScopes();
    }
    
  private:
    NObject* o_;
    
    ThreadContext mainContext_;
    ThreadData* threadData_;
    NBroker* broker_;
    
    bool exact_ : 1;
    bool strict_ : 1;
    bool sharedScope_ : 1;
  };
  
} // end namespace neu

FuncMap::FuncMap(){
  add("Reset", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Reset();
      });
  
  add("Add", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Add(v[0], v[1]);
      });
  
  add("Sub", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Sub(v[0], v[1]);
      });
  
  add("Mul", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Mul(v[0], v[1]);
      });
  
  add("Div", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Div(v[0], v[1]);
      });
  
  add("Mod", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Mod(v[0], v[1]);
      });
  
  add("Neg", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Neg(v[0]);
      });
  
  add("AddBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        AddBy(v[0], v[1]);
      });
  
  add("SubBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        SubBy(v[0], v[1]);
      });
  
  add("MulBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        MulBy(v[0], v[1]);
      });
  
  add("DivBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        DivBy(v[0], v[1]);
      });
  
  add("ModBy", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        ModBy(v[0], v[1]);
      });
  
  add("Inc", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Inc(v[0]);
      });
  
  add("PostInc", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PostInc(v[0]);
      });
  
  add("Dec", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Dec(v[0]);
      });
  
  add("PostDec", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PostDec(v[0]);
      });
  
  add("LT", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        LT(v[0], v[1]);
      });
  
  add("LE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        LE(v[0], v[1]);
      });
  
  add("GT", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        GT(v[0], v[1]);
      });
  
  add("GE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        GE(v[0], v[1]);
      });
  
  add("EQ", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        EQ(v[0], v[1]);
      });
  
  add("NE", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        NE(v[0], v[1]);
      });
  
  add("And", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        And(v[0], v[1]);
      });
  
  add("Or", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Or(v[0], v[1]);
      });
  
  add("Not", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Not(v[0]);
      });
  
  add("Sqrt", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Sqrt(v[0]);
      });
  
  add("Exp", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Exp(v[0]);
      });
  
  add("Abs", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Abs(v[0]);
      });
  
  add("Floor", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Floor(v[0]);
      });
  
  add("Ceil", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Ceil(v[0]);
      });
  
  add("Log10", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Log10(v[0]);
      });
  
  add("Log", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Log(v[0]);
      });
  
  add("Cos", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Cos(v[0]);
      });
  
  add("Acos", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Acos(v[0]);
      });
  
  add("Cosh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Cosh(v[0]);
      });
  
  add("Sin", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Sin(v[0]);
      });
  
  add("Asin", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Asin(v[0]);
      });
  
  add("Sinh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Sinh(v[0]);
      });
  
  add("Tan", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Tan(v[0]);
      });
  
  add("Atan", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Atan(v[0]);
      });
  
  add("Tanh", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Tanh(v[0]);
      });
  
  add("Var", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Var(v[0]);
      });
  
  add("Var", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Var(v[0], v[1]);
      });
  
  add("Var", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Var(v[0], v[1], v[2]);
      });
  
  add("Set", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Set(v[0], v[1]);
      });
  
  add("VarSet", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        VarSet(v[0], v[1]);
      });
  
  add("Get", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Get(v[0]);
      });

  add("Get", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Get(v[0], v[1]);
      });
  
  add("Idx", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Idx(v[0], v[1]);
      });
  
  add("Dot", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Dot(v[0], v[1]);
      });
  
  add("Put", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Put(v[0], v[1]);
      });
  
  add("DotPut", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        DotPut(v[0], v[1]);
      });
  
  add("Cs", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Cs(v[0]);
      });
  
  add("In", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        In(v[0], v[1]);
      });
  
  add("Call", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Call(v[0]);
      });
  
  add("Call", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Call(v[0], v[1]);
      });
  
  add("Def", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Def(v[0], v[1]);
      });
  
  add("Def", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Def(v[0], v[1], v[2]);
      });
  
  add("New", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        New(v[0]);
      });
  
  add("New", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        New(v[0], v[1]);
      });
  
  add("Block",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Block_n(v);
      });
  
  add("ScopedBlock",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        ScopedBlock_n(v);
      });
  
  add("Print",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Print_n(v);
      });
  
  _ret0Func =
  add("Ret", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  _ret1Func =
  add("Ret", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Ret(v[0]);
      });
  
  _breakFunc =
  add("Break", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  _continueFunc =
  add("Continue", 0,
      [](void* o, const nvar& v) -> nvar{
        return v;
      });
  
  add("PushScope", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PushScope(v[0]);
      });
  
  add("PopScope", 0,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PopScope();
      });
  
  add("PushBack", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PushBack(v[0], v[1]);
      });
  
  add("TouchMultimap", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        TouchMultimap(v[0]);
      });
  
  add("TouchList", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        TouchList(v[0]);
      });
  
  add("Keys", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Keys(v[0]);
      });
  
  add("PushFront", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PushFront(v[0], v[1]);
      });
  
  add("PopBack", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        PopBack(v[0]);
      });
  
  add("HasKey", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        HasKey(v[0], v[1]);
      });
  
  add("Insert", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Insert(v[0], v[1], v[2]);
      });
  
  add("Clear", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Clear(v[0]);
      });
  
  add("Empty", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Empty(v[0]);
      });
  
  add("Back", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Back(v[0]);
      });
  
  add("Erase", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Erase(v[0], v[1]);
      });
  
  add("Merge", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Merge(v[0], v[1]);
      });
  
  add("OuterMerge", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        OuterMerge(v[0], v[1]);
      });

  add("If", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        If(v[0], v[1]);
      });
  
  add("If", 3,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        If(v[0], v[1], v[2]);
      });
  
  add("For", 4,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        For(v[0], v[1], v[2], v[3]);
      });
  
  add("While", 2,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        While(v[0], v[1]);
      });
  
  add("Class", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Class(v[0]);
      });
  
  add("foo", 1,
      [](void* o, const nvar& v) -> nvar{
        NObject_::inner(static_cast<NObject*>(o))->
        foo(*v[0]); return none;
      });
  
  add("dumpScopes", 0,
      [](void* o, const nvar& v) -> nvar{
        NObject_::inner(static_cast<NObject*>(o))->
        dumpScopes(); return none;
      });
}

NObject::NObject(){
  x_ = new NObject_(this);
}

NObject::NObject(NScope* sharedScope){
  x_ = new NObject_(this, sharedScope);
}

NObject::NObject(NBroker* broker){
  x_ = new NObject_(this, broker);
}

NObject::~NObject(){
  delete x_;
}

void NObject::enableThreading(){
  x_->enableThreading();
}

nvar NObject::process(const nvar& v, uint32_t flags){
  return x_->process(v, flags);
}

nvar NObject::remoteProcess(const nvar& v){
  return x_->remoteProcess(v);
}

NFunc NObject::handle(const nvar& v, uint32_t flags){
  return _funcMap.map(v);
}

int NObject::precedence(const nvar& f){
  return _global.precedence(f);
}

void NObject::setStrict(bool flag){
  x_->setStrict(flag);
}

void NObject::setExact(bool flag){
  x_->setExact(flag);
}

bool NObject::isRemote(){
  return x_->isRemote();
}

nvar NObject::Reset(){
  return x_->Reset();
}

nvar NObject::Throw(const nvar& v1, const nvar& v2){
  return x_->Throw(v1, v2);
}

nvar NObject::Add(const nvar& v1, const nvar& v2){
  return x_->Add(v1, v2);
}

nvar NObject::Sub(const nvar& v1, const nvar& v2){
  return x_->Sub(v1, v2);
}

nvar NObject::Mul(const nvar& v1, const nvar& v2){
  return x_->Mul(v1, v2);
}

nvar NObject::Div(const nvar& v1, const nvar& v2){
  return x_->Div(v1, v2);
}

nvar NObject::Mod(const nvar& v1, const nvar& v2){
  return x_->Mod(v1, v2);
}

nvar NObject::Neg(const nvar& v){
  return x_->Neg(v);
}

nvar NObject::AddBy(const nvar& v1, const nvar& v2){
  return x_->AddBy(v1, v2);
}

nvar NObject::SubBy(const nvar& v1, const nvar& v2){
  return x_->SubBy(v1, v2);
}

nvar NObject::MulBy(const nvar& v1, const nvar& v2){
  return x_->MulBy(v1, v2);
}

nvar NObject::DivBy(const nvar& v1, const nvar& v2){
  return x_->DivBy(v1, v2);
}

nvar NObject::ModBy(const nvar& v1, const nvar& v2){
  return x_->ModBy(v1, v2);
}

nvar NObject::Inc(const nvar& v){
  return x_->Inc(v);
}

nvar NObject::PostInc(const nvar& v){
  return x_->PostInc(v);
}

nvar NObject::Dec(const nvar& v){
  return x_->Dec(v);
}

nvar NObject::PostDec(const nvar& v){
  return x_->PostDec(v);
}

nvar NObject::LT(const nvar& v1, const nvar& v2){
  return x_->LT(v1, v2);
}

nvar NObject::LE(const nvar& v1, const nvar& v2){
  return x_->LE(v1, v2);
}

nvar NObject::GT(const nvar& v1, const nvar& v2){
  return x_->GT(v1, v2);
}

nvar NObject::GE(const nvar& v1, const nvar& v2){
  return x_->GE(v1, v2);
}

nvar NObject::EQ(const nvar& v1, const nvar& v2){
  return x_->EQ(v1, v2);
}

nvar NObject::NE(const nvar& v1, const nvar& v2){
  return x_->NE(v1, v2);
}

nvar NObject::And(const nvar& v1, const nvar& v2){
  return x_->And(v1, v2);
}

nvar NObject::Or(const nvar& v1, const nvar& v2){
  return x_->Or(v1, v2);
}

nvar NObject::Not(const nvar& v){
  return x_->Not(v);
}

nvar NObject::Sqrt(const nvar& v){
  return x_->Sqrt(v);
}

nvar NObject::Exp(const nvar& v){
  return x_->Exp(v);
}

nvar NObject::Abs(const nvar& v){
  return x_->Abs(v);
}

nvar NObject::Floor(const nvar& v){
  return x_->Floor(v);
}

nvar NObject::Ceil(const nvar& v){
  return x_->Ceil(v);
}

nvar NObject::Log10(const nvar& v){
  return x_->Log10(v);
}

nvar NObject::Log(const nvar& v){
  return x_->Log(v);
}

nvar NObject::Cos(const nvar& v){
  return x_->Cos(v);
}

nvar NObject::Acos(const nvar& v){
  return x_->Acos(v);
}

nvar NObject::Cosh(const nvar& v){
  return x_->Cosh(v);
}

nvar NObject::Sin(const nvar& v){
  return x_->Sin(v);
}

nvar NObject::Asin(const nvar& v){
  return x_->Asin(v);
}

nvar NObject::Sinh(const nvar& v){
  return x_->Sinh(v);
}

nvar NObject::Tan(const nvar& v){
  return x_->Tan(v);
}

nvar NObject::Atan(const nvar& v){
  return x_->Atan(v);
}

nvar NObject::Tanh(const nvar& v){
  return x_->Tanh(v);
}

nvar NObject::Var(const nvar& v){
  return x_->Var(v);
}

nvar NObject::Var(const nvar& v1, const nvar& v2){
  return x_->Var(v1, v2);
}

nvar NObject::Var(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Var(v1, v2, v3);
}

nvar NObject::Set(const nvar& v1, const nvar& v2){
  return x_->Set(v1, v2);
}

nvar NObject::VarSet(const nvar& v1, const nvar& v2){
  return x_->VarSet(v1, v2);
}

nvar NObject::Get(const nvar& v){
  return x_->Get(v);
}

nvar NObject::Idx(const nvar& v1, const nvar& v2){
  return x_->Idx(v1, v2);
}

nvar NObject::Get(const nvar& v1, const nvar& v2){
  return x_->Get(v1, v2);
}

nvar NObject::Dot(const nvar& v1, const nvar& v2){
  return x_->Dot(v1, v2);
}

nvar NObject::Put(const nvar& v1, const nvar& v2){
  return x_->Put(v1, v2);
}

nvar NObject::DotPut(const nvar& v1, const nvar& v2){
  return x_->DotPut(v1, v2);
}

nvar NObject::Cs(const nvar& v){
  return x_->Cs(v);
}

nvar NObject::In(const nvar& v1, const nvar& v2){
  return x_->In(v1, v2);
}

nvar NObject::Call(const nvar& v){
  return x_->Call(v);
}

nvar NObject::Call(const nvar& v1, const nvar& v2){
  return x_->Call(v1, v2);
}

nvar NObject::Def(const nvar& v1, const nvar& v2){
  return x_->Def(v1, v2);
}

nvar NObject::Def(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Def(v1, v2, v3);
}

nvar NObject::New(const nvar& v){
  return x_->New(v);
}

nvar NObject::New(const nvar& v1, const nvar& v2){
  return x_->New(v1, v2);
}

nvar NObject::PushScope(const nvar& v){
  return x_->PushScope(v);
}

nvar NObject::PopScope(){
  return x_->PopScope();
}

nvar NObject::PushBack(const nvar& v1, const nvar& v2){
  return x_->PushBack(v1, v2);
}

nvar NObject::TouchMultimap(const nvar& v1){
  return x_->TouchMultimap(v1);
}

nvar NObject::TouchList(const nvar& v1){
  return x_->TouchList(v1);
}

nvar NObject::Keys(const nvar& v1){
  return x_->Keys(v1);
}

nvar NObject::PushFront(const nvar& v1, const nvar& v2){
  return x_->PushFront(v1, v2);
}

nvar NObject::PopBack(const nvar& v1){
  return x_->PopBack(v1);
}

nvar NObject::HasKey(const nvar& v1, const nvar& v2){
  return x_->HasKey(v1, v2);
}

nvar NObject::Insert(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->Insert(v1, v2, v3);
}

nvar NObject::Clear(const nvar& v1){
  return x_->Clear(v1);
}

nvar NObject::Empty(const nvar& v1){
  return x_->Empty(v1);
}

nvar NObject::Back(const nvar& v1){
  return x_->Back(v1);
}

nvar NObject::Erase(const nvar& v1, const nvar& v2){
  return x_->Erase(v1, v2);
}

nvar NObject::Merge(const nvar& v1, const nvar& v2){
  return x_->Merge(v1, v2);
}

nvar NObject::OuterMerge(const nvar& v1, const nvar& v2){
  return x_->OuterMerge(v1, v2);
}

nvar NObject::For(const nvar& v1,
                  const nvar& v2,
                  const nvar& v3,
                  const nvar& v4){
  return x_->For(v1, v2, v3, v4);
}

nvar NObject::While(const nvar& v1, const nvar& v2){
  return x_->While(v1, v2);
}

nvar NObject::If(const nvar& v1, const nvar& v2){
  return x_->If(v1, v2);
}

nvar NObject::If(const nvar& v1, const nvar& v2, const nvar& v3){
  return x_->If(v1, v2, v3);
}

void NObject::foo(nvar& x){
  x_->foo(x);
}

NScope* NObject::currentScope(){
  return x_->currentScope();
}

NScope* NObject::objectScope(){
  return x_->objectScope();
}

void NObject::dumpScopes(){
  x_->dumpScopes();
}
