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
  typedef NVector<Value*> ValueVec;
  typedef NMap<nvar, Function*, nvarLess<nvar>> FunctionMap;
  
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
    FKEY_Select_3,
    FKEY_Switch_2,
    FKEY_Local_1,
    FKEY_Local_2,
    FKEY_While_2,
    FKEY_For_4,
    FKEY_AddBy_2,
    FKEY_SubBy_2,
    FKEY_MulBy_2,
    FKEY_DivBy_2,
    FKEY_ModBy_2,
    FKEY_Block_n,
    FKEY_ScopedBlock_n,
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
    FKEY_Normalize_1,
    FKEY_Magnitude_1,
    FKEY_DotProduct_2,
    FKEY_CrossProduct_2,
    FKEY_Call_1,
    FKEY_Ptr_1,
    FKEY_DePtr_1,
    FKEY_Float_1
  };
  
  typedef NMap<pair<nstr, int>, FunctionKey> FunctionKeyMap;
  
  static FunctionKeyMap _functionMap;
  
  enum SymbolKey{
    SKEY_NO_KEY,
    SKEY_this,
  };
  
  typedef NMap<nstr, SymbolKey> SymbolKeyMap;
  
  static SymbolKeyMap _symbolMap;
  
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
    _functionMap[{"Select", 3}] = FKEY_Select_3;
    _functionMap[{"Switch", 2}] = FKEY_Switch_2;
    _functionMap[{"Set", 2}] = FKEY_Set_2;
    _functionMap[{"AddBy", 2}] = FKEY_AddBy_2;
    _functionMap[{"SubBy", 2}] = FKEY_SubBy_2;
    _functionMap[{"MulBY", 2}] = FKEY_MulBy_2;
    _functionMap[{"DivBy", 2}] = FKEY_DivBy_2;
    _functionMap[{"ModBy", 2}] = FKEY_ModBy_2;
    _functionMap[{"Block", -1}] = FKEY_Block_n;
    _functionMap[{"ScopedBlock", -1}] = FKEY_ScopedBlock_n;
    _functionMap[{"Inc", 1}] = FKEY_Inc_1;
    _functionMap[{"PostInc", 1}] = FKEY_PostInc_1;
    _functionMap[{"Dec", 1}] = FKEY_Dec_1;
    _functionMap[{"PostDec", 1}] = FKEY_PostDec_1;
    _functionMap[{"Local", 1}] = FKEY_Local_1;
    _functionMap[{"Local", 2}] = FKEY_Local_2;
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
    _functionMap[{"Normalize", 1}] = FKEY_Normalize_1;
    _functionMap[{"Magnitude", 1}] = FKEY_Magnitude_1;
    _functionMap[{"DotProduct", 2}] = FKEY_DotProduct_2;
    _functionMap[{"CrossProduct", 2}] = FKEY_CrossProduct_2;
    _functionMap[{"Call", 1}] = FKEY_Call_1;
    _functionMap[{"Ptr", 1}] = FKEY_Ptr_1;
    _functionMap[{"DePtr", 1}] = FKEY_DePtr_1;
    _functionMap[{"Float", 1}] = FKEY_Float_1;
  }
  
  static void _initSymbolMap(){
    _symbolMap["this"] = SKEY_this;
  }
  
  class Global{
  public:
    Global();
    
    Function* getFunction(const nstr& f);
    
  private:
    LLVMContext& context_;
    Module module_;
    FunctionMap functionMap_;
  };
  
  NBasicMutex _mutex;
  Global* _global;
  
  Function* _globalFunc(const nstr& f){
    return _global->getFunction(f);
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
    
    NPLCompiler(Module& module, FunctionMap& functionMap, ostream& estr)
    : module_(module),
    context_(module.getContext()),
    builder_(context_),
    estr_(&estr),
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
                             Type* returnType,
                             const TypeVec& argTypes,
                             bool external=true){
      FunctionType* ft =
      FunctionType::get(returnType, argTypes.vector(), false);
      
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
    
    Value* error(const nstr& msg, const nvar& n){
      size_t line = n.get("__line", 0);
      nstr file = n.get("__file", "");
      
      *estr_ << "NPL compiler error:";
      
      if(line > 0){
        *estr_ << file << ":" << line << ":";
      }
      
      *estr_ << " " << msg << ": " << n.toStr() << endl;
      
      foundError_ = true;
      
      return 0;
    }

    Value* getDouble(double v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    Value* getFloat(float v){
      return ConstantFP::get(context_, APFloat(v));
    }
    
    ConstantInt* getInt1(bool v){
      return ConstantInt::get(context_, APInt(1, v));
    }
    
    ConstantInt* getInt8(int8_t v){
      return ConstantInt::get(context_, APInt(8, v, true));
    }
    
    ConstantInt* getUInt8(uint8_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(8, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt16(int16_t v){
      return ConstantInt::get(context_, APInt(16, v, true));
    }
    
    ConstantInt* getUInt16(uint16_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(16, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt32(int32_t v){
      return ConstantInt::get(context_, APInt(32, v, true));
    }
    
    ConstantInt* getUInt32(int32_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(32, v, false));
      
      setUnsigned(vi);
      
      return vi;
    }
    
    ConstantInt* getInt64(int64_t v){
      return ConstantInt::get(context_, APInt(64, v, true));
    }
    
    ConstantInt* getUInt64(uint64_t v){
      ConstantInt* vi = ConstantInt::get(context_, APInt(64, v, false));
      
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
  
    Value* convertNum(Value* from, Value* to, bool trunc=true){
      return convertNum(from, to->getType(), trunc);
    }
    
    Value* convertNum(Value* from, Type* toType, bool trunc=true){
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

    Value* convert(Value* from, const nstr& toType, bool trunc=true){
      return convert(from, type(toType), trunc);
    }
    
    Value* convert(Value* from, Type* toType, bool trunc=true){
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
    
    ValueVec normalize(Value* v1, Value* v2, bool trunc=true){
      if(!v1 || !v2){
        return ValueVec();
      }
      
      Type* t1 = v1->getType();
      Type* t2 = v2->getType();
      
      Type* e1 = elementType(t1);
      Type* e2 = elementType(t2);
      
      Type* tn = 0;
      
      if(IntegerType* intType1 = dyn_cast<IntegerType>(e1)){
        size_t bits1 = intType1->getBitWidth();

        if(IntegerType* intType2 = dyn_cast<IntegerType>(e2)){
          size_t bits2 = intType2->getBitWidth();
          
          if(bits1 >= bits2){
            tn = t1;
          }
          else{
            tn = t2;
          }
        }
        else if(e2->isDoubleTy() || e2->isFloatTy()){
          tn = trunc ? t2 : 0;
        }
      }
      else if(e1->isDoubleTy()){
        if(IntegerType* intType2 = dyn_cast<IntegerType>(e2)){
          size_t bits2 = intType2->getBitWidth();
          
          tn = trunc ? t1 : 0;
        }
        else if(e2->isDoubleTy() || e2->isFloatTy()){
          tn = t1;
        }
      }
      else if(e1->isFloatTy()){
        if(IntegerType* intType2 = dyn_cast<IntegerType>(e2)){
          size_t bit2 = intType2->getBitWidth();
          
          tn = trunc ? t2 : 0;
        }
        else if(e2->isDoubleTy() || e2->isFloatTy()){
          tn = t2;
        }
      }
      
      if(!tn){
        return ValueVec();
      }
      
      return {convert(v1, tn), convert(v2, tn)};
    }
    
    Value* convert(Value* from, Value* to, bool trunc=true){
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
    
    size_t vectorLength(VectorType* vt){
      return vt->getNumElements();
    }
    
    size_t vectorLength(Type* t){
      VectorType* vt = dyn_cast<VectorType>(t);
      if(!vt){
        return 0;
      }
      
      return vectorLength(vt);
    }
    
    size_t vectorLength(Value* v){
      return vectorLength(v->getType());
    }
    
    Value* getNumeric(const nvar& x, Value* v=0){
      return getNumeric(x, v ? v->getType() : 0);
    }
    
    Value* getNumeric(const nvar& x, const nstr& t){
      return getNumeric(x, type(t));
    }
    
    Value* getNumeric(const nvar& x, Type* l=0){
      if(l){
        Type* t = elementType(l);
        if(IntegerType* it = dyn_cast<IntegerType>(t)){
          return ConstantInt::get(it, x.toLong());
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
        case nvar::Integer:{
          int64_t i = x.asLong();
          return nvar::intBytes(i) > 32 ? getInt64(i) : getInt32(i);
        }
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
          
          attributeMap_[s] = ap;
          
          return ap;
        }
        
        return 0;
      }
      
      return itr->second;
    }
   
    Value* createGEP(Value* ptr, Value* index){
      return builder_.CreateGEP(ptr, index, "gep");
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
    
    Value* createAlloca(Type* t, const nstr& name){
      nstr n = name + ".ptr";
      return builder_.CreateAlloca(t, 0, n.c_str());
    }
    
    Value* createAdd(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateAdd(v1, v2, "add.out");
      }

      return builder_.CreateFAdd(v1, v2, "fadd.out");
    }
    
    Value* createSub(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateSub(v1, v2, "sub.out");
      }

      return builder_.CreateFSub(v1, v2, "fsub.out");
    }
    
    Value* createMul(Value* v1, Value* v2){
      if(isIntegral(v1)){
        return builder_.CreateMul(v1, v2, "mul.out");
      }
      
      return builder_.CreateFMul(v1, v2, "fmul.out");
    }
    
    Value* createDiv(Value* v1, Value* v2){
      if(isIntegral(v1)){
        if(isUnsigned(v1) && isUnsigned(v2)){
          return builder_.CreateUDiv(v1, v2, "udiv.out");
        }
      
        return builder_.CreateSDiv(v1, v2, "sdiv.out");
      }

      return builder_.CreateFDiv(v1, v2, "fdiv.out");
    }
    
    Value* createRem(Value* v1, Value* v2){
      if(isIntegral(v1)){
        if(isUnsigned(v1) && isUnsigned(v2)){
          Value* ret = builder_.CreateURem(v1, v2, "urem.out");
          setUnsigned(ret);
          return ret;
        }
        
        return builder_.CreateSRem(v1, v2, "srem.out");
      }
      
      return builder_.CreateFRem(v1, v2, "frem.out");
    }
    
    Value* createShl(Value* v1, Value* v2){
      Value* ret = builder_.CreateShl(v1, v2, "shl.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createLShr(Value* v1, Value* v2){
      Value* ret = builder_.CreateLShr(v1, v2, "lshr.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createAnd(Value* v1, Value* v2){
      Value* ret = builder_.CreateAnd(v1, v2, "and.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createOr(Value* v1, Value* v2){
      Value* ret = builder_.CreateOr(v1, v2, "or.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
    }
    
    Value* createXor(Value* v1, Value* v2){
      Value* ret = builder_.CreateXor(v1, v2, "xor.out");
      if(isUnsigned(v1) && isUnsigned(v2)){
        setUnsigned(ret);
      }
      
      return ret;
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
    
    SymbolKey getSymbolKey(const nstr& s){
      SymbolKeyMap::const_iterator itr = _symbolMap.find(s);
      
      if(itr == _symbolMap.end()){
        return SKEY_NO_KEY;
      }

      return itr->second;
    }
    
    Value* compile(const nvar& n, Value* lhs=0){
      if(n.isNumeric()){
        return getNumeric(n, lhs);
      }
      else if(n.isSymbol()){
        SymbolKey key = getSymbolKey(n);
        
        switch(key){
          case SKEY_this:
            return this_;
          default:
            break;
        }
        
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
      else if(!n.isFunction()){
        error("invalid input", n);
        
        return 0;
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_Add_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createAdd(v[0], v[1]);
        }
        case FKEY_Sub_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createSub(v[0], v[1]);
        }
        case FKEY_Mul_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createMul(v[0], v[1]);
        }
        case FKEY_Div_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createDiv(v[0], v[1]);
        }
        case FKEY_Mod_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createRem(v[0], v[1]);
        }
        case FKEY_ShL_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createShl(v[0], v[1]);
        }
        case FKEY_ShR_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createLShr(v[0], v[1]);
        }
        case FKEY_BitAnd_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createAnd(v[0], v[1]);
        }
        case FKEY_BitOr_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createOr(v[0], v[1]);
        }
        case FKEY_BitXOr_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return createXor(v[0], v[1]);
        }
        case FKEY_Block_n:{
          if(n.empty()){
            return getInt32(0);
          }
          
          Value* rv;
          bool ok = true;
          for(size_t i = 0; i < n.size(); ++i){
            rv = compile(n[i]);
            if(!rv){
              ok = false;
            }
          }
          
          return ok ? rv : 0;
        }
        case FKEY_ScopedBlock_n:{
          if(n.empty()){
            return getInt32(0);
          }
         
          pushScope();
          
          Value* rv;
          bool ok = true;
          for(size_t i = 0; i < n.size(); ++i){
            rv = compile(n[i]);
            if(!rv){
              ok = false;
            }
          }
          
          popScope();
          
          return ok ? rv : 0;
        }
        case FKEY_Local_1:{
          Type* t = type(n[0]);
          
          const nstr& s = n[0];
          
          Value* v = createAlloca(t, s);

          putLocal(s, v);

          return v;
        }
        case FKEY_Local_2:{
          Type* t = type(n[0]);
          
          const nstr& s = n[0];
          
          Value* v = createAlloca(t, s);
          Value* r = compile(n[1], v);
          if(!r){
            return 0;
          }
          
          Value* rn = convert(r, v);
          if(!rn){
            error("invalid operands", n);
          }
          
          createStore(rn, v);
          
          putLocal(s, v);
          
          return v;
        }
        case FKEY_Set_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* rc = convert(r, l);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          createStore(rc, l);
          
          return getInt32(0);
        }
        case FKEY_AddBy_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* lv = createLoad(l);
          Value* rc = convert(r, lv);

          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          Value* o = createAdd(lv, rc);
          
          createStore(o, l);
          
          return l;
        }
        case FKEY_SubBy_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* lv = createLoad(l);
          Value* rc = convert(r, lv);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          Value* o = createSub(lv, rc);
          
          createStore(o, l);
          
          return l;
        }
        case FKEY_MulBy_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* lv = createLoad(l);
          Value* rc = convert(r, lv);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          Value* o = createMul(lv, rc);
          
          createStore(o, l);
          
          return l;
        }
        case FKEY_DivBy_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* lv = createLoad(l);
          Value* rc = convert(r, lv);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          Value* o = createDiv(lv, rc);
          
          createStore(o, l);
          
          return l;
        }
        case FKEY_ModBy_2:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = compile(n[1], l);
          if(!r){
            return 0;
          }
          
          Value* lv = createLoad(l);
          Value* rc = convert(r, lv);
          
          if(!rc){
            error("invalid operands", n);
            return 0;
          }
          
          Value* o = createRem(lv, rc);
          
          createStore(o, l);
          
          return l;
        }
        case FKEY_Not_1:{
          Value* r = compile(n[0]);
          
          if(!r){
            return 0;
          }
          
          ValueVec v = normalize(r, getInt8(0), false);
          if(v.empty()){
            error("invalid operand", n[0]);
            return 0;
          }
          
          Value* rc = builder_.CreateICmpNE(v[0], v[1], "cmp.out");
          
          return builder_.CreateSelect(rc, getInt1(0), getInt1(1), "not.out");
        }
        case FKEY_XOr_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return builder_.CreateXor(v[0], v[1], "xor.out");
        }
        case FKEY_And_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return builder_.CreateAnd(v[0], v[1], "and.out");
        }
        case FKEY_Or_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          return builder_.CreateOr(v[0], v[1], "or.out");
        }
        case FKEY_EQ_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            return builder_.CreateICmpEQ(v[0], v[1], "eq.out");
          }
          else{
            return builder_.CreateFCmpUEQ(v[0], v[1], "feq.out");
          }
        }
        case FKEY_NE_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            return builder_.CreateICmpNE(v[0], v[1], "ne.out");
          }
          else{
            return builder_.CreateFCmpUNE(v[0], v[1], "fne.out");
          }
        }
        case FKEY_LT_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            if(isUnsigned(v[0]) && isUnsigned(v[1])){
              return builder_.CreateICmpULT(v[0], v[1], "lt.out");
            }
            
            return builder_.CreateICmpSLT(v[0], v[1], "lt.out");
          }
          
          return builder_.CreateFCmpULT(v[0], v[1], "flt.out");
        }
        case FKEY_LE_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            if(isUnsigned(v[0]) && isUnsigned(v[1])){
              return builder_.CreateICmpULE(v[0], v[1], "le.out");
            }
            
            return builder_.CreateICmpSLE(v[0], v[1], "le.out");
          }
          
          return builder_.CreateFCmpULE(v[0], v[1], "fle.out");
        }
        case FKEY_GT_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            if(isUnsigned(v[0]) && isUnsigned(v[1])){
              return builder_.CreateICmpUGT(v[0], v[1], "gt.out");
            }
            
            return builder_.CreateICmpSGT(v[0], v[1], "gt.out");
          }

          return builder_.CreateFCmpUGT(v[0], v[1], "fgt.out");
        }
        case FKEY_GE_2:{
          Value* l = compile(n[0], lhs);
          Value* r = compile(n[1], l);
          
          ValueVec v = normalize(l, r);
          
          if(v.empty()){
            error("invalid operands", n);
            return 0;
          }
          
          if(isIntegral(v[0])){
            if(isUnsigned(v[0]) && isUnsigned(v[1])){
              return builder_.CreateICmpUGE(v[0], v[1], "ge.out");
            }
            
            return builder_.CreateICmpSGE(v[0], v[1], "ge.out");
          }
          
          return builder_.CreateFCmpUGE(v[0], v[1], "fge.out");
        }
        case FKEY_If_2:
        {
          Value* cv = compile(n[0]);
          if(!cv){
            return 0;
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "if.cond");
          
          BasicBlock* tb = BasicBlock::Create(context_, "then", func_);
          BasicBlock* mb = BasicBlock::Create(context_, "if.merge");
          
          builder_.CreateCondBr(cv, tb, mb);
          
          builder_.SetInsertPoint(tb);
          
          Value* tv = compile(n[1]);
          
          if(!tv){
            return 0;
          }
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          func_->getBasicBlockList().push_back(mb);
          
          builder_.SetInsertPoint(mb);
          
          return getInt32(0);
        }
        case FKEY_If_3:{
          Value* cv = compile(n[0]);
          
          if(!cv){
            return 0;
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "if.cond");
          
          BasicBlock* tb = BasicBlock::Create(context_, "then", func_);
          BasicBlock* eb = BasicBlock::Create(context_, "else");
          BasicBlock* mb = BasicBlock::Create(context_, "if.merge");
          
          builder_.CreateCondBr(cv, tb, eb);
          
          builder_.SetInsertPoint(tb);
          
          Value* tv = compile(n[1]);
          if(!tv){
            return 0;
          }
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          tb = builder_.GetInsertBlock();
          
          func_->getBasicBlockList().push_back(eb);
          builder_.SetInsertPoint(eb);
          
          Value* ev = compile(n[2]);
          if(!ev){
            return 0;
          }
          
          ev = convert(ev, tv);
          
          if(!builder_.GetInsertBlock()->getTerminator()){
            builder_.CreateBr(mb);
          }
          
          eb = builder_.GetInsertBlock();
          
          func_->getBasicBlockList().push_back(mb);
          
          builder_.SetInsertPoint(mb);
          
          return getInt32(0);
        }
        case FKEY_Select_3:{
          Value* cv = compile(n[0]);
          
          if(!cv){
            return 0;
          }
          
          Value* tv = compile(n[1]);
          if(!tv){
            return 0;
          }
          
          Value* fv = compile(n[2]);
          if(!fv){
            return 0;
          }
          
          return builder_.CreateSelect(cv, tv, fv, "select");
        }
        case FKEY_Switch_2:{
          cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
          cout << n << endl;
          cout << "---------------------------------------" << endl;
          
          Value* v = compile(n[0]);
          
          dump(v);
          
          v = convert(v, "long");
          
          dump(v);
          
          if(!v){
            error("invalid operand", n[0]);
            return 0;
          }
          
          loopMerge_ = BasicBlock::Create(context_, "switch.merge", func_);
          
          const nvar& d = n[1];
          
          BasicBlock* db = 0;
          BasicBlock* cb = builder_.GetInsertBlock();
          
          if(d.some()){
            db = BasicBlock::Create(context_, "case", func_);
            builder_.SetInsertPoint(db);
            
            if(!compile(d)){
              return 0;
            }
            
            builder_.CreateBr(loopMerge_);
          }

          builder_.SetInsertPoint(cb);
          
          const nmap& m = n;
          
          SwitchInst* s = builder_.CreateSwitch(v, db ? db : loopMerge_, m.size());
          
          for(auto& itr : m){
            const nvar& k = itr.first;

            if(k.isHidden()){
              continue;
            }
            
            if(!k.isInteger()){
              error("invalid case: " + k, n);
              return 0;
            }

            const nvar& c = itr.second;
            
            BasicBlock* b = BasicBlock::Create(context_, "case", func_);
            builder_.SetInsertPoint(b);
            Value* cv = compile(c);
            builder_.CreateBr(loopMerge_);
            
            s->addCase(getInt64(k), b);
          }

          builder_.SetInsertPoint(loopMerge_);
          
          return s;
        }
        case FKEY_While_2:{
          loopContinue_ = BasicBlock::Create(context_, "while.cond", func_);
          
          builder_.CreateBr(loopContinue_);
          
          builder_.SetInsertPoint(loopContinue_);
          
          Value* cv = compile(n[0]);
          if(!cv){
            return 0;
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "while.cmp");
          
          BasicBlock* lb = BasicBlock::Create(context_, "while.body");
          loopMerge_ = BasicBlock::Create(context_, "while.merge");
          
          func_->getBasicBlockList().push_back(lb);
          func_->getBasicBlockList().push_back(loopMerge_);
          
          builder_.CreateCondBr(cv, loopContinue_, loopMerge_);
          
          builder_.SetInsertPoint(lb);
          Value* lv = compile(n[1]);
          if(!lv){
            return 0;
          }
          
          builder_.CreateBr(loopContinue_);
          
          builder_.SetInsertPoint(loopMerge_);
          
          loopContinue_ = 0;
          loopMerge_ = 0;
          
          return getInt32(0);
        }
        case FKEY_For_4:{
          Value* iv = compile(n[0]);
          if(!iv){
            return 0;
          }
          
          BasicBlock* lb = BasicBlock::Create(context_, "for.body", func_);
          loopMerge_ = BasicBlock::Create(context_, "for.merge");
          
          builder_.CreateBr(lb);
          builder_.SetInsertPoint(lb);
          
          Value* bv = compile(n[3]);
          if(!bv){
            return 0;
          }
          
          Value* nv = compile(n[2]);
          if(!nv){
            return 0;
          }
          
          loopContinue_ = BasicBlock::Create(context_, "for.cond");
          func_->getBasicBlockList().push_back(loopContinue_);
          builder_.CreateBr(loopContinue_);
          builder_.SetInsertPoint(loopContinue_);
          
          Value* cv = compile(n[1]);
          if(!cv){
            return 0;
          }
          
          if(!isIntegral(cv)){
            cv = convert(cv, "int");
          }
          
          if(!cv){
            return error("not a boolean", n[0]);
          }
          
          cv = builder_.CreateICmpNE(cv, getInt1(0), "for.cmp");
          
          func_->getBasicBlockList().push_back(loopMerge_);
          
          builder_.CreateCondBr(cv, lb, loopMerge_);
          
          builder_.SetInsertPoint(loopMerge_);
          
          loopContinue_ = 0;
          loopMerge_ = 0;
          
          return getInt32(0);
        }
        case FKEY_Break_0:{
          if(!loopMerge_){
            return error("break not in the context of a loop", n);
          }
          
          builder_.CreateBr(loopMerge_);
          return getInt32(0);
        }
        case FKEY_Continue_0:{
          if(!loopContinue_){
            return error("continue not in the context of a loop", n);
          }
          
          builder_.CreateBr(loopContinue_);
          return getInt32(0);
        }
        case FKEY_Ret_0:{
          builder_.CreateRetVoid();
          
          foundReturn_ = true;
          
          return getInt32(0);
        }
        case FKEY_Ret_1:{
          if(!rt_){
            return error("attempt to return from a non-function", n);
          }
          else if(rt_->isVoidTy()){
            return error("attempt to return a value from a void function", n);
          }
          
          Value* r = compile(n[0]);
          
          if(!r){
            return 0;
          }
          
          r = convert(r, rt_);
          
          if(!r){
            return error("invalid conversion to return type", n);
          }
          
          builder_.CreateRet(r);
          
          foundReturn_ = true;
          
          return getInt32(0);
        }
        case FKEY_Inc_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = getNumeric(1, l);
          
          Value* lv = createLoad(l);
          Value* o = createAdd(lv, r);
          
          createStore(o, l);
          
          return o;
        }
        case FKEY_PostInc_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = getNumeric(1, l);
          
          Value* lv = createLoad(l);
          Value* o = createAdd(lv, r);
          
          createStore(o, l);
          
          return lv;
        }
        case FKEY_Dec_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = getNumeric(1, l);
          
          Value* lv = createLoad(l);
          Value* o = createSub(lv, r);
          
          createStore(o, l);
          
          return o;
        }
        case FKEY_PostDec_1:{
          Value* l = getLValue(n[0]);
          if(!l){
            return 0;
          }
          
          Value* r = getNumeric(1, l);
          
          Value* lv = createLoad(l);
          Value* o = createSub(lv, r);
          
          createStore(o, l);
          
          return lv;
        }
        case FKEY_Idx_2:{
          Value* v = compile(n[0]);
          
          if(vectorLength(v) == 0){
            return error("not a vector", n[0]);
          }
          
          Value* i = compile(n[1]);
          
          if(!i){
            return 0;
          }
          
          i = convert(i, "int");
          
          Value* vi = builder_.CreateExtractElement(v, i);
          
          if(isUnsigned(v)){
            setUnsigned(vi);
          }
          
          return vi;
        }
        case FKEY_Len_1:
        case FKEY_Size_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          Value* r = getUInt64(vectorLength(v));
          
          return r;
        }
        case FKEY_Neg_1:{
          Value* vn = compile(n[0]);
          if(!vn){
            return 0;
          }
          
          Value* zero = getInt32(0);
          ValueVec v = normalize(zero, vn);
          
          if(v.empty()){
            return error("type mismatch", n[0]);
          }
          
          return createSub(v[0], v[1]);
        }
        case FKEY_Inv_1:{
          Value* vn = compile(n[0]);
          if(!vn){
            return 0;
          }
          
          Value* one = getDouble(1);
          ValueVec v = normalize(one, vn);
          
          if(v.empty()){
            return error("type mismatch", n);
          }
          
          return createDiv(v[0], v[1]);
        }
        case FKEY_Vec_n:{
          size_t size = n.size();
          
          ValueVec v;
          for(size_t i = 0; i < size; ++i){
            v.push_back(getNumeric(n[i], lhs));
          }
                        
          VectorType* vt = VectorType::get(v[0]->getType(), size);
          
          Value* vr = createAlloca(vt, "vec");
          vr = createLoad(vr);
          
          for(size_t i = 0; i < size; ++i){
            vr = builder_.CreateInsertElement(vr, v[i], getInt32(i));
          }
          
          return vr;
        }
        case FKEY_Pow_2:{
          Value* l = compile(n[0]);
          if(!l){
            return 0;
          }
          
          l = convert(l, "double");
          if(!l){
            return error("type mismatch", n[0]);
          }
          
          Value* r = compile(n[1]);
          if(!r){
            return 0;
          }
          
          r = convert(r, "double");
          if(!r){
            return error("type mismatch", n[1]);
          }
          
          ValueVec args;
          args.push_back(l);
          args.push_back(r);
          
          return builder_.CreateCall(_globalFunc("llvm.pow.f64"),
                                     args.vector(), "pow");
        }
        case FKEY_Sqrt_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          v = convert(v, "double");
          if(!v){
            return error("type mismatch", n[0]);
          }
          
          ValueVec args;
          args.push_back(v);
          
          return builder_.CreateCall(_globalFunc("llvm.sqrt.f64"),
                                     args.vector(), "sqrt");
        }
        case FKEY_Exp_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          v = convert(v, "double");
          if(!v){
            return error("type mismatch", n[0]);
          }
          
          ValueVec args;
          args.push_back(v);
          
          return builder_.CreateCall(_globalFunc("llvm.exp.f64"),
                                     args.vector(), "exp");
        }
        case FKEY_Log_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          v = convert(v, "double");
          if(!v){
            return error("type mismatch", n[0]);
          }
          
          ValueVec args;
          args.push_back(v);
          
          return builder_.CreateCall(_globalFunc("llvm.log.f64"),
                                     args.vector(), "log");
        }
        case FKEY_Floor_1:
        {
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          return convert(v, "int");
        }
        case FKEY_Normalize_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          VectorType* vt = dyn_cast<VectorType>(v->getType());
          if(!vt){
            return v;
          }
          
          size_t length = vt->getNumElements();
          
          Type* et = elementType(vt);
          Value* d;
          if(et->isDoubleTy()){
            d = getDouble(0);
          }
          else if(et->isFloatTy()){
            d = getFloat(0);
          }
          else{
            return error("not a float/double vector", n[0]);
          }
          
          Value* vd = createAlloca(vt, "vec");
          vd = builder_.CreateLoad(vd);
          
          for(size_t i = 0; i < length; ++i){
            Value* e = builder_.CreateExtractElement(v, getInt32(i));
            e = builder_.CreateFMul(e, e);
            d = builder_.CreateFAdd(d, e);
          }
          
          ValueVec args;
          args.push_back(d);
          
          Value* sr;
          
          if(et->isDoubleTy()){
            sr = builder_.CreateCall(_globalFunc("llvm.sqrt.f64"),
                                     args.vector(), "sqrt");
          }
          else{
            sr = builder_.CreateCall(_globalFunc("llvm.sqrt.f32"),
                                     args.vector(), "sqrt");
          }
          
          for(size_t i = 0; i < length; ++i){
            vd = builder_.CreateInsertElement(vd, sr, getInt32(i));
          }
          
          return builder_.CreateFDiv(v, vd);
        }
        case FKEY_Magnitude_1:{
          Value* v = compile(n[0]);
          if(!v){
            return 0;
          }
          
          VectorType* vt = dyn_cast<VectorType>(v->getType());
          if(!vt){
            return v;
          }
          
          size_t length = vt->getNumElements();
          
          Type* et = elementType(vt);
          Value* d;
          if(et->isDoubleTy()){
            d = getDouble(0);
          }
          else if(et->isFloatTy()){
            d = getFloat(0);
          }
          else{
            return error("not a float/double vector", n[0]);
          }
          
          for(size_t i = 0; i < length; ++i){
            Value* e = builder_.CreateExtractElement(v, getInt32(i));
            e = builder_.CreateFMul(e, e);
            d = builder_.CreateFAdd(d, e);
          }
          
          ValueVec args;
          args.push_back(d);
          
          if(et->isDoubleTy()){
            return builder_.CreateCall(_globalFunc("llvm.sqrt.f64"),
                                       args.vector(), "sqrt");
          }

          return builder_.CreateCall(_globalFunc("llvm.sqrt.f32"),
                                     args.vector(), "sqrt");
        }
        case FKEY_DotProduct_2:{
          Value* lv = compile(n[0]);
          if(!lv){
            return 0;
          }
          
          size_t length = vectorLength(lv);
          if(length == 0){
            return error("expected a vector", n[0]);
          }
          
          Value* rv = compile(n[1]);
          if(!rv){
            return 0;
          }
          
          size_t length2 = vectorLength(rv);
          if(length == 0){
            return error("expected a vector", n[1]);
          }
          
          if(length != length2){
            return error("vector length mismatch", n);
          }
          
          ValueVec v = normalize(lv, rv);
          
          if(v.empty()){
            return error("type mismatch", n);
          }
          
          Value* r = getNumeric(0, elementType(v[0]));

          for(size_t i = 0; i < length; ++i){
            Value* e1 = builder_.CreateExtractElement(v[i], getInt32(i));
            Value* e2 = builder_.CreateExtractElement(v[i], getInt32(i));
            r = createAdd(r, createMul(e1, e2));
          }
          
          return r;
        }
        case FKEY_CrossProduct_2:{
          Value* lv = compile(n[0]);
          if(!lv){
            return 0;
          }
          
          if(vectorLength(lv) != 3){
            return error("expected a vector of length 3", n[0]);
          }
          
          Value* rv = compile(n[1]);
          if(!rv){
            return 0;
          }
          
          if(vectorLength(rv) != 3){
            return error("expected a vector of length 3", n[1]);
          }
          
          ValueVec v = normalize(lv, rv);
          
          if(v.empty()){
            return error("type mismatch", n);
          }
          
          VectorType* vt = cast<VectorType>(v[0]->getType());
          Type* et = vt->getElementType();
          
          Value* vr = createLoad(createAlloca(vt, "vec"));

          Value* e1 = builder_.CreateExtractElement(v[0], getInt32(1));
          Value* e2 = builder_.CreateExtractElement(v[1], getInt32(2));
          Value* e3 = builder_.CreateExtractElement(v[0], getInt32(2));
          Value* e4 = builder_.CreateExtractElement(v[1], getInt32(1));
          
          Value* e = createSub(createMul(e1, e2), createMul(e3, e4));
          
          vr = builder_.CreateInsertElement(vr, e, getInt32(0));
          
          e1 = builder_.CreateExtractElement(v[0], getInt32(2));
          e2 = builder_.CreateExtractElement(v[1], getInt32(0));
          e3 = builder_.CreateExtractElement(v[0], getInt32(0));
          e4 = builder_.CreateExtractElement(v[1], getInt32(2));
          
          e = createSub(createMul(e1, e2), createMul(e3, e4));
          vr = builder_.CreateInsertElement(vr, e, getInt32(1));
          
          e1 = builder_.CreateExtractElement(v[0], getInt32(0));
          e2 = builder_.CreateExtractElement(v[1], getInt32(1));
          e3 = builder_.CreateExtractElement(v[0], getInt32(1));
          e4 = builder_.CreateExtractElement(v[1], getInt32(0));
          
          e = createSub(createMul(e1, e2), createMul(e3, e4));
          vr = builder_.CreateInsertElement(vr, e, getInt32(2));
          
          return vr;
        }
        case FKEY_Call_1:{
          const nvar& fs = n[0];
          
          if(!fs.isFunction()){
            return error("invalid call", fs);
          }
          
          Value* ret;
          
          size_t size = fs.size();
          nstr name = fs;
          
          auto itr = functionMap_.find({className_, name, size});
          if(itr == functionMap_.end()){
            itr = functionMap_.find({name, size});
          
            if(itr == functionMap_.end()){
              return error("unknown function", n[0]);
            }
            
            Function* f = itr->second;

            ValueVec args;
            
            for(size_t i = 0; i < size; ++i){
              const nvar& fi = fs[i];
              
              Value* vi = compile(fi);
              args.push_back(vi);
            }
            
            if(f->getReturnType()->isVoidTy()){
              builder_.CreateCall(f, args.vector());
              return getInt32(0);
            }
            
            return builder_.CreateCall(f, args.vector(), name.c_str());
          }
          else{
            Function* f = itr->second;

            Value* as = f->arg_begin();
            Value* ap = createAlloca(elementType(as->getType()), "args");
            Value* tp = createStructGEP(ap, 1, "this.ptr");
            createStore(this_, tp);
            
            for(size_t i = 0; i < size; ++i){
              const nvar& fi = fs[i];
              
              Value* vi = compile(fi);
              Value* vp = createStructGEP(ap, i + 2, fi.str() + ".ptr");
              vi = convert(vi, vp);
              if(!vi){
                error("invalid operand", fi);
              }
              
              createStore(vi, vp);
            }
            
            if(f->getReturnType()->isVoidTy()){
              builder_.CreateCall(f, {ap});
              return getInt32(0);
            }

            return builder_.CreateCall(f, {ap}, name.c_str());
          }
          
          return 0;
        }
        case FKEY_Ptr_1:{
          return getLValue(n[0]);
        }
        case FKEY_DePtr_1:{
          return compile(n[0]);
        }
        case FKEY_Float_1:{
          return getNumeric(n[0], "float");
        }
        case FKEY_Abs_1:{
          nvar f = nfunc("If") << (nfunc("LT") << n[0] << 0) <<
          (nfunc("Neg") << n[0]) << n[0];
          
          return compile(f);
        }
        case FKEY_Log10_1:{
          nvar f = nfunc("Div") << (nfunc("Log") << n[0]) <<
          (nfunc("Log") << 10.0);
          
          return compile(f);
        }
        case FKEY_Ceil_1:{
          nvar f = nfunc("Floor") << (nfunc("Add") << n[0] << 1.0);
          
          return compile(f);
        }
        case FKEY_BitComplement_1:{
          nvar f = nfunc("Sub") << (nfunc("Neg") << n[0]) << 1;

          return compile(f);
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
    
    bool compileTop(const nvar& code){
      foundError_ = false;
      
      code_ = &code;
      
      nvec cs;
      code.keys(cs);

      for(size_t i = 0; i < cs.size(); ++i){
        const nvar& ck = cs[i];
        const nvar& ci = code[ck];
        
        if(ck.size() == 2){
          Type* rt = type(ci[0]);
          
          TypeVec args;
          
          const nvar& fs = ci[1];
          
          for(const nvar& a : fs){
            args.push_back(type(a));
          }
          
          Function* e = createFunction(fs, rt, args);
          functionMap_[{fs.sym(), fs.size()}] = e;
        }
      }
      
      for(size_t i = 0; i < cs.size(); ++i){
        const nvar& ck = cs[i];
        const nvar& ci = code[ck];
        
        if(ck.size() == 2){
          continue;
        }
        
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
            functionMap_[{ck, mk[0], mk[1]}] = createPrototype(ck, mj);
          }
        }
        
        for(size_t j = 0; j < ms.size(); ++j){
          const nvar& mk = ms[j];
          const nvar& mj = ci[mk];

          if(mk.size() == 2){
            currentFunc_ = &mj;
            
            compileFunction(ck, mj);
          }
        }
      }
      
      // ndm - debug
      module_.dump();
      
      return foundError_;
    }
    
    Function* createPrototype(const nstr& className, const nvar& f){
      const nvar& fs = f[1];
      
      nstr n = className + "_" + fs.str() + "_" + fs.size();
      
      TypeVec args;
      args.push_back(type("void*"));
      args.push_back(pointerType(classStruct_));
      
      for(const nvar& a : fs){
        args.push_back(type(a));
      }
      
      nstr an = n + "_args";
      
      Type* argsStruct =
      StructType::create(context_, args.vector(), an.c_str());
      
      return createFunction(n, type(f[0]), {pointerType(argsStruct)});
    }
    
    Function* compileFunction(const nstr& className, const nvar& f){
      scopeStack_.clear();
      attributeMap_.clear();
      loopContinue_ = 0;
      loopMerge_ = 0;
      
      className_ = className;
      
      const nvar& fs = f[1];
      
      func_ = functionMap_[{className_, fs.str(), fs.size()}];

      rt_ = func_->getReturnType();
      
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
      
      Value* b = compile(f[2]);
      
      popScope();
      
      if(!b){
        func_->eraseFromParent();
        return 0;
      }
      
      if(rt_->isVoidTy()){
        builder_.CreateRetVoid();
      }
      else if(!foundReturn_){
        error("no return found with non-void return type", f);
        func_->eraseFromParent();
        return 0;
      }
      
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
    
    bool isUnsigned(Value* v){
      const nvar& info = getInfo(v);
      return !info.get("signed", true);
    }
    
    void setUnsigned(Value* v){
      nvar& info = getInfo(v);
      info("signed") = false;
    }
    
    bool isIntegral(Value* v){
      return isIntegral(v->getType());
    }
    
    bool isIntegral(Type* t){
      return dyn_cast<IntegerType>(elementType(t));
    }
    
  private:
    typedef NVector<LocalScope*> ScopeStack_;
    typedef NMap<Value*, nvar> InfoMap_;
    typedef NMap<nstr, Value*> AttributeMap_;
    typedef NMap<nstr, StructType*> StructMap_;
    
    LLVMContext& context_;
    Module& module_;
    IRBuilder<> builder_;

    ostream* estr_;
    FunctionMap& functionMap_;
    StructMap_ structMap_;
    ScopeStack_ scopeStack_;
    AttributeMap_ attributeMap_;
    InfoMap_ infoMap_;
    Value* args_;
    Type* rt_;
    Function* func_;
    const nvar* code_;
    const nvar* currentClass_;
    const nvar* currentFunc_;
    BasicBlock* loopContinue_;
    BasicBlock* loopMerge_;
    BasicBlock* entry_;
    BasicBlock* begin_;
    bool foundReturn_;
    bool foundError_;
    StructType* classStruct_;
    Value* this_;
    nstr className_;
  };
  
  Global::Global()
  : context_(getGlobalContext()),
  module_("global", context_){
    
    InitializeNativeTarget();
    
    _initFunctionMap();
    _initSymbolMap();
    
    NPLCompiler compiler(module_, functionMap_, cerr);
    
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
  
  Function* Global::getFunction(const nstr& f){
    auto itr = functionMap_.find(f);
    assert(itr != functionMap_.end());
    return itr->second;
  }
  
} // end namespace

namespace neu{
  
  class NPLModule_{
  public:
    NPLModule_(NPLModule* o)
    : o_(o),
    context_(getGlobalContext()),
    module_("module", context_),
    estr_(&cerr){
      
      initGlobal();
      
      engine_ = EngineBuilder(&module_).create();
    }
    
    ~NPLModule_(){
      
    }
    
    bool compile(const nvar& code){
      NPLCompiler compiler(module_, functionMap_, *estr_);
      
      return compiler.compileTop(code);
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
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
  private:
    typedef NMap<NPLFunc::FP, Function*> FunctionPtrMap_;
    
    NPLModule* o_;
    
    LLVMContext& context_;
    Module module_;
    ExecutionEngine* engine_;
    FunctionMap functionMap_;
    FunctionPtrMap_ functionPtrMap_;
    ostream* estr_;
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

void NPLModule::setErrorStream(ostream& estr){
  x_->setErrorStream(estr);
}
