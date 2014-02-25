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

#include <neu/NObject.h>

#include <neu/NFuncMap.h>
#include <neu/NScope.h>
#include <neu/NFactory.h>
#include <neu/NThread.h>
#include <neu/NRWMutex.h>

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
    
    void registerFactory(NFactory* factory){
      const nstr& className = factory->className();
      
      if(factoryMap_.hasKey(className)){
        return;
      }
      
      factoryMap_[className] = factory;
    }
    
    NObjectBase* create(const nvar& f){
      auto itr = factoryMap_.find(f);
      
      if(itr == factoryMap_.end()){
        return 0;
      }
      
      return itr->second->create(f);
    }
    
    NScope* globalScope(){
      return globalScope_;
    }
    
    const int precedence(const nstr& op){
      return precedenceMap_.get(op, -1);
    }
    
  private:
    typedef NMap<nstr, NFactory*> FactoryMap_;
    
    NScope* globalScope_;
    FactoryMap_ factoryMap_;
    nvar precedenceMap_;
  };
  
  Global* _global = 0;
  
  class Factory : public NFactory{
  public:
    Factory() : NFactory("NObject"){
      
    }
    
    NObjectBase* create(const nvar& f){
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
  
  Factory _factory;
  
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
        contextMap_.insert(std::make_pair(threadId, context));
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
    threadData_(0){
      
      NScope* gs = _global->globalScope();
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
    threadData_(0){
      
      NScope* gs = _global->globalScope();
      mainContext_.pushScope(gs);
      mainContext_.pushScope(sharedScope);
      
      NScope* s = new NScope;
      s->setSymbolFast("Global", gs);
      s->setSymbolFast("Shared", sharedScope);
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
          i = sharedScope_ ? 2 : 1;
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
          i = sharedScope_ ? 2 : 1;
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
              return r;
            }
            catch(NError& e){
              context->popScope();
              return none;
            }
          }
          
          NERROR("failed to process: " + v);
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
      nstr msg = process(v2);
      
      NERROR(msg);
      
      return 0;
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
      
      return nvar(&v, nvar::PointerType);
    }
    
    nvar Idx(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      nvar& v = *p1[p2];
      
      return nvar(&v, nvar::PointerType);
    }
    
    nvar Dot(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      nvar& v = *p1[v2];
      
      return nvar(&v, nvar::PointerType);
    }
    
    nvar Put(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      nvar p2 = process(v2);
      
      nvar& v = *p1(p2);
      
      return nvar(&v, nvar::PointerType);
    }
    
    nvar DotPut(const nvar& v1, const nvar& v2){
      nvar p1 = process(v1);
      
      nvar& v = *p1(v2);
      
      return nvar(&v, nvar::PointerType);
    }
    
    nvar Cls(const nvar& v){
      return v;
    }
    
    nvar Call(const nvar& v){
      const nvar& p = *v;
      
      nvar f(v.str(), nvar::FunctionType);
      
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
      
      nvar f(v2.str(), nvar::FunctionType);
      
      size_t size = p2.size();
      for(size_t i = 0; i < size; ++i){
        f << process(p2[i]);
      }
      
      return o->process(f);
    }
    
    nvar Def(const nvar& v1, const nvar& v2){
      ThreadContext* context = getContext();
      
      NScope* scope = context->topScope();
      
      switch(v2.type()){
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
      NObjectBase* o = _global->create(v);
      
      if(o){
        return o;
      }
      
      return Throw(v, "New[0] failed to create: " + v);
    }
    
    nvar Block_n(const nvar& v){
      size_t size = v.size();
      
      nvar r = none;
      for(size_t i = 0; i < size; ++i){
        r = process(v[i]);
      }
      
      return r;
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
    
  private:
    NObject* o_;
    
    ThreadContext mainContext_;
    ThreadData* threadData_;
    
    bool exact_ : 1;
    bool strict_ : 1;
    bool sharedScope_ : 1;
  };
  
} // end namespace neu

FuncMap::FuncMap(){
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
  
  add("Cls", 1,
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Cls(v[0]);
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
  
  add("Block",
      [](void* o, const nvar& v) -> nvar{
        return NObject_::inner(static_cast<NObject*>(o))->
        Block_n(v);
      });
  
  add("foo", 1,
      [](void* o, const nvar& v) -> nvar{
        NObject_::inner(static_cast<NObject*>(o))->
        foo(*v[0]); return none;
      });
}

NObject::NObject(){
  x_ = new NObject_(this);
}

NObject::NObject(NScope* sharedScope){
  x_ = new NObject_(this, sharedScope);
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

NFunc NObject::handle(const nvar& v, uint32_t flags){
  return _funcMap.map(v);
}

void NObject::registerFactory_(NFactory* factory){
  if(!_global){
    _global = new Global;
  }
  
  _global->registerFactory(factory);
}

int NObject::precedence(const nvar& f){
  return _global->precedence(f);
}

NObjectBase* NObject::create(const nvar& f){
  return _global->create(f);
}

void NObject::setStrict(bool flag){
  x_->setStrict(flag);
}

void NObject::setExact(bool flag){
  x_->setExact(flag);
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

nvar NObject::Cls(const nvar& v){
  return x_->Cls(v);
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

void NObject::foo(nvar& x){
  x_->foo(x);
}

NScope* NObject::currentScope(){
  return x_->currentScope();
}

NScope* NObject::objectScope(){
  return x_->objectScope();
}
