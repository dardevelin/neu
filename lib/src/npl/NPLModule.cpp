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

#include <neu/NPLModule.h>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"

#include <neu/NPLParser.h>
#include <neu/NBasicMutex.h>

using namespace std;
using namespace llvm;
using namespace neu;

namespace{

  typedef NVector<Type*> TypeVec;
  typedef NMap<pair<nstr, size_t>, Function*> FunctionMap;
  
  enum FunctionKey{
    FKEY_Add_2 = 1,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2
  };
  
  typedef NMap<pair<nstr, int>, FunctionKey> FuncMap;
  
  static FuncMap _funcMap;
  
  enum SymbolKey{
    SKEY_true = 1,
    SKEY_false,
    SKEY_this,
    SKEY_that
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  static SymbolMap _symMap;
  
  static void _initFunctionMap(){
    _funcMap[make_pair("Add", 2)] = FKEY_Add_2;
    _funcMap[make_pair("Sub", 2)] = FKEY_Sub_2;
    _funcMap[make_pair("Mul", 2)] = FKEY_Mul_2;
    _funcMap[make_pair("Div", 2)] = FKEY_Div_2;
  }
  
  static void _initSymbolMap(){
    _symMap["true"] = SKEY_true;
    _symMap["false"] = SKEY_false;
    _symMap["this"] = SKEY_this;
    _symMap["that"] = SKEY_that;
  }
  
  class NPLCompiler{
  public:
    
    class LocalScope{
    public:
      Value* get(const nstr& s){
        auto itr = scopeMap_.find(s);
        
        if(itr == scopeMap_.end()){
          return 0;
        }
        
        return itr->second;
      }
      
      void put(const nstr& s, Value* vp){
        scopeMap_[s] = vp;
      }
    
    private:
      typedef NMap<nstr, Value*> ScopeMap_;
      
      ScopeMap_ scopeMap_;
    };
    
    NPLCompiler(LLVMContext& context, Module& module, FunctionMap& functionMap)
    : context_(context),
    module_(module),
    functionMap_(functionMap){

    }
    
    ~NPLCompiler(){
      
    }
    
    TypeVec typeVec(const nvec& v){
      TypeVec tv;
      for(const nstr& vi : v){
        tv.push_back(type(vi));
      }
      
      return tv;
    }
    
    Type* type(const char* t){
      return type(NPLParser::parseType(t));
    }
    
    Type* type(const nstr& t){
      return type(NPLParser::parseType(t));
    }
    
    Type* type(const nvar& t){
      size_t bits = t["bits"];
      bool ptr = t.get("ptr", false);
      
      if(bits == 0){
        if(ptr){
          return llvm::PointerType::get(Type::getIntNTy(context_, 8), 0);
        }
        
        return Type::getVoidTy(context_);
      }
      
      size_t len = t.get("len", 0);
      bool isFloat = t.get("float", false);
      
      Type* baseType;
      
      if(isFloat){
        if(bits == 64){
          baseType = Type::getDoubleTy(context_);
        }
        else if(bits == 32){
          baseType = Type::getFloatTy(context_);
        }
        else{
          NERROR("invalid float type: " + t);
        }
      }
      else{
        baseType = Type::getIntNTy(context_, bits);
      }
      
      if(len > 0){
        Type* vecType = VectorType::get(baseType, len);
        
        if(ptr){
          return PointerType::get(vecType, 0);
        }

        return vecType;
      }
      else if(ptr){
        return PointerType::get(baseType, 0);
      }

      return baseType;
    }

    Function* createFunction(const nstr& name,
                             const nstr& returnType,
                             const nvec& argTypes,
                             bool external=true){
      FunctionType* ft =
      FunctionType::get(type(returnType), typeVec(argTypes).vector(), false);
      
      Function* f =
      Function::Create(ft,
                       external ?
                       Function::ExternalLinkage : Function::InternalLinkage,
                       name.c_str(), &module_);
      
      functionMap_[{name, argTypes.size()}] = f;
      
      return f;
    }
    
    Function* compile(const nvar& code, const nstr& className, const nstr& func){
      c_ = code[className];
      f_ = c_[{func, 0}];
      
      pushScope();

      func_ = createFunction(className + "_" + func, "void", {"void*", "void*"});
      
      entry_ =
      BasicBlock::Create(context_, "entry", func_);
      
      Function::arg_iterator aitr = func_->arg_begin();
      aitr->setName("op");
      op_ = aitr;
      ++aitr;
      aitr->setName("op2");
      op2_ = aitr;
      
      func_->dump();
      
      return func_;
    }
    
    LocalScope* pushScope(){
      LocalScope* scope = new LocalScope;
      scopeStack_.push_back(scope);
      
      return scope;
    }
    
    void popScope(){
      assert(!scopeStack_.empty());
      
      LocalScope* scope = scopeStack_.back();
      delete scope;
      scopeStack_.pop_back();
    }
    
  private:
    typedef NVector<LocalScope*> ScopeStack_;
    
    LLVMContext& context_;
    Module& module_;

    ScopeStack_ scopeStack_;
    FunctionMap functionMap_;
    Function* func_;
    BasicBlock* entry_;
    Value* op_;
    Value* op2_;
    nvar c_;
    nvar f_;
  };
  
  class Global{
  public:
    Global()
    : context_(getGlobalContext()),
    module_("global", context_){
      
      InitializeNativeTarget();
      
      _initFunctionMap();
      _initSymbolMap();
      
      NPLCompiler compiler(context_, module_, functionMap_);
      
      compiler.createFunction("llvm.sqrt.f64", "double", {"double"});
      compiler.createFunction("llvm.sqrt.f32", "float", {"float"});
      compiler.createFunction("llvm.pow.f64", "double", {"double", "double"});
      compiler.createFunction("llvm.pow.f32", "float", {"float", "float"});
      compiler.createFunction("llvm.log.f64", "double", {"double"});
      compiler.createFunction("llvm.log.f32", "float", {"float"});
      compiler.createFunction("llvm.exp.f64", "double", {"double"});
      compiler.createFunction("llvm.exp.f32", "float", {"float"});
    }
    
  private:
    LLVMContext& context_;
    Module module_;
    FunctionMap functionMap_;
  };
  
  NBasicMutex _mutex;
  Global* _global;
  
} // end namespace

namespace neu{
  
  class NPLModule_{
  public:
    NPLModule_(NPLModule* o)
    : o_(o),
    context_(getGlobalContext()),
    module_("module", context_){
      
      initGlobal();
      
      engine_ = EngineBuilder(&module_).create();
    }
    
    ~NPLModule_(){
      // ndm - do we need to delete engine_?
    }
    
    NPLFunc compile(const nvar& code,
                    const nstr& className,
                    const nstr& func){
    
      NPLCompiler compiler(context_, module_, functionMap_);
      
      compiler.compile(code, className, func);
      
      return 0;
    }
    
    void initGlobal(){
      _mutex.lock();
      if(!_global){
        _global = new Global;
      }
      _mutex.unlock();
    }
    
  private:
    NPLModule* o_;
    
    LLVMContext& context_;
    Module module_;
    ExecutionEngine* engine_;
    FunctionMap functionMap_;
  };
  
} // end namespace neu

NPLModule::NPLModule(){
  x_ = new NPLModule_(this);
}

NPLModule::~NPLModule(){
  delete x_;
}

NPLFunc NPLModule::compile(const nvar& code,
                           const nstr& className,
                           const nstr& func){
  return x_->compile(code, className, func);
}
