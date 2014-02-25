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

#ifndef NEU_NPL_PARSER__H
#define NEU_NPL_PARSER__H

#include <unistd.h>

#include <neu/NPLParser.h>
#include <neu/NSys.h>
#include <neu/NProgram.h>

#include "parse.h"

using namespace std;
using namespace neu;

namespace neu{
  
  class NPLParser_{
  public:
    
    NPLParser_(NPLParser* o)
    : o_(o),
    estr_(&cerr),
    metadata_(false){
      
    }
    
    ~NPLParser_(){
      
    }
    
    void setMetadata(bool flag){
      metadata_ = flag;
    }
    
    void advance(const char* text, const nstr& tag=""){
      advance(strlen(text), tag);
    }
    
    void advance(size_t count, const nstr& tag=""){
      char_ += count;
      
      if(tags_ && !tag.empty()){
        nvar t;
        t("tag") = tag;
        t("start") = lastChar_;
        t("end") = char_;
        t("line") = line_;
        tags_->pushBack(move(t));
      }
      
      lastChar_ = char_;
    }
    
    nvar token(const char* text, const nstr& tag=""){
      char_ += strlen(text);
      
      nvar token = text;
      token("start") = lastChar_;
      token("end") = char_;
      token("line") = line_;
      
      if(!tag.empty()){
        if(tags_){
          tags_->pushBack(nvar());
          nvar& t = tags_->back();
          t("tag") = tag;
          t("start") = lastChar_;
          t("end") = char_;
          t("line") = line_;
        }
      }
      
      lastChar_ = char_;
      
      return token;
    }
    
    nvar parse(const nstr& code, nvar* tags){
      out_ = undef;
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = "";
      
      npl_lex_init(&scanner_);
      npl_set_extra(this, scanner_);
      
      nstr tempPath = NSys::tempFilePath();
      FILE* file = fopen(tempPath.c_str(), "w+");
      fwrite(code.c_str(), 1, code.length(), file);
      fclose(file);
      
      file = fopen(tempPath.c_str(), "r");
      npl_set_in(file, scanner_);
      
      npl_parse(this, scanner_);
      fclose(file);
      
      if(remove(tempPath.c_str()) != 0){
        NERROR("failed to delete to delete temp file: " + tempPath);
      }
      
      npl_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    nvar parseFile(const nstr& path, nvar* tags){
      out_ = undef;
      
      tags_ = tags;
      line_ = 1;
      char_ = 0;
      status_ = 0;
      file_ = NSys::basename(path);
      
      npl_lex_init(&scanner_);
      npl_set_extra(this, scanner_);
      
      FILE* file = fopen(path.c_str(), "r");
      
      if(!file){
        NERROR("failed to open: " + path);
      }
      
      npl_set_in(file, scanner_);
      npl_parse(this, scanner_);
      fclose(file);
      
      npl_lex_destroy(scanner_);
      
      if(status_ != 0){
        return none;
      }
      
      return out_;
    }
    
    nvar newClass(){
      nvar c;
      c("__offset") = 0;
      
      return c;
    }
    
    void addClass(const nstr& c, nvar& cv){
      if(out_.hasKey(c)){
        error("class exists: " + c);
        return;
      }
      
      out_(c) = move(cv);
    }
    
    void addMethod(nvar& c, nvar& f){
      nvar k = {f[1].sym(), f[1].size()};
      
      if(c.hasKey(k)){
        error("method exists: " + k);
        return;
      }
      
      c(k) = move(f);
    }
    
    void addAttribute(nvar& c, nvar& a){
      const nstr& k = a.back();
      
      if(c.hasKey(k)){
        error("attribute exists: " + k);
        return;
      }
      
      const nvar& t = getType(a);
      
      nvar av;
      av("offset") = c["__offset"];
      av("type") = nvar(a.str().lowercase(), nvar::Sym);

      c["__offset"] += t["size"];
      
      c(k) = move(av);
    }
    
    void define(const nstr& sym, const nvar& expr){
      defineMap_(sym) = expr;
    }
    
    void setErrorStream(ostream& estr){
      estr_ = &estr;
    }
    
    void emit(const nvar& n){
      if(n.some()){
        out_ << n;
      }
    }
    
    nvar error(const nvar& n, const nstr& message, bool warn=false){
      status_ = 1;
      
      (*estr_) << "NPLParser: " << file_ << ":" << line_ << ": " <<
      message << endl;
      
      return nsym("Error");
    }
    
    void error(const nstr& type){
      error(none, type);
    }
    
    nvar func(const nstr& f){
      nvar v = nfunc(f);
      v("_line") = line_;
      
      if(!file_.empty()){
        v("_file") = file_;
      }
      
      return v;
    }
    
    nvar func(const char* f){
      nvar v = nfunc(f);
      v("_line") = line_;
      
      if(!file_.empty()){
        v("_file") = file_;
      }
      
      return v;
    }
    
    nvar func(const nvar& v){
      return func(v.str());
    }
    
    nvar sym(const nstr& s){
      nvar v;
      
      if(defineMap_.hasKey(s)){
        v = defineMap_[s];
      }
      else{
        v = nsym(s);
      }

      v("_line") = line_;
      
      if(!file_.empty()){
        v("_file") = file_;
      }
      
      return v;
    }
    
    nvar sym(const nvar& v){
      return sym(v.str());
    }
    
    nvar sym(const char* s){
      nvar v;
      
      if(defineMap_.hasKey(s)){
        v = defineMap_[s];
      }
      else{
        v = nsym(s);
      }
      
      v("_line") = line_;
      
      if(!file_.empty()){
        v("_file") = file_;
      }
      
      return v;
    }
    
    nvar var(const nvar& v){
      return v;
    }
    
    void newLine(size_t count){
      line_ += count;
    }
    
    void newLine(){
      ++line_;
      advance(1);
    }
    
    size_t line() const{
      return line_;
    }
    
    void handleGet(const nvar& head, nvar& rest, nvar& out){
      out = head;
      
      for(size_t i = 0; i < rest.size(); ++i){
        nvar& h = rest[i];
        h.pushFront(out);
        out = move(h);
      }
    }
    
    const nvar& getType(const nstr& t);
    
  private:
    NPLParser* o_;
    ostream* estr_;
    nstr file_;
    size_t line_;
    size_t char_;
    size_t lastChar_;
    int status_;
    nvar* tags_;
    void* scanner_;
    nvar out_;
    bool metadata_;
    nvar defineMap_;
  };
  
} // end namespace neu

#endif // NEU_NPL_PARSER__H
