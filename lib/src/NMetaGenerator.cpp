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

#include <neu/nvar.h>
#include <neu/NSys.h>

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
    Database(const nvec& includes)
    : includes_(includes){
      
    }
    
    CompileCommandVec getCompileCommands(StringRef path) const{
      CompileCommandVec cv;
      
      CompileCommand c;
      c.Directory = ".";
      
      c.CommandLine = {"clang-tool", "-std=c++11"};

#ifdef __APPLE__
      c.CommandLine.push_back("-stdlib=libc++");

      c.CommandLine.push_back("-resource-dir");
      c.CommandLine.push_back("/Users/nickm/llvm-3.4/build-release/bin/../lib/clang/3.4");
      c.CommandLine.push_back("-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/usr/include");
#endif
      
      for(const nstr& i : includes_){
        //c.CommandLine.push_back("-I" + i);
      }
      
      c.CommandLine.push_back(path.str());
      
      cv.push_back(c);
      
      return cv;
    }
    
    CompileCommandVec getAllCompileCommands() const{
      return CompileCommandVec();
    }
    
    StringVec getAllFiles() const{
      return StringVec();
    }
    
  private:
    const nvec& includes_;
  };
  
} // end namespace

namespace neu{
  
  class NMetaGenerator_ : public RecursiveASTVisitor<NMetaGenerator_>{
  public:
    NMetaGenerator_(NMetaGenerator* o, ostream& ostr)
    : o_(o),
    ostr_(ostr),
    enableHandles_(true),
    enableClasses_(true),
    enableMetadata_(true){
    
      nstr h;
      if(!NSys::getEnv("NEU_HOME", h)){
        NERROR("NEU_HOME environment variable is undefined");
      }
      
      includes_.push_back(h + "/include");
    }
    
    ~NMetaGenerator_(){
      
    }
    
    void enableHandles(bool flag){
      enableHandles_ = flag;
    }
    
    void enableClasses(bool flag){
      enableClasses_ = flag;
    }
    
    void enableMetadata(bool flag){
      enableMetadata_ = flag;
    }
    
    void addInclude(const nstr& path){
      includes_.push_back(path);
    }
    
    void addFile(const nstr& path){
      files_.push_back(path);
    }
    
    void generate(){
      Database db(includes_);
      
      ClangTool tool(db, files_);
      
      int result =
      tool.run(newFrontendActionFactory<Action>());
    }
    
    bool VisitFunctionDecl(FunctionDecl* d){
      d->dump();
      return true;
    }
    
    void setCompilerInstance(CompilerInstance* ci){
      ci_ = ci;
    }
    
  private:
    NMetaGenerator* o_;
    ostream& ostr_;
    CompilerInstance* ci_;
    nvec includes_;
    StringVec files_;
    bool enableHandles_;
    bool enableClasses_;
    bool enableMetadata_;
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

void NMetaGenerator::enableHandles(bool flag){
  x_->enableHandles(flag);
}

void NMetaGenerator::enableClasses(bool flag){
  x_->enableClasses(flag);
}

void NMetaGenerator::enableMetadata(bool flag){
  x_->enableMetadata(flag);
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
