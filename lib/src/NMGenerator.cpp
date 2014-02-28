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

#include <neu/NMGenerator.h>

using namespace std;
using namespace neu;

#include <neu/NObject.h>

namespace{
  
  enum SymbolKey{
    
  };
  
  enum FunctionKey{
    FKEY_NO_KEY,
    FKEY_Add_2,
    FKEY_Sub_2,
    FKEY_Mul_2,
    FKEY_Div_2
  };
  
  typedef NMap<nstr, SymbolKey> SymbolMap;
  
  typedef NMap<pair<nstr, int>,
  pair<FunctionKey, NMGenerator::Type>> FunctionMap;
  
  static SymbolMap _symbolMap;
  
  static FunctionMap _functionMap;
  
  static void _initSymbolMap(){
    
  }
  
  static void _initFunctionMap(){
    _functionMap[{"Add", 2}] = {FKEY_Add_2, NMGenerator::Supported};
    _functionMap[{"Sub", 2}] = {FKEY_Sub_2, NMGenerator::Supported};
    _functionMap[{"Mul", 2}] = {FKEY_Mul_2, NMGenerator::Supported};
    _functionMap[{"Div", 2}] = {FKEY_Div_2, NMGenerator::Supported};
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
      FunctionMap::const_iterator itr =
      _functionMap.find(make_pair(f.str(), f.size()));
      
      if(itr == _functionMap.end()){
        itr = _functionMap.find(make_pair(f.str(), -1));
      }
      
      if(itr == _functionMap.end()){
        return FKEY_NO_KEY;
      }
      
      return itr->second.first;
    }
    
    void emitExpression(ostream& ostr,
                        const nstr& indent,
                        const nvar& v,
                        int prec=100){
      switch(v.type()){
        case nvar::Function:
          break;
        default:
          ostr << v;
          return;
      }
      
      FunctionKey key = getFunctionKey(v);
      
      switch(key){
        case FKEY_Add_2:{
          int p = NObject::precedence(v);
          
          if(p > prec){
            ostr << "(";
          }
          
          emitExpression(ostr, indent, v[0], p);
          ostr << " + ";
          emitExpression(ostr, indent, v[1], p);
          
          if(p > prec){
            ostr << ")";
          }
          break;
        }
        case FKEY_Sub_2:{
          int p = NObject::precedence(v);
          
          if(p > prec){
            ostr << "(";
          }
          
          emitExpression(ostr, indent, v[0], p);
          ostr << " - ";
          emitExpression(ostr, indent, v[1], p);
          
          if(p > prec){
            ostr << ")";
          }
          break;
        }
        case FKEY_Mul_2:{
          int p = NObject::precedence(v);
          
          if(p > prec){
            ostr << "(";
          }
          
          emitExpression(ostr, indent, v[0], p);
          ostr << " * ";
          emitExpression(ostr, indent, v[1], p);
          
          if(p > prec){
            ostr << ")";
          }
          break;
        }
        case FKEY_Div_2:{
          int p = NObject::precedence(v);
          
          if(p > prec){
            ostr << "(";
          }
          
          emitExpression(ostr, indent, v[0], p);
          ostr << " / ";
          emitExpression(ostr, indent, v[1], p);
          
          if(p > prec){
            ostr << ")";
          }
          break;
        }
        default:
          NERROR("[1] invalid function: " + v);
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
  auto itr = _functionMap.find(make_pair(v.str(), v.size()));
  
  if(itr == _functionMap.end()){
    itr = _functionMap.find(make_pair(v.str(), -1));
  }
  
  if(itr == _functionMap.end()){
    return 0;
  }
  
  return itr->second.second;
}
