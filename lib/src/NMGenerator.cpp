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

#include <neu/NMGenerator.h>

#include <neu/NObject.h>

using namespace std;
using namespace neu;

namespace{
  
  enum SymbolKey{
    SKEY_NO_KEY,
    SKEY_Pi,
    SKEY_Eu,
    SKEY_Inf,
    SKEY_NegInf
  };
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2,
    FKEY_Pow_2,
    FKEY_Mod_2,
    FKEY_LT_2,
    FKEY_GT_2,
    FKEY_LE_2,
    FKEY_GE_2,
    FKEY_EQ_2,
    FKEY_NE_2,
    FKEY_And_2,
    FKEY_Or_2,
    FKEY_Not_1,
    FKEY_Neg_1,
    FKEY_Inc_1,
    FKEY_PostInc_1,
    FKEY_Dec_1,
    FKEY_PostDec_1,
    FKEY_AddBy_2,
    FKEY_SubBy_2,
    FKEY_MulBy_2,
    FKEY_DivBy_2,
    FKEY_ModBy_2,
    FKEY_Set_2,
    FKEY_Integrate_2
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  typedef NMap<pair<nstr, int>,
  pair<FunctionKey, NMGenerator::Type>> FunctionMap;
  
  static SymbolMap _symbolMap;
  
  static FunctionMap _functionMap;
  
  static void _initSymbolMap(){
    _symbolMap["Pi"] = SKEY_Pi;
    _symbolMap["Eu"] = SKEY_Eu;
    _symbolMap["Inf"] = SKEY_Inf;
    _symbolMap["NegInf"] = SKEY_NegInf;
  }
  
  static void _initFunctionMap(){
    auto S = NMGenerator::Supported;
    auto R = NMGenerator::Requested;
    
    _functionMap[{"Add", 2}] = {FKEY_Add_2, S};
    _functionMap[{"Sub", 2}] = {FKEY_Sub_2, S};
    _functionMap[{"Mul", 2}] = {FKEY_Mul_2, S};
    _functionMap[{"Div", 2}] = {FKEY_Div_2, S};
    _functionMap[{"Pow", 2}] = {FKEY_Pow_2, S};
    _functionMap[{"Mod", 2}] = {FKEY_Mod_2, S};
    _functionMap[{"Set", 2}] = {FKEY_Set_2, S};
    _functionMap[{"LT", 2}] = {FKEY_LT_2, S};
    _functionMap[{"GT", 2}] = {FKEY_GT_2, S};
    _functionMap[{"LE", 2}] = {FKEY_LE_2, S};
    _functionMap[{"GE", 2}] = {FKEY_GE_2, S};
    _functionMap[{"EQ", 2}] = {FKEY_EQ_2, S};
    _functionMap[{"NE", 2}] = {FKEY_NE_2, S};
    _functionMap[{"Not", 1}] = {FKEY_Not_1, S};
    _functionMap[{"Neg", 1}] = {FKEY_Neg_1, S};
    _functionMap[{"And", 2}] = {FKEY_And_2, S};
    _functionMap[{"Or", 2}] = {FKEY_Or_2, S};
    _functionMap[{"Inc", 1}] = {FKEY_Inc_1, S};
    _functionMap[{"PostInc", 1}] = {FKEY_PostInc_1, S};
    _functionMap[{"Dec", 1}] = {FKEY_Dec_1, S};
    _functionMap[{"PostDec", 1}] = {FKEY_PostDec_1, S};
    _functionMap[{"AddBy", 2}] = {FKEY_AddBy_2, S};
    _functionMap[{"SubBy", 2}] = {FKEY_SubBy_2, S};
    _functionMap[{"MulBy", 2}] = {FKEY_MulBy_2, S};
    _functionMap[{"DivBy", 2}] = {FKEY_DivBy_2, S};
    _functionMap[{"ModBy", 2}] = {FKEY_ModBy_2, S};
    _functionMap[{"Integrate", 2}] = {FKEY_Integrate_2, R};
  };
  
  class _FunctionMapLoader{
  public:
    _FunctionMapLoader(){
      _initSymbolMap();
      _initFunctionMap();
    }
  };
  
  static _FunctionMapLoader* _functionMapLoader = new _FunctionMapLoader;
  
} // end namespace

namespace neu{
  
  class NMGenerator_{
  public:
    NMGenerator_(NMGenerator* o, NMObject* obj)
    : o_(o),
    obj_(obj){
      
    }
    
    ~NMGenerator_(){
      
    }
    
    void generate(ostream& ostr, const nvar& v){
      emitExpression(ostr, "", v);
      ostr << " // FullForm\n";
    }
    
    FunctionKey getFunctionKey(const nvar& f){
      auto itr = _functionMap.find({f.str(), f.size()});
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find({f.str(), -1});
      }
      
      if(itr == _functionMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second.first;
    }
    
    SymbolKey getSymbolKey(const nvar& f){
      auto itr = _symbolMap.find(f);
      
      if(itr == _symbolMap.end()){
        return SKEY_NO_KEY;
      }
      
      return itr->second;
    }
    
    void emitFunc(ostream& ostr, const nvar& n, const nstr& func=""){
      if(func.empty()){
        ostr << n.str();
      }
      else{
        ostr << func;
      }
      
      ostr << "[";
      
      size_t size = n.size();
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ", ";
        }
        emitExpression(ostr, "", n[i]);
      }
      
      ostr << "]";
    }
    
    void emitBinOp(ostream& ostr,
                   const nvar& n,
                   const nstr& op,
                   int prec){
      
      int p = NObject::precedence(n);
      
      if(p > prec){
        ostr << "(";
      }
      
      emitExpression(ostr, "", n[0], p);
      ostr << op;
      emitExpression(ostr, "", n[1], p);
      
      if(p > prec){
        ostr << ")";
      }
    }
    
    void emitUnaryOp(ostream& ostr,
                     const nvar& n,
                     const nstr& op,
                     int prec){
      int p = NObject::precedence(n);
      
      ostr << op;
      
      if(p > prec){
        ostr << "(";
      }
      
      emitExpression(ostr, "", n[0], p);
      
      if(p > prec){
        ostr << ")";
      }
    }
    
    void emitPostUnaryOp(ostream& ostr,
                         const nvar& n,
                         const nstr& op,
                         int prec){
      int p = NObject::precedence(n);
      
      if(p > prec){
        ostr << "(";
      }
      
      emitExpression(ostr, "", n[0], p);
      
      if(p > prec){
        ostr << ")";
      }
      
      ostr << op;
    }
    
    bool emitSequence(ostream& ostr, const nvar& n){
      size_t size = n.size();

      if(size == 0){
        return false;
      }
      
      for(size_t i = 0; i < size; ++i){
        if(i > 0){
          ostr << ", ";
        }
        emitExpression(ostr, "", n[i]);
      }
      
      return true;
    }
    
    void emitMap(ostream& ostr, const nvar& n, bool first=true){
      const nmap& m = n;

      for(auto& itr : m){
        if(first){
          first = false;
        }
        else{
          ostr << ", ";
        }
      
        emitExpression(ostr, "", itr.first);
        ostr << "-> ";
        emitExpression(ostr, "", itr.second);
      }
    }

    void emitMultimap(ostream& ostr, const nvar& n, bool first=true){
      const nmmap& m = n;
      
      for(auto& itr : m){
        if(first){
          first = false;
        }
        else{
          ostr << ", ";
        }
        
        emitExpression(ostr, "", itr.first);
        ostr << "-> ";
        emitExpression(ostr, "", itr.second);
      }
    }
    
    void emitExpression(ostream& ostr,
                        const nstr& indent,
                        const nvar& n,
                        int prec=100){
      switch(n.type()){
        case nvar::False:
          ostr << "False";
          return;
        case nvar::True:
          ostr << "True";
          return;
        case nvar::Symbol:{
          SymbolKey key = getSymbolKey(n);
          switch(key){
            case SKEY_Pi:
              ostr << "Pi";
              return;
            case SKEY_Eu:
              ostr << "E";
              return;
            case SKEY_Inf:
              ostr << "Infinity";
              return;
            case SKEY_NegInf:
              ostr << "-Infinity";
              return;
            default:
              ostr << n;
              return;
          }
        }
        case nvar::Real:
          ostr << n.real().toStr(false);
          return;
        case nvar::Rational:
        case nvar::Integer:
        case nvar::String:
          ostr << n;
          return;
        case nvar::Float:{
          nstr s = n.toStr(false);
          s.findReplace("e", "`*^");
          ostr << s;
          return;
        }
        case nvar::List:
        case nvar::Vector:{
          ostr << "{";
          emitSequence(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::Map:{
          ostr << "{";
          emitMap(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::Multimap:{
          ostr << "{";
          emitMultimap(ostr, n);
          ostr << "}";
          return;
        }
        case nvar::SequenceMap:{
          ostr << "{";
          bool first = emitSequence(ostr, n);

          if(n.hasMap()){
            emitMap(ostr, n, first);
          }
          else{
            emitMultimap(ostr, n, first);
          }
          ostr << "}";
          return;
        }
        case nvar::Function:
          break;
        default:
          NERROR("invalid expression: " + n);
      }
      
      FunctionKey key = getFunctionKey(n);
      
      switch(key){
        case FKEY_NO_KEY:
          NERROR("invalid function: " + n);
        case FKEY_Add_2:
          emitBinOp(ostr, n, " + ", prec);
          break;
        case FKEY_Sub_2:
          emitBinOp(ostr, n, " - ", prec);
          break;
        case FKEY_Mul_2:
          emitBinOp(ostr, n, "*", prec);
          break;
        case FKEY_Div_2:
          emitBinOp(ostr, n, "/", prec);
          break;
        case FKEY_Mod_2:
          emitBinOp(ostr, n, "%", prec);
          break;
        case FKEY_Pow_2:
          emitBinOp(ostr, n, "^", prec);
          break;
        case FKEY_LT_2:
          emitBinOp(ostr, n, " < ", prec);
          break;
        case FKEY_GT_2:
          emitBinOp(ostr, n, " > ", prec);
          break;
        case FKEY_LE_2:
          emitBinOp(ostr, n, " <= ", prec);
          break;
        case FKEY_GE_2:
          emitBinOp(ostr, n, " >= ", prec);
          break;
        case FKEY_EQ_2:
          emitBinOp(ostr, n, " == ", prec);
          break;
        case FKEY_NE_2:
          emitBinOp(ostr, n, " != ", prec);
          break;
        case FKEY_And_2:
          emitBinOp(ostr, n, " && ", prec);
          break;
        case FKEY_Or_2:
          emitBinOp(ostr, n, " || ", prec);
          break;
        case FKEY_AddBy_2:
          emitBinOp(ostr, n, " += ", prec);
          break;
        case FKEY_SubBy_2:
          emitBinOp(ostr, n, " -= ", prec);
          break;
        case FKEY_MulBy_2:
          emitBinOp(ostr, n, " *= ", prec);
          break;
        case FKEY_DivBy_2:
          emitBinOp(ostr, n, " /= ", prec);
          break;
        case FKEY_ModBy_2:
          emitBinOp(ostr, n, " %= ", prec);
          break;
        case FKEY_Not_1:
          emitUnaryOp(ostr, n, "!", prec);
          break;
        case FKEY_Neg_1:
          emitUnaryOp(ostr, n, "-", prec);
          break;
        case FKEY_Inc_1:
          emitUnaryOp(ostr, n, "++", prec);
          break;
        case FKEY_PostInc_1:
          emitPostUnaryOp(ostr, n, "++", prec);
          break;
        case FKEY_Dec_1:
          emitUnaryOp(ostr, n, "--", prec);
          break;
        case FKEY_PostDec_1:
          emitPostUnaryOp(ostr, n, "--", prec);
          break;
        case FKEY_Integrate_2:
          emitFunc(ostr, n);
          break;
        default:
          NERROR("function not implemented: " + n);
      }
    }
    
  private:
    NMGenerator* o_;
    NMObject* obj_;
  };
  
} // end namespace neu

const NMGenerator::Type NMGenerator::Supported;
const NMGenerator::Type NMGenerator::Requested;

NMGenerator::NMGenerator(NMObject* obj){
  x_ = new NMGenerator_(this, obj);
}

NMGenerator::~NMGenerator(){
  delete x_;
}

void NMGenerator::generate(std::ostream& ostr, const nvar& v){
  x_->generate(ostr, v);
}

NMGenerator::Type NMGenerator::type(const nvar& v){
  auto itr = _functionMap.find({v.str(), v.size()});
  
  if(itr == _functionMap.end()){
    itr = _functionMap.find({v.str(), -1});
  }
  
  if(itr == _functionMap.end()){
    return 0;
  }
  
  return itr->second.second;
}
