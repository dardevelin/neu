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
    FKEY_NO_KEY,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2,
    FKEY_Mod_2,
    FKEY_And_2,
    FKEY_Or_2,
    FKEY_XOr_2,
    FKEY_ShL_2,
    FKEY_ShR_2,
    FKEY_BitAnd_2,
    FKEY_BitOr_2,
    FKEY_BitXOr_2,
    FKEY_BitComplement_1,
    FKEY_Not_1,
    FKEY_EQ_2,
    FKEY_NE_2,
    FKEY_GT_2,
    FKEY_GE_2,
    FKEY_LT_2,
    FKEY_LE_2,
    FKEY_If_2,
    FKEY_If_3,
    FKEY_While_2,
    FKEY_For_4,
    FKEY_AddBy_2,
    FKEY_SubBy_2,
    FKEY_MulBy_2,
    FKEY_DivBy_2,
    FKEY_ModBy_2,
    FKEY_Block_n,
    FKEY_Set_2,
    FKEY_Inc_1,
    FKEY_PostInc_1,
    FKEY_Dec_1,
    FKEY_PostDec_1,
    FKEY_Idx_2,
    FKEY_Len_1,
    FKEY_Size_1,
    FKEY_Neg_1,
    FKEY_Inv_1,
    FKEY_Sqrt_1,
    FKEY_Pow_2,
    FKEY_Exp_1,
    FKEY_Vec_n,
    FKEY_Abs_1,
    FKEY_Log_1,
    FKEY_Log10_1,
    FKEY_Floor_1,
    FKEY_Ceil_1,
    FKEY_Break_0,
    FKEY_Continue_0,
    FKEY_Ret_1,
    FKEY_Ret_0,
    FKEY_O2_1,
    FKEY_Normalize_1,
    FKEY_Magnitude_1,
    FKEY_DotProduct_2,
    FKEY_CrossProduct_2,
    FKEY_Call_1
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
    _funcMap[{"Add", 2}] = FKEY_Add_2;
    _funcMap[{"Sub", 2}] = FKEY_Sub_2;
    _funcMap[{"Mul", 2}] = FKEY_Mul_2;
    _funcMap[{"Div", 2}] = FKEY_Div_2;
    _funcMap[{"Mod", 2}] = FKEY_Mod_2;
    _funcMap[{"And", 2}] = FKEY_And_2;
    _funcMap[{"Or", 2}] = FKEY_Or_2;
    _funcMap[{"XOr", 2}] = FKEY_XOr_2;
    _funcMap[{"ShR", 2}] = FKEY_ShR_2;
    _funcMap[{"ShL", 2}] = FKEY_ShL_2;
    _funcMap[{"BitAnd", 2}] = FKEY_BitAnd_2;
    _funcMap[{"BitOr", 2}] = FKEY_BitOr_2;
    _funcMap[{"BitXOr", 2}] = FKEY_BitXOr_2;
    _funcMap[{"BitComplement", 1}] = FKEY_BitComplement_1;
    _funcMap[{"Not", 1}] = FKEY_Not_1;
    _funcMap[{"EQ", 2}] = FKEY_EQ_2;
    _funcMap[{"NE", 2}] = FKEY_NE_2;
    _funcMap[{"LT", 2}] = FKEY_LT_2;
    _funcMap[{"LE", 2}] = FKEY_LE_2;
    _funcMap[{"GT", 2}] = FKEY_GT_2;
    _funcMap[{"GE", 2}] = FKEY_GE_2;
    _funcMap[{"If", 2}] = FKEY_If_2;
    _funcMap[{"If", 3}] = FKEY_If_3;
    _funcMap[{"Set", 2}] = FKEY_Set_2;
    _funcMap[{"AddBy", 2}] = FKEY_AddBy_2;
    _funcMap[{"SubBy", 2}] = FKEY_SubBy_2;
    _funcMap[{"MulBY", 2}] = FKEY_MulBy_2;
    _funcMap[{"DivBy", 2}] = FKEY_DivBy_2;
    _funcMap[{"ModBy", 2}] = FKEY_ModBy_2;
    _funcMap[{"Block", -1}] = FKEY_Block_n;
    _funcMap[{"Inc", 1}] = FKEY_Inc_1;
    _funcMap[{"PostInc", 1}] = FKEY_PostInc_1;
    _funcMap[{"Dec", 1}] = FKEY_Dec_1;
    _funcMap[{"PostDec", 1}] = FKEY_PostDec_1;
    _funcMap[{"While", 2}] = FKEY_While_2;
    _funcMap[{"For", 4}] = FKEY_For_4;
    _funcMap[{"Idx", 2}] = FKEY_Idx_2;
    _funcMap[{"Len", 1}] = FKEY_Len_1;
    _funcMap[{"Size", 1}] = FKEY_Size_1;
    _funcMap[{"Neg", 1}] = FKEY_Neg_1;
    _funcMap[{"Inv", 1}] = FKEY_Inv_1;
    _funcMap[{"Sqrt", 1}] = FKEY_Sqrt_1;
    _funcMap[{"Pow", 2}] = FKEY_Pow_2;
    _funcMap[{"Exp", 1}] = FKEY_Exp_1;
    _funcMap[{"Vec", -1}] = FKEY_Vec_n;
    _funcMap[{"Abs", 1}] = FKEY_Abs_1;
    _funcMap[{"Log", 1}] = FKEY_Log_1;
    _funcMap[{"Log10", 1}] = FKEY_Log10_1;
    _funcMap[{"Floor", 1}] = FKEY_Floor_1;
    _funcMap[{"Ceil", 1}] = FKEY_Ceil_1;
    _funcMap[{"Break", 0}] = FKEY_Break_0;
    _funcMap[{"Continue", 0}] = FKEY_Continue_0;
    _funcMap[{"Ret", 1}] = FKEY_Ret_1;
    _funcMap[{"Ret", 0}] = FKEY_Ret_0;
    _funcMap[{"O2", 1}] = FKEY_O2_1;
    _funcMap[{"Normalize", 1}] = FKEY_Normalize_1;
    _funcMap[{"Magnitude", 1}] = FKEY_Magnitude_1;
    _funcMap[{"DotProduct", 2}] = FKEY_DotProduct_2;
    _funcMap[{"CrossProduct", 2}] = FKEY_CrossProduct_2;
    _funcMap[{"Call", 1}] = FKEY_Call_1;
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
    functionMap_(functionMap),
    builder_(context_),
    estr_(&cerr){

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
      size_t bits = t.get("bits", 0);
      bool ptr = t.get("ptr", false);
      
      if(bits == 0){
        if(ptr){
          return PointerType::get(Type::getIntNTy(context_, 8), 0);
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
    
    Value* error(const nstr& msg, const nvar& n, int arg=-1){
      nstr as;
      if(arg != -1){
        as = "[" + nvar(arg) + "]";
      }
      
      *estr_ << "NPL compiler error: " << msg << as << ": " <<
      n.toStr() << endl;
      
      foundError_ = true;
      
      return 0;
    }

    Value* getDouble(double v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    Value* getFloat(float v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    Value* getInt1(bool v){
      return ConstantInt::get(context_, APInt(1, v));
    }
    
    Value* getInt8(int8_t v){
      return ConstantInt::get(context_, APInt(8, v, true));
    }
    
    Value* getUInt8(uint8_t v){
      Value* vi = ConstantInt::get(context_, APInt(8, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    Value* getInt16(int16_t v){
      return ConstantInt::get(context_, APInt(16, v, true));
    }
    
    Value* getUInt16(uint16_t v){
      Value* vi = ConstantInt::get(context_, APInt(16, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    Value* getInt32(int32_t v){
      return ConstantInt::get(context_, APInt(32, v, true));
    }
    
    Value* getUInt32(int32_t v){
      Value* vi = ConstantInt::get(context_, APInt(32, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    Value* getInt64(int64_t v){
      return ConstantInt::get(context_, APInt(64, v, true));
    }
    
    Value* getUInt64(uint64_t v){
      Value* vi = ConstantInt::get(context_, APInt(64, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }

    Value* getLValue(const nvar& n){
      if(n.isSymbol()){
        Value* v = getLocal(n);
        if(v){
          return v;
        }
        
        v = getAttribute(n, c_, op_);
        
        if(v){
          return v;
        }
      }
      
      if(!n.isFunction()){
        error("expected an l-value", n);
        return 0;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        // ndm - implement
      }
      
      return 0;
    }

    Type* elementType(Value* v){
      return elementType(v->getType());
    }
    
    Type* elementType(Type* t){
      if(PointerType* pt = dyn_cast<PointerType>(t)){
        return pt->getElementType();
      }
      
      return t;
    }
    
    Value* getNumeric(const nvar& x, Value* l=0){
      if(l){
        Type* t = elementType(l);
        if(IntegerType* it = dyn_cast<IntegerType>(t)){
          ConstantInt::get(it, x.toLong());
        }
        else if(t->isFloatingPointTy()){
          return ConstantFP::get(t, x.toDouble());
        }
        else{
          NERROR("not a numeric type");
        }
      }
      
      switch(x.type()){
        case nvar::False:
          return getInt1(false);
        case nvar::True:
          return getInt1(true);
        case nvar::Integer:
          return getInt64(x.asLong());
        case nvar::Float:
          return getDouble(x.asDouble());
        default:
          NERROR("invalid numeric type");
      }
      
      return 0;
    }
    
    Value* getRValue(const nvar& n, Value* l=0){
      if(n.isNumeric()){
        return getNumeric(n, l);
      }
      else if(n.isSymbol()){
        Value* v = getLocal(n);
        if(v){
          return createLoad(v);
        }
      }

      if(!n.isFunction()){
        error("expected an r-value", n);
        return 0;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        // ndm - implement
      }
      
      return 0;
    }
    
    Value* getLocal(const nstr& s){
      for(LocalScope* localScope : scopeStack_){
        Value* v = localScope->get(s);
        if(v){
          return v;
        }
      }
      
      return 0;
    }

    Type* getPointerType(Value* v){
      return getPointerType(v->getType());
    }
    
    Type* getPointerType(Type* t){
      return PointerType::get(t, 0);
    }
    
    Value* getAttribute(const nstr& s, const nvar& c, Value* op){
      auto itr = attributeMap_.find(s);
      
      if(itr == attributeMap_.end()){
        if(c_.hasKey(s)){
          const nvar& a = c[s];
          Type* t = getPointerType(type(a));

          BasicBlock* cb = builder_.GetInsertBlock();
          builder_.SetInsertPoint(entry_);
          
          Value* ep = builder_.CreateGEP(op, getInt32(a["offset"]));
          nstr name = a.str() + ".ptr";
          Value* vp = builder_.CreateBitCast(ep, t, name.c_str());
          
          builder_.SetInsertPoint(cb);
          
          return vp;
        }
        NERROR("invalid attribute: " + s);
      }
      
      return itr->second;
    }
   
    Value* createLoad(Value* v){
      nstr name = v->getName().str();
      name.findReplace(".ptr", "");
      return builder_.CreateLoad(v, name.c_str());
    }
    
    void createStore(Value* from, Value* to){
      builder_.CreateStore(from, to);
    }
    
    FunctionKey getFunctionKey(const nvar& n){
      FuncMap::const_iterator itr = _funcMap.find({n.str(), n.size()});
      
      if(itr == _funcMap.end()){
        itr = _funcMap.find({n.str(), -1});
      }
      
      if(itr == _funcMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second;
    }
    
    Value* compile(const nvar& n){
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_Block_n:{
          if(n.empty()){
            return getInt32(0);
          }
          
          Value* rv;
          for(size_t i = 0; i < n.size(); ++i){
            rv = compile(n[i]);
            if(!rv){
              return 0;
            }
          }
          
          return rv;
        }
        case FKEY_Set_2:{
          Value* l = getLValue(n[0]);
          Value* r = getRValue(n[1], l);
          
          createStore(r, l);
          
          return getInt64(0);
        }
        default:
          func_->dump();
          NERROR("unimplemented function: " + n);
      }
      
      return 0;
    }
    
    Function* compile(const nvar& code,
                      const nstr& className,
                      const nstr& func,
                      const nstr& className2){
      c_ = code[className];
      c2_ = code[className2.empty() ? className : className2];
      
      f_ = c_[{func, 0}];
      
      pushScope();

      func_ = createFunction(className + "_" + func, "void", {"void*", "void*"});

      Function::arg_iterator aitr = func_->arg_begin();
      aitr->setName("op");
      op_ = aitr;
      ++aitr;
      aitr->setName("op2");
      op2_ = aitr;
      
      entry_ =
      BasicBlock::Create(context_, "entry", func_);
      
      begin_ =
      BasicBlock::Create(context_, "begin", func_);

      builder_.SetInsertPoint(begin_);
      
      foundReturn_ = false;
      foundError_ = false;
      
      Value* b = compile(f_[2]);
      
      popScope();
      
      if(!b){
        func_->eraseFromParent();
        return 0;
      }
      
      builder_.CreateRetVoid();
      
      builder_.SetInsertPoint(entry_);
      builder_.CreateBr(begin_);
      
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

    nvar& getInfo(Value* v){
      auto itr = infoMap_.find(v);
      
      if(itr == infoMap_.end()){
        auto itr = infoMap_.insert({v, undef});
        return itr.first->second;
      }
      
      return itr->second;
    }
    
    bool isSigned(Value* v){
      const nvar& info = getInfo(v);
      return info.get("signed", true);
    }
    
    void setUnsigned(Value* v){
      nvar& info = getInfo(v);
      info("signed") = false;
    }
    
  private:
    typedef NVector<LocalScope*> ScopeStack_;
    typedef NMap<Value*, nvar> InfoMap_;
    typedef NMap<nstr, Value*> AttributeMap_;
    
    LLVMContext& context_;
    Module& module_;

    ScopeStack_ scopeStack_;
    AttributeMap_ attributeMap_;
    FunctionMap functionMap_;
    Function* func_;
    BasicBlock* loopContinue_;
    BasicBlock* loopMerge_;
    BasicBlock* entry_;
    BasicBlock* begin_;
    Value* op_;
    Value* op2_;
    nvar c_;
    nvar c2_;
    nvar f_;
    bool foundReturn_;
    bool foundError_;
    IRBuilder<> builder_;
    ostream* estr_;
    InfoMap_ infoMap_;
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
                    const nstr& func,
                    const nstr& className2){
    
      NPLCompiler compiler(context_, module_, functionMap_);
      
      compiler.compile(code, className, func, className2);
      
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
                           const nstr& func,
                           const nstr& className2){
  return x_->compile(code, className, func, className2);
}
