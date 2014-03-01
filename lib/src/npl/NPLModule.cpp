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
  typedef NMap<nvar, Function*> FunctionMap;
  
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
  
  typedef NMap<pair<nstr, int>, FunctionKey> FunctionKeyMap;
  
  static FunctionKeyMap _functionMap;
  
  enum SymbolKey{
    SKEY_true = 1,
    SKEY_false,
    SKEY_this,
    SKEY_that
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  static SymbolMap _symbolMap;
  
  static void _initFunctionMap(){
    _functionMap[{"Add", 2}] = FKEY_Add_2;
    _functionMap[{"Sub", 2}] = FKEY_Sub_2;
    _functionMap[{"Mul", 2}] = FKEY_Mul_2;
    _functionMap[{"Div", 2}] = FKEY_Div_2;
    _functionMap[{"Mod", 2}] = FKEY_Mod_2;
    _functionMap[{"And", 2}] = FKEY_And_2;
    _functionMap[{"Or", 2}] = FKEY_Or_2;
    _functionMap[{"XOr", 2}] = FKEY_XOr_2;
    _functionMap[{"ShR", 2}] = FKEY_ShR_2;
    _functionMap[{"ShL", 2}] = FKEY_ShL_2;
    _functionMap[{"BitAnd", 2}] = FKEY_BitAnd_2;
    _functionMap[{"BitOr", 2}] = FKEY_BitOr_2;
    _functionMap[{"BitXOr", 2}] = FKEY_BitXOr_2;
    _functionMap[{"BitComplement", 1}] = FKEY_BitComplement_1;
    _functionMap[{"Not", 1}] = FKEY_Not_1;
    _functionMap[{"EQ", 2}] = FKEY_EQ_2;
    _functionMap[{"NE", 2}] = FKEY_NE_2;
    _functionMap[{"LT", 2}] = FKEY_LT_2;
    _functionMap[{"LE", 2}] = FKEY_LE_2;
    _functionMap[{"GT", 2}] = FKEY_GT_2;
    _functionMap[{"GE", 2}] = FKEY_GE_2;
    _functionMap[{"If", 2}] = FKEY_If_2;
    _functionMap[{"If", 3}] = FKEY_If_3;
    _functionMap[{"Set", 2}] = FKEY_Set_2;
    _functionMap[{"AddBy", 2}] = FKEY_AddBy_2;
    _functionMap[{"SubBy", 2}] = FKEY_SubBy_2;
    _functionMap[{"MulBY", 2}] = FKEY_MulBy_2;
    _functionMap[{"DivBy", 2}] = FKEY_DivBy_2;
    _functionMap[{"ModBy", 2}] = FKEY_ModBy_2;
    _functionMap[{"Block", -1}] = FKEY_Block_n;
    _functionMap[{"Inc", 1}] = FKEY_Inc_1;
    _functionMap[{"PostInc", 1}] = FKEY_PostInc_1;
    _functionMap[{"Dec", 1}] = FKEY_Dec_1;
    _functionMap[{"PostDec", 1}] = FKEY_PostDec_1;
    _functionMap[{"While", 2}] = FKEY_While_2;
    _functionMap[{"For", 4}] = FKEY_For_4;
    _functionMap[{"Idx", 2}] = FKEY_Idx_2;
    _functionMap[{"Len", 1}] = FKEY_Len_1;
    _functionMap[{"Size", 1}] = FKEY_Size_1;
    _functionMap[{"Neg", 1}] = FKEY_Neg_1;
    _functionMap[{"Inv", 1}] = FKEY_Inv_1;
    _functionMap[{"Sqrt", 1}] = FKEY_Sqrt_1;
    _functionMap[{"Pow", 2}] = FKEY_Pow_2;
    _functionMap[{"Exp", 1}] = FKEY_Exp_1;
    _functionMap[{"Vec", -1}] = FKEY_Vec_n;
    _functionMap[{"Abs", 1}] = FKEY_Abs_1;
    _functionMap[{"Log", 1}] = FKEY_Log_1;
    _functionMap[{"Log10", 1}] = FKEY_Log10_1;
    _functionMap[{"Floor", 1}] = FKEY_Floor_1;
    _functionMap[{"Ceil", 1}] = FKEY_Ceil_1;
    _functionMap[{"Break", 0}] = FKEY_Break_0;
    _functionMap[{"Continue", 0}] = FKEY_Continue_0;
    _functionMap[{"Ret", 1}] = FKEY_Ret_1;
    _functionMap[{"Ret", 0}] = FKEY_Ret_0;
    _functionMap[{"O2", 1}] = FKEY_O2_1;
    _functionMap[{"Normalize", 1}] = FKEY_Normalize_1;
    _functionMap[{"Magnitude", 1}] = FKEY_Magnitude_1;
    _functionMap[{"DotProduct", 2}] = FKEY_DotProduct_2;
    _functionMap[{"CrossProduct", 2}] = FKEY_CrossProduct_2;
    _functionMap[{"Call", 1}] = FKEY_Call_1;
  }
  
  static void _initSymbolMap(){
    _symbolMap["true"] = SKEY_true;
    _symbolMap["false"] = SKEY_false;
    _symbolMap["this"] = SKEY_this;
    _symbolMap["that"] = SKEY_that;
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
    
    NPLCompiler(Module& module, FunctionMap& functionMap)
    : module_(module),
    context_(module.getContext()),
    builder_(context_),
    estr_(&cerr),
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
      size_t bits = t.get("bits", 0);
      size_t ptr = t.get("ptr", 0);
      
      if(bits == 0){
        if(ptr > 0){
          Type* pt = PointerType::get(Type::getIntNTy(context_, 8), 0);
          for(size_t i = 1; i < ptr; ++i){
            pt = PointerType::get(pt, 0);
          }
          
          return pt;
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
        
        if(ptr > 0){
          Type* pt = PointerType::get(vecType, 0);

          for(size_t i = 1; i < ptr; ++i){
            pt = PointerType::get(pt, 0);
          }
          
          return pt;
        }

        return vecType;
      }
      else if(ptr){
        Type* pt = PointerType::get(baseType, 0);
        
        for(size_t i = 1; i < ptr; ++i){
          pt = PointerType::get(pt, 0);
        }
        
        return pt;
      }

      return baseType;
    }

    Function* createFunction(const nstr& name,
                             const Type* returnType,
                             const TypeVec& argTypes,
                             bool external=true){
      FunctionType* ft =
      FunctionType::get(type(returnType), argTypes.vector(), false);
      
      Function* f =
      Function::Create(ft,
                       external ?
                       Function::ExternalLinkage : Function::InternalLinkage,
                       name.c_str(), &module_);
      
      return f;
    }
    
    Function* createFunction(const nstr& name,
                             const nstr& returnType,
                             const nvec& argTypes,
                             bool external=true){
      
      return createFunction(name, type(returnType), typeVec(argTypes), external);
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
        
        v = getAttribute(n);
        
        if(v){
          return v;
        }
        
        error("undefined symbol", n);
        
        return 0;
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

    Value* getRValue(const nvar& n, Value* l=0){
      if(n.isNumeric()){
        return getNumeric(n, l);
      }
      else if(n.isSymbol()){
        Value* v = getLocal(n);
        if(v){
          return createLoad(v);
        }
        
        v = getAttribute(n);
        
        if(v){
          return createLoad(v);
        }
        
        error("undefined symbol", n);
        
        return 0;
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
  
    Value* convertNum(Value* from, Value* to, bool trunc=false){
      return convertNum(from, to->getType(), trunc);
    }
    
    Value* convertNum(Value* from, Type* toType, bool trunc=false){
      if(toType->isPointerTy()){
        toType = elementType(toType);
      }
      
      nstr name = from->getName().str();
      
      Type* fromType = from->getType();
      Type* fromElementType = elementType(fromType);
      Type* toElementType = elementType(toType);
      
      if(IntegerType* toIntType = dyn_cast<IntegerType>(toElementType)){
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();
          size_t toBits = toIntType->getBitWidth();
          name += ".i" + nvar(toBits);
          
          if(fromBits > toBits){
            return trunc ? builder_.CreateTrunc(from, toType, name.c_str()) : 0;
          }
          else if(fromBits < toBits){
            return builder_.CreateSExt(from, toType, name.c_str());
          }
          else{
            return from;
          }
        }
        else if(fromType->isDoubleTy()){
          return trunc ? builder_.CreateFPToSI(from, toType, name.c_str()) : 0;
        }
        else if(fromType->isFloatTy()){
          return trunc ? builder_.CreateFPToSI(from, toType, name.c_str()) : 0;
        }
      }
      else if(toElementType->isDoubleTy()){
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();

          name += ".f64";
          
          return trunc || fromBits <= 32 ?
          builder_.CreateSIToFP(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isDoubleTy()){
          return from;
        }
        else if(fromElementType->isFloatTy()){
          return builder_.CreateFPExt(from, toType, name.c_str());
        }
      }
      else if(toElementType->isFloatTy()){
        name += ".f32";
        
        if(IntegerType* fromIntType = dyn_cast<IntegerType>(fromElementType)){
          size_t fromBits = fromIntType->getBitWidth();
          
          return trunc || fromBits <= 32 ?
          builder_.CreateSIToFP(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isDoubleTy()){
          return trunc ? builder_.CreateFPTrunc(from, toType, name.c_str()) : 0;
        }
        else if(fromElementType->isFloatTy()){
          return from;
        }
      }
      
      return 0;
    }
    
    Value* convert(Value* from, Type* toType, bool trunc=false){
      Type* fromType = from->getType();
      if(VectorType* fromVecType = dyn_cast<VectorType>(fromType)){
        if(VectorType* toVecType = dyn_cast<VectorType>(toType)){
          size_t fromN = fromVecType->getNumElements();
          size_t toN = toVecType->getNumElements();
          
          if(fromN != toN){
            return 0;
          }

          return convertNum(from, toVecType, trunc);
        }
        else{
          return 0;
        }
      }
      
      return convertNum(from, toType, trunc);
    }
    
    Value* convert(Value* from, Value* to, bool trunc=false){
      return convert(from, to->getType(), trunc);
    }
    
    Type* elementType(Value* v){
      return elementType(v->getType());
    }
    
    Type* elementType(Type* t){
      if(PointerType* pt = dyn_cast<PointerType>(t)){
        return pt->getElementType();
      }
      else if(SequentialType* st = dyn_cast<PointerType>(t)){
        return st->getElementType();
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
    
    Value* getLocal(const nstr& s){
      for(LocalScope* localScope : scopeStack_){
        Value* v = localScope->get(s);
        if(v){
          return v;
        }
      }
      
      return 0;
    }

    Type* pointerType(Value* v){
      return pointerType(v->getType());
    }
    
    Type* pointerType(Type* t){
      return PointerType::get(t, 0);
    }
    
    void dump(Value* v){
      cerr << "value ---------------" << endl;
      v->dump();
      cerr << endl;
    }
    
    void dump(Type* t){
      cerr << "type ---------------" << endl;
      t->dump();
      cerr << endl;
    }
    
    Value* getAttribute(const nstr& s){
      auto itr = attributeMap_.find(s);
      
      if(itr == attributeMap_.end()){
        const nvar& c = currentClass();
        
        if(c.hasKey(s)){
          const nvar& a = c[s];

          BasicBlock* cb = builder_.GetInsertBlock();
          builder_.SetInsertPoint(entry_);
          
          Value* ap = createStructGEP(this_, a["index"], s + ".ptr");

          builder_.SetInsertPoint(cb);
          
          return ap;
        }
        
        return 0;
      }
      
      return itr->second;
    }
   
    Value* createGEP(Value* ptr, Value* index){
      return builder_.CreateGEP(ptr, index);
    }
    
    Value* createStructGEP(Value* structPtr, size_t index, const nstr& name){
      return builder_.CreateStructGEP(structPtr, index, name.c_str());
    }
    
    Value* createLoad(Value* ptr){
      nstr name = ptr->getName().str();
      
      if(name.endsWith(".ptr")){
        name.erase(name.length() - 4, 4);
      }
      
      return builder_.CreateLoad(ptr, name.c_str());
    }
    
    void createStore(Value* v, Value* ptr){
      builder_.CreateStore(v, ptr);
    }
    
    FunctionKey getFunctionKey(const nvar& n){
      FunctionKeyMap::const_iterator itr = _functionMap.find({n.str(), n.size()});
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find({n.str(), -1});
      }
      
      if(itr == _functionMap.end()){
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

          if(!l || !r){
            return 0;
          }
          
          Value* rc = convert(r, l, false);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          createStore(rc, l);
          
          return getInt64(0);
        }
        default:
          func_->dump();
          NERROR("unimplemented function: " + n);
      }
      
      return 0;
    }
    
    const nvar& code(){
      return *code_;
    }
    
    const nvar& currentClass(){
      return *currentClass_;
    }
    
    const nvar& currentFunc(){
      return *currentFunc_;
    }
    
    void compileTop(const nvar& code){
      code_ = &code;
      
      nvec cs;
      code.keys(cs);
      
      for(size_t i = 0; i < cs.size(); ++i){
        const nstr& ck = cs[i];
        const nvar& ci = code[ck];
        
        currentClass_ = &ci;
        
        nvec ms;
        ci.keys(ms);
        
        TypeVec tv;
       
        nvar am;
        
        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];
          
          if(mk.size() == 0){
            am(mj["index"]) = mj;
          }
        }
        
        nvec keys;
        am.keys(keys);
        for(const nvar& k : keys){
          tv.push_back(type(am[k]));
        }
        
        classStruct_ = StructType::create(context_, tv.vector(), ck.c_str());
        structMap_[ck] = classStruct_;
        
        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];

          if(mk.size() == 2){
            nvar fk = {ck, mk[0], mk[1]};
            
            if(functionMap_.hasKey(fk)){
              continue;
            }

            currentFunc_ = &mj;
            
            Function* f = compileFunction(ck, mj);
            
            if(f){
              functionMap_[fk] = f;
            }
          }
        }
      }
      
      module_.dump();
    }
    
    Function* compileFunction(const nstr& className, const nvar& f){
      const nvar& fs = f[1];
      
      nstr n = className + "_" + fs.str() + "_" + fs.size();

      TypeVec args;
      args.push_back(type("void*"));
      args.push_back(pointerType(classStruct_));
      
      for(const nvar& a : fs){
        args.push_back(type(a));
      }
      
      nstr an = n + "_args";
      
      argsStruct_ =
      StructType::create(context_, args.vector(), an.c_str());
      
      func_ = createFunction(n, type("void"), {pointerType(argsStruct_)});
      
      Function::arg_iterator aitr = func_->arg_begin();
      aitr->setName("args");
      args_ = aitr;
      
      entry_ =
      BasicBlock::Create(context_, "entry", func_);
      
      builder_.SetInsertPoint(entry_);

      this_ = createLoad(createStructGEP(args_, 1, "this.ptr"));
      
      pushScope();
      
      for(size_t i = 0; i < fs.size(); ++i){
        const nstr& s = fs[i];
        
        Value* v = createStructGEP(args_, i + 2, s + ".ptr");
        putLocal(s, v);
      }
      
      begin_ =
      BasicBlock::Create(context_, "begin", func_);
      
      builder_.SetInsertPoint(begin_);
      
      foundReturn_ = false;
      foundError_ = false;
      
      Value* b = compile(f[2]);
      
      popScope();
      
      if(!b){
        func_->eraseFromParent();
        return 0;
      }
      
      builder_.CreateRetVoid();
      
      builder_.SetInsertPoint(entry_);
      builder_.CreateBr(begin_);
      
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

    void putLocal(const nstr& s, Value* v){
      assert(!scopeStack_.empty());
      
      LocalScope* scope = scopeStack_.back();
      scope->put(s, v);
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
    typedef NMap<nstr, StructType*> StructMap_;
    
    LLVMContext& context_;
    Module& module_;

    ScopeStack_ scopeStack_;
    AttributeMap_ attributeMap_;
    FunctionMap& functionMap_;
    Function* func_;
    BasicBlock* loopContinue_;
    BasicBlock* loopMerge_;
    BasicBlock* entry_;
    BasicBlock* begin_;
    Value* args_;
    bool foundReturn_;
    bool foundError_;
    IRBuilder<> builder_;
    ostream* estr_;
    InfoMap_ infoMap_;
    const nvar* code_;
    const nvar* currentClass_;
    const nvar* currentFunc_;
    StructMap_ structMap_;
    StructType* classStruct_;
    StructType* argsStruct_;
    Value* this_;
  };
  
  class Global{
  public:
    Global()
    : context_(getGlobalContext()),
    module_("global", context_){
      
      InitializeNativeTarget();
      
      _initFunctionMap();
      _initSymbolMap();
      
      NPLCompiler compiler(module_, functionMap_);
      
      functionMap_["llvm.sqrt.f64"] =
      compiler.createFunction("llvm.sqrt.f64", "double", {"double"});
      
      functionMap_["llvm.sqrt.f32"] =
      compiler.createFunction("llvm.sqrt.f32", "float", {"float"});
      
      functionMap_["llvm.pow.f64"] =
      compiler.createFunction("llvm.pow.f64", "double", {"double", "double"});
      
      functionMap_["llvm.pow.f32"] =
      compiler.createFunction("llvm.pow.f32", "float", {"float", "float"});
      
      functionMap_["llvm.log.f64"] =
      compiler.createFunction("llvm.log.f64", "double", {"double"});
      
      functionMap_["llvm.log.f32"] =
      compiler.createFunction("llvm.log.f32", "float", {"float"});
      
      functionMap_["llvm.exp.f64"] =
      compiler.createFunction("llvm.exp.f64", "double", {"double"});
      
      functionMap_["llvm.exp.f32"] =
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
    
    bool compile(const nvar& code){
      NPLCompiler compiler(module_, functionMap_);
      
      compiler.compileTop(code);
      
      return true;
    }
    
    void getFunc(const nvar& func, NPLFunc* f){
      auto itr = functionMap_.find(func);
      
      if(itr == functionMap_.end()){
        NERROR("invalid function: " + func);
      }
      
      f->fp = (NPLFunc::FP)engine_->getPointerToFunction(itr->second);
      
      functionPtrMap_[f->fp] = itr->second;
    }
    
    void release(NPLFunc* f){
      auto itr = functionPtrMap_.find(f->fp);
      
      if(itr == functionPtrMap_.end()){
        NERROR("invalid function");
      }
      
      engine_->freeMachineCodeForFunction(itr->second);
    }
    
    void initGlobal(){
      _mutex.lock();
      if(!_global){
        _global = new Global;
      }
      _mutex.unlock();
    }
    
  private:
    typedef NMap<NPLFunc::FP, Function*> FunctionPtrMap_;
    
    NPLModule* o_;
    
    LLVMContext& context_;
    Module module_;
    ExecutionEngine* engine_;
    FunctionMap functionMap_;
    FunctionPtrMap_ functionPtrMap_;
  };
  
} // end namespace neu

NPLModule::NPLModule(){
  x_ = new NPLModule_(this);
}

NPLModule::~NPLModule(){
  delete x_;
}

bool NPLModule::compile(const nvar& code){
  return x_->compile(code);
}

void NPLModule::getFunc(const nvar& func, NPLFunc* f){
  x_->getFunc(func, f);
}

void NPLModule::release(NPLFunc* f){
  x_->release(f);
}
