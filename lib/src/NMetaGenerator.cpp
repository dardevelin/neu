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

#include <neu/NMetaGenerator.h>

#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace std;
using namespace llvm;
using namespace clang;
using namespace tooling;
using namespace neu;

namespace{
  
  typedef vector<string> StringVec;
  typedef vector<CompileCommand> CompileCommandVec;
  
  class Consumer : public ASTConsumer{
  public:
    Consumer(CompilerInstance* ci)
    : ci_(ci){
      
    }
    
    void HandleTranslationUnit(ASTContext& context);
    
  private:
    NMetaGenerator_* visitor_;
    CompilerInstance* ci_;
  };
  
  class Action : public ASTFrontendAction {
  public:
    ASTConsumer* CreateASTConsumer(CompilerInstance& compilerInstance,
                                   StringRef file){
      
      return new Consumer(&compilerInstance);
    }
  };
  
  class Database : public CompilationDatabase{
  public:
    CompileCommandVec getCompileCommands(StringRef path) const{
      CompileCommandVec cv;
      
      CompileCommand c;
      c.Directory = ".";
      c.CommandLine =
      {"clang-tool", "-I/Users/nickm/testinclude", path.str()};
      
      cv.push_back(c);
      
      return cv;
    }
    
    CompileCommandVec getAllCompileCommands() const{
      return CompileCommandVec();
    }
    
    StringVec getAllFiles() const{
      return StringVec();
    }
  };
  
} // end namespace

namespace neu{
  
  class NMetaGenerator_ : public RecursiveASTVisitor<NMetaGenerator_>{
  public:
    NMetaGenerator_(NMetaGenerator* o, ostream& ostr)
    : o_(o),
    ostr_(ostr){
      
    }
    
    ~NMetaGenerator_(){
      
    }
    
    void setHandle(bool flag){
      
    }
    
    void setClass(bool flag){
      
    }
    
    void setMetadata(bool flag){
      
    }
    
    void addInclude(const nstr& path){
      
    }
    
    void addFile(const nstr& path){
      
    }
    
    void generate(){
      
    }
    
    void setCompilerInstance(CompilerInstance* ci){
      ci_ = ci;
    }
    
  private:
    NMetaGenerator* o_;
    ostream& ostr_;
    CompilerInstance* ci_;
  };
  
} // end namespace neu

void Consumer::HandleTranslationUnit(ASTContext& context){
  visitor_->setCompilerInstance(ci_);
  visitor_->TraverseDecl(context.getTranslationUnitDecl());
}

NMetaGenerator::NMetaGenerator(ostream& ostr){
  x_ = new NMetaGenerator_(this, ostr);
}

NMetaGenerator::~NMetaGenerator(){
  delete x_;
}

void NMetaGenerator::setHandle(bool flag){
  x_->setHandle(flag);
}

void NMetaGenerator::setClass(bool flag){
  x_->setClass(flag);
}

void NMetaGenerator::setMetadata(bool flag){
  x_->setMetadata(flag);
}

void NMetaGenerator::addInclude(const nstr& path){
  x_->addInclude(path);
}

void NMetaGenerator::addFile(const nstr& path){
  x_->addFile(path);
}

void NMetaGenerator::generate(){
  x_->generate();
}
