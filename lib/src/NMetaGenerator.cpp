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
#include "clang/Sema/Sema.h"
#include "clang/AST/Comment.h"

#include <neu/nvar.h>
#include <neu/NSys.h>
#include <neu/NRegex.h>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace tooling;
using namespace comments;
using namespace neu;

namespace{
  
  NRegex _pointerRegex("(?:const )?class .*::(\\w+) \\*$");
  
  class Global{
  public:
    Global(){
      nTypeMap_["void"] = true;
      nTypeMap_["bool"] = true;
      nTypeMap_["const bool"] = true;
      nTypeMap_["const bool &"] = true;
      nTypeMap_["_Bool"] = true;
      nTypeMap_["const _Bool"] = true;
      nTypeMap_["const _Bool &"] = true;
      nTypeMap_["unsigned char"] = true;
      nTypeMap_["const unsigned char"] = true;
      nTypeMap_["const unsigned char &"] = true;
      nTypeMap_["signed char"] = true;
      nTypeMap_["const signed char"] = true;
      nTypeMap_["const signed char &"] = true;
      nTypeMap_["char"] = true;
      nTypeMap_["const char"] = true;
      nTypeMap_["const char &"] = true;
      nTypeMap_["short"] = true;
      nTypeMap_["const short"] = true;
      nTypeMap_["const short &"] = true;
      nTypeMap_["unsigned short"] = true;
      nTypeMap_["const unsigned short"] = true;
      nTypeMap_["const unsigned short &"] = true;
      nTypeMap_["int"] = true;
      nTypeMap_["const int"] = true;
      nTypeMap_["const int &"] = true;
      nTypeMap_["unsigned int"] = true;
      nTypeMap_["const unsigned int"] = true;
      nTypeMap_["const unsigned int &"] = true;
      nTypeMap_["long"] = true;
      nTypeMap_["const long"] = true;
      nTypeMap_["const long &"] = true;
      nTypeMap_["long long"] = true;
      nTypeMap_["const long long"] = true;
      nTypeMap_["const long long &"] = true;
      nTypeMap_["unsigned long"] = true;
      nTypeMap_["const unsigned long"] = true;
      nTypeMap_["const unsigned long &"] = true;
      nTypeMap_["unsigned long long"] = true;
      nTypeMap_["const unsigned long long"] = true;
      nTypeMap_["const unsigned long long &"] = true;
      nTypeMap_["float"] = true;
      nTypeMap_["const float"] = true;
      nTypeMap_["const float &"] = true;
      nTypeMap_["double"] = true;
      nTypeMap_["const double"] = true;
      nTypeMap_["const double &"] = true;
      nTypeMap_["class neu::nvar"] = true;
      nTypeMap_["class neu::nvar *"] = true;
      nTypeMap_["const class neu::nvar"] = true;
      nTypeMap_["const class neu::nvar &"] = true;
      nTypeMap_["class neu::nvar &"] = true;
      nTypeMap_["class neu::nstr"] = true;
      nTypeMap_["const class neu::nstr"] = true;
      nTypeMap_["const class neu::nstr &"] = true;
      nTypeMap_["class neu::nstr &"] = true;
      nTypeMap_["class std::basic_string<char>"] = true;
      nTypeMap_["const class std::basic_string<char>"] = true;
      nTypeMap_["const class std::basic_string<char> &"] = true;
      
      nTypeMap_["class neu::NVector<class neu::nvar, "
                "class std::allocator<class neu::nvar> >"] = true;
      nTypeMap_["class neu::NVector<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> >"] = true;
      
      nTypeMap_["const class neu::NVector<class neu::nvar, "
                "class std::allocator<class neu::nvar> >"] = true;
      nTypeMap_["const class neu::NVector<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> >"] = true;
      
      nTypeMap_["const class neu::NVector<class neu::nvar, "
                "class std::allocator<class neu::nvar> > &"] = true;
      nTypeMap_["const class neu::NVector<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> > &"] = true;
      
      nTypeMap_["class neu::NList<class neu::nvar, "
                "class std::allocator<class neu::nvar> >"] = true;
      nTypeMap_["class neu::NList<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> >"] = true;
      
      
      nTypeMap_["const class neu::NList<class neu::nvar, "
                "class std::allocator<class neu::nvar> >"] = true;
      nTypeMap_["const class neu::NList<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> >"] = true;
      
      nTypeMap_["const class neu::NList<class neu::nvar, "
                "class std::allocator<class neu::nvar> > &"] = true;
      nTypeMap_["const class neu::NList<class neu::nvar, "
                "class std::__1::allocator<class neu::nvar> > &"] = true;
      
      nTypeMap_["class neu::nrat"] = true;
      nTypeMap_["const class neu::nrat"] = true;
      nTypeMap_["const class neu::nrat &"] = true;
      nTypeMap_["class neu::nreal"] = true;
      nTypeMap_["const class neu::nreal"] = true;
      nTypeMap_["const class neu::nreal &"] = true;
      nTypeMap_["class neu::nstr *"] = true;
    }
    
    bool isNType(const nstr& t){
      return nTypeMap_.hasKey(t);
    }
    
  private:
    typedef NMap<nstr, bool> NTypeMap_;
    
    NTypeMap_ nTypeMap_;
  };
  
  // ndm - create on constructor below
  Global* _global = new Global;
  
} // end namespace

namespace neu{
  
  class NMetaGenerator_ : public RecursiveASTVisitor<NMetaGenerator_>{
  public:
    typedef vector<string> StringVec;
    typedef vector<CompileCommand> CompileCommandVec;
    
    class Consumer : public ASTConsumer{
    public:
      Consumer(CompilerInstance* ci, NMetaGenerator_* visitor)
      : ci_(ci),
      visitor_(visitor){
        
      }
      
      void HandleTranslationUnit(ASTContext& context){
        visitor_->init(ci_);
        visitor_->TraverseDecl(context.getTranslationUnitDecl());
      }
      
    private:
      NMetaGenerator_* visitor_;
      CompilerInstance* ci_;
    };
    
    class Action : public ASTFrontendAction {
    public:
      Action(NMetaGenerator_* visitor)
      : visitor_(visitor){
        
      }
      
      ASTConsumer* CreateASTConsumer(CompilerInstance& compilerInstance,
                                     StringRef file){
        
        return new Consumer(&compilerInstance, visitor_);
      }
      
    private:
      NMetaGenerator_* visitor_;
    };
    
    class Database : public CompilationDatabase{
    public:
      Database(const nvec& includes)
      : includes_(includes){
        
      }
      
      CompileCommandVec getCompileCommands(StringRef path) const{
        nstr p = path.str();
        
        CompileCommandVec cv;
        
        CompileCommand c;
        c.Directory = ".";
        
        c.CommandLine = {"clang-tool", "-std=c++11"};
        
#ifdef __APPLE__
        c.CommandLine.push_back("-stdlib=libc++");
        
        c.CommandLine.push_back("-resource-dir");
        c.CommandLine.push_back("/Users/nickm/llvm-3.4/build-release/bin/../"
                                "lib/clang/3.4");
        
        c.CommandLine.push_back("-I/Applications/Xcode.app/Contents/Developer/"
                                "Platforms/MacOSX.platform/Developer/SDKs/"
                                "MacOSX10.9.sdk/usr/include");
#endif
        
        for(const nstr& i : includes_){
          c.CommandLine.push_back("-I" + i);
        }
        
        if(p.endsWith(".h")){
          c.CommandLine.push_back("-x");
          c.CommandLine.push_back("c++-header");
        }
        
        c.CommandLine.push_back(p);
        
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
    
    class Factory : public FrontendActionFactory{
    public:
      Factory(NMetaGenerator_* visitor)
      : visitor_(visitor){
        
      }
      
      Action* create(){
        return new Action(visitor_);
      }
      
    private:
      NMetaGenerator_* visitor_;
    };
    
    NMetaGenerator_(NMetaGenerator* o)
    : o_(o),
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
    
    void generate(ostream& ostr){
      ostr << "#include <neu/nvar.h>" << endl;
      
      if(enableHandles_){
        ostr << "#include <neu/NFuncMap.h>" << endl;
      }
      
      if(enableClasses_){
        ostr << "#include <neu/NClass.h>" << endl;
      }

      ostr << endl;
      
      for(const nstr& f : files_){
        ostr << "#include \"" << f << "\"" << endl;
      }
      
      ostr << endl;
      
      ostr_ = &ostr;
      
      Database db(includes_);
      
      ClangTool tool(db, files_);
      
      Factory factory(this);
      int result = tool.run(&factory);
      
      //cout << "result is: " << result << endl;
    }
    
    void init(){
      
    }
    
    CXXRecordDecl* getSuperClass(CXXRecordDecl* rd, const string& className){
      if(!rd->hasDefinition()){
        return 0;
      }
      
      for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
          bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
        CXXBaseSpecifier b = *bitr;
        QualType qt = b.getType();
        
        QualType ct = sema_->Context.getCanonicalType(qt);
        
        if(ct.getAsString() == "class " + className){
          return rd;
        }
        
        const Type* t = ct.getTypePtr();
        if(const RecordType* rt = dyn_cast<RecordType>(t)){
          if(CXXRecordDecl* srd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
            CXXRecordDecl* s = getSuperClass(srd, className);
            if(s){
              return s;
            }
          }
        }
      }
      return 0;
    }
    
    CXXRecordDecl* getFirstSuperClass(CXXRecordDecl* rd,
                                      const string& className){
      if(!rd->hasDefinition()){
        return 0;
      }
      
      for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
          bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
        CXXBaseSpecifier b = *bitr;
        QualType qt = b.getType();
        
        QualType ct = sema_->Context.getCanonicalType(qt);
        
        const Type* t = ct.getTypePtr();
        if(const RecordType* rt = dyn_cast<RecordType>(t)){
          if(CXXRecordDecl* srd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
            if(getSuperClass(srd, className)){
              return srd;
            }
          }
        }
      }
      
      return 0;
    }
    
    bool VisitCXXRecordDecl(CXXRecordDecl* d){
      if(sema_->SourceMgr.isInMainFile(d->getLocStart())){
        CXXRecordDecl* s = getSuperClass(d, "neu::NObject");
        if(s){
          generateHandler(s, d);
          generateClass(d);
        }
      }

      return true;
    }
    
    void init(CompilerInstance* ci){
      ci_ = ci;
      sema_ = &ci_->getSema();
    }
    
    bool isBaseType(QualType qt, const string& baseType){
      QualType bct = sema_->Context.getCanonicalType(qt);
      
      if(const RecordType* rt = dyn_cast<RecordType>(bct)){
        if(CXXRecordDecl* rd = dyn_cast<CXXRecordDecl>(rt->getDecl())){
          if(!rd->hasDefinition()){
            return false;
          }
          
          for(CXXRecordDecl::base_class_iterator bitr = rd->bases_begin(),
              bitrEnd = rd->bases_end(); bitr != bitrEnd; ++bitr){
            CXXBaseSpecifier b = *bitr;
            
            QualType ct = sema_->Context.getCanonicalType(b.getType());
            
            if(ct.getAsString() == "class " + baseType){
              return true;
            }
            
            if(isBaseType(ct, baseType)){
              return true;
            }
          }
        }
      }
      return false;
    }
    
    int isNCompatibleType(QualType qt){
      QualType ct = sema_->Context.getCanonicalType(qt);
      
      if(_global->isNType(ct.getAsString())){
        return 1;
      }
      
      const Type* t = qt.getTypePtr();
      
      if(t->isPointerType()){
        if(isBaseType(t->getPointeeType(), "neu::NObjectBase")){
          return 2;
        }
      }
      else if(isBaseType(qt, "neu::NObjectBase")){
        return 3;
      }
      
      return 0;
    }
    
    int isNCallable(FunctionDecl* fd){
      if(!isNCompatibleType(fd->getResultType())){
        return 0;
      }
      
      int t = 1;
      for(CXXMethodDecl::param_iterator itr = fd->param_begin(),
          itrEnd = fd->param_end(); itr != itrEnd; ++itr){
        int ti = isNCompatibleType((*itr)->getType());
        if(!ti){
          return 0;
        }
        
        if(ti == 2){
          if(t < 2){
            t = 2;
          }
        }
        else if(ti == 3){
          t = 3;
        }
      }
      
      return t;
    }
    
    void generateHandler(CXXRecordDecl* srd,
                         CXXRecordDecl* rd){
      
      stringstream ostr;
      
      nstr className = rd->getName().str();
      nstr fullClassName = rd->getQualifiedNameAsString();
      
      ostr << "namespace{" << endl << endl;
      ostr << "class " << className << "_FuncMap : public neu::NFuncMap{" << endl;
      ostr << "public:" << endl;
      ostr << "  " << className << "_FuncMap(){" << endl;
      
      typedef NVector<CXXMethodDecl*> MethodVec;
      MethodVec mv;
      
      for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
        CXXMethodDecl* md = *mitr;
        if(md->isUserProvided() &&
           !md->isOverloadedOperator() &&
           md->getAccess() == AS_public &&
           !(isa<CXXConstructorDecl>(md) || isa<CXXDestructorDecl>(md))){
          
          nstr methodName = md->getName().str();
          
          if(methodName.empty() || methodName == "handle"){
            continue;
          }
          
          int m = isNCallable(md);
          if(m > 0 && m < 3){
            mv.push_back(md);
          }
        }
      }
      
      if(mv.empty()){
        return;
      }
      
      for(size_t i = 0; i < mv.size(); ++i){
        if(i > 0){
          ostr << endl;
        }
        CXXMethodDecl* md = mv[i];
        
        bool isVoid = md->getResultType().getTypePtr()->isVoidType();
        
        for(size_t j = md->getMinRequiredArguments(); j <= md->param_size(); ++j){
          ostr << "    add(\"" << md->getName().str() << "\", " << j <<
          ", " << endl;
          ostr << "      [](void* o, const neu::nvar& n) -> neu::nvar{" << endl;
          ostr << "        ";
          if(!isVoid){
            ostr << "return";
          }
          ostr << " static_cast<" << fullClassName << "*>(o)->";
          ostr << md->getName().str() << "(";
          
          for(size_t k = 0; k < j; ++k){
            if(k > 0){
              ostr << ", ";
            }
            
            ParmVarDecl* p = md->getParamDecl(k);
            
            const Type* t = p->getType().getTypePtr();
            
            if(t->isPointerType() &&
               isBaseType(t->getPointeeType(), "neu::NObjectBase")){
              nstr cn = t->getPointeeType().getAsString();
              
              nstr c = cn.substr(0, 6);
              
              assert(c == "class ");
              nstr name = cn.substr(6);
              
              name.findReplace("<anonymous>::", "");
              
              ostr << "static_cast<" << name << "*>(n[" << k << "].obj())";
            }
            else{
              ostr << "n[" << k << "]";
            }
          }
          ostr << ");" << endl;
          
          if(isVoid){
            ostr << "    return 0;" << endl;
          }
          ostr << "    });" << endl;
        }
      }
      
      ostr << "  }" << endl;
      
      ostr << "};" << endl << endl;
      
      ostr << "" << className << "_FuncMap _" << className <<
      "_FuncMap;" << endl << endl;
      
      ostr << "} // end namespace" << endl << endl;
      
      ostr << "neu::NFunc " << fullClassName << "::handle(const neu::nvar& n, uint32_t flags){" << endl;

      if(srd == rd){
        ostr << "  return _" << className <<
        "_FuncMap.map(n) ? : neu::NObject::handle(n);" << endl;
      }
      else{
        ostr << "  return _" << className << "_FuncMap.map(n) ? : " << endl;
        ostr << getQualifiedName(getFirstSuperClass(rd, "neu::NObject"));
        ostr << "::handle(v, flags);";
      }
      
      ostr << "}" << endl;
      
      *ostr_ << ostr.str() << endl;
    }

    nstr getQualifiedName(NamedDecl* decl){
      PrintingPolicy p(sema_->LangOpts);
      nstr ret = decl->getQualifiedNameAsString(p);
      ret.findReplace("<anonymous namespace>::", "");
      
      return ret;
    }
    
    void generateClass(CXXRecordDecl* rd){
      nstr name = rd->getName().str();
      nstr fullName = rd->getQualifiedNameAsString();
      
      stringstream ostr;
      
      ostr << "namespace{" << endl << endl;
      
      ostr << "class " << name << "_Class : public neu::NClass{" << endl;
      ostr << "public: " << endl;
      
      ostr << "  " << name << "_Class() : NClass(\"" << fullName << "\"){";
      
      if(enableMetadata_){
        ostr << endl;
        ostr << "    setMetadata(metadata_());" << endl;
        ostr << "  }";
      }
      else{
        ostr << "}";
      }
      
      ostr << endl << endl;
      
      ostr << "  neu::NObjectBase* construct(const neu::nvar& f){" << endl;
      
      for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
        
        CXXMethodDecl* md = *mitr;
        
        if(md->isUserProvided() && isa<CXXConstructorDecl>(md)){
          int m = isNCallable(md);
          
          if(m != 1 && m != 2){
            continue;
          }
          
          for(size_t k = md->getMinRequiredArguments();
              k <= md->param_size(); ++k){
            
            ostr << "    if(f.size() == " << k << "){" << endl;
            ostr << "      return new " << fullName << "(";
            
            for(size_t i = 0; i < k; ++i){
              if(i > 0){
                ostr << ", ";
              }
              
              ParmVarDecl* p = md->getParamDecl(i);
              
              const Type* t = p->getType().getTypePtr();
              
              // ndm - remove the reference type check?
              if((t->isPointerType() || t->isReferenceType()) &&
                 isBaseType(t->getPointeeType(), "neu::NObjectBase")){
                
                nstr cn = t->getPointeeType().getAsString();
                
                nstr c = cn.substr(0, 6);
                
                assert(c == "class ");
                nstr name = cn.substr(6);
                
                if(!t->isPointerType()){
                  ostr << "*";
                }
                
                ostr << "static_cast<" << fullName << "*>(f[" << i << "].obj())";
              }
              else{
                ostr << "f[" << i << "]";
              }
            }
            ostr << ");" << endl;
            ostr << "    }" << endl;
          }
        }
      }
      ostr << "    return 0;" << endl;
      ostr << "  }" << endl << endl;

      if(enableMetadata_){
        generateMetadata(ostr, rd);
      }
      
      ostr << "};" << endl << endl;
      
      ostr << name << "_Class _" << name << "Class();" << endl << endl;

      ostr << "} // end namespace" << endl << endl;

      *ostr_ << ostr.str();
    }
    
    void commentToStr(Comment* c, nstr& str){
      for(Comment::child_iterator itr = c->child_begin(),
          itrEnd = c->child_end(); itr != itrEnd; ++itr){
        Comment* ci = *itr;
        if(TextComment* tc = dyn_cast<TextComment>(ci)){
          // ndm - fix w/o c_str() - problem in nstr +=
          nstr tci = tc->getText().str();
          tci.strip();
          str += tci;
        }
        else{
          commentToStr(ci, str);
        }
      }
    }
    
    nstr getDeclComment(Decl* d){
      // ndm - make attribute for AST context
      FullComment* comment =
      ci_->getASTContext().getCommentForDecl(d, &ci_->getPreprocessor());
      
      if(comment){
        nstr cstr;
        commentToStr(comment, cstr);
        return cstr;
      }
      
      return "";
    }
    
    void generateMetadata(ostream& ostr, CXXRecordDecl* rd){
      nstr className = rd->getName().str();
      
      CXXRecordDecl* srd = getFirstSuperClass(rd, "neu::NObject");
      
      ostr << "  neu::nvar metadata_(){" << endl;
      
      ostr << "    neu::nvar ret;" << endl;
      
      ostr << "    neu::nvar& c = ret(\"" << className << "\");" << endl;
      
      nstr cs = getDeclComment(rd);
      if(!cs.empty()){
        cs.escapeForC();
        ostr << "    c(\"comment\") = \"" << cs << "\";" << endl;
      }
      
      if(srd){
        ostr << "    c(\"extends\") = \"" << srd->getName().str() << "\";" << endl;
      }
      else{
        ostr << "    c(\"extends\") = \"NObject\";" << endl;
      }

      ostr << "    neu::nvar& m = c(\"methods\");" << endl;
      
      for(CXXRecordDecl::method_iterator mitr = rd->method_begin(),
          mitrEnd = rd->method_end(); mitr != mitrEnd; ++mitr){
        CXXMethodDecl* md = *mitr;
        if(md->isUserProvided() &&
           md->getAccess() == AS_public &&
           !md->isOverloadedOperator()){
          
          nstr methodName = md->getNameAsString();
          
          if(methodName.empty()){
            continue;
          }
          
          int m = isNCallable(md);
          if(m > 0 && m < 3){
            for(size_t k = md->getMinRequiredArguments();
                k <= md->param_size(); ++k){
              ostr << "    {" << endl;
              
              nstr methodName = md->getNameAsString();
              
              ostr << "      neu::nvar& mi = m({\"" << methodName << "\", " <<
              k << "});" << endl;
              
              nstr cs = getDeclComment(md);
              
              if(!cs.empty()){
                cs.escapeForC();
                
                ostr << "      mi(\"comment\") = \"" << cs << "\";" << endl;
              }
              
              ostr << "      mi(\"const\") = ";
              
              if(md->getTypeQualifiers() & Qualifiers::Const){
                ostr << "true";
              }
              else{
                ostr << "false";
              }
              
              ostr << ";" << endl;
              
              if(!md->getResultType().getTypePtr()->isVoidType()){
                ostr << "      neu::nvar& ri = mi(\"return\");" << endl;
                
                nstr rawType = md->getResultType().getAsString();
                
                nvec m;
                if(_pointerRegex.match(rawType, m)){
                  ostr << "      ri(\"type\") = \"" << m[1].str() << "\";" << endl;
                }
                
                ostr << "      ri(\"const\") = ";
                
                if(rawType.find("const") == 0){
                  ostr << "true";
                }
                else{
                  ostr << "false";
                }
                
                ostr << ";" << endl;
              }
              
              for(size_t i = 0; i < k; ++i){
                ostr << "      {" << endl;
                
                ParmVarDecl* p = md->getParamDecl(i);
                nstr rawType = p->getType().getAsString();
                
                ostr << "        mi.pushBack(neu::nvar());" << endl;
                ostr << "        neu::nvar& pi = mi.back();" << endl;
                ostr << "        pi = \"" << p->getName().str() << "\";" << endl;
                
                nvec m;
                if(_pointerRegex.match(rawType, m)){
                  ostr << "        pi(\"type\") = \"" << m[1].str() << "\";" << endl;
                }
                
                ostr << "        pi(\"const\") = ";
                
                if(rawType.find("const") == 0){
                  ostr << "true";
                }
                else{
                  ostr << "false";
                }
                
                ostr << ";" << endl;
                
                ostr << "      }" << endl;
              }
              ostr << "    }" << endl;
            }
          }
        }
      }
    
      ostr << "    return ret;" << endl;
    
      ostr << "  }" << endl;
    }

  private:
    NMetaGenerator* o_;
    ostream* ostr_;
    CompilerInstance* ci_;
    Sema* sema_;
    nvec includes_;
    StringVec files_;
    bool enableHandles_;
    bool enableClasses_;
    bool enableMetadata_;
    
  };
  
} // end namespace neu

NMetaGenerator::NMetaGenerator(){
  x_ = new NMetaGenerator_(this);
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

void NMetaGenerator::generate(ostream& ostr){
  x_->generate(ostr);
}
