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

#include <neu/NProgram.h>

#include <sstream>

#include <signal.h>

#include <neu/NRegex.h>
#include <neu/NReadGuard.h>
#include <neu/NWriteGuard.h>
#include <neu/NError.h>
#include <neu/NSys.h>
#include <neu/NThread.h>
#include <neu/NResourceManager.h>
#include <neu/global.h>
#include <neu/NMutex.h>

#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>

#ifndef META_NO_PRECISE

#include <gmp.h>
#include <mpfr.h>

#endif

using namespace std;
using namespace neu;

namespace{
  
  static NMutex _exitMutex;
  
  static void signal_handler(int signo){
    
    signal(signo, signal_handler);
    
    switch(signo){
      case SIGHUP:
      {
        _nprogram->onSigHup();
        break;
      }
      case SIGINT:
      {
        _nprogram->onSigInt();
        break;
      }
      case SIGQUIT:
      {
        _nprogram->onSigQuit();
        break;
      }
      case SIGILL:
      {
        _nprogram->onSigIll();
        break;
      }
      case SIGTRAP:
      {
        _nprogram->onSigTrap();
        break;
      }
      case SIGABRT:
      {
        _nprogram->onSigAbrt();
        break;
      }
      case SIGFPE:
      {
        _nprogram->onSigFpe();
        break;
      }
      case SIGBUS:
      {
        _nprogram->onSigBus();
        break;
      }
      case SIGSEGV:
      {
        _nprogram->onSigSegv();
        break;
      }
      case SIGSYS:
      {
        _nprogram->onSigSys();
        break;
      }
      case SIGPIPE:
      {
        _nprogram->onSigPipe();
        break;
      }
      case SIGALRM:
      {
        _nprogram->onSigAlrm();
        break;
      }
      case SIGTERM:
      {
        _nprogram->onSigTerm();
        break;
      }
      case SIGURG:
      {
        _nprogram->onSigUrg();
        break;
      }
      case SIGCONT:
      {
        _nprogram->onSigCont();
        break;
      }
      case SIGCHLD:
      {
        _nprogram->onSigChld();
        break;
      }
      case SIGIO:
      {
        _nprogram->onSigIo();
        break;
      }
      case SIGXCPU:
      {
        _nprogram->onSigXCPU();
        break;
      }
      case SIGXFSZ:
      {
        _nprogram->onSigXFSz();
        break;
      }
      case SIGVTALRM:
      {
        _nprogram->onSigVtAlrm();
        break;
      }
      case SIGPROF:
      {
        _nprogram->onSigProf();
        break;
      }
      case SIGWINCH:
      {
        _nprogram->onSigWInch();
        break;
      }
      case SIGUSR1:
      {
        _nprogram->onSigUsr1();
        break;
      }
      case SIGUSR2:
      {
        _nprogram->onSigUsr2();
        break;
      }
    }
  }
  
  class Opt{
  public:
    nstr name;
    nstr alias;
    nvar def;
    bool required;
    nstr description;
    bool multi;
  };
  
  typedef NMap<nstr, Opt*> OptMap;
  OptMap _optMap;
  nvar _args;
  
} // end namespace

namespace neu{
  
  using namespace std;
  
  class NProgram_{
  public:
    NProgram_(NProgram* program, const nvar& args)
    : o_(program){

      init_();
      
      _args = args;
    }
    
    NProgram_(NProgram* program, int& argc, char** argv, const nvar& args)
    : o_(program){
      
      init_();
      
      NProgram::parseArgs(argc, argv, _args);
      _args.merge(args);
      
      NProgram::argc = argc;
      NProgram::argv = argv;
    }
    
    ~NProgram_(){
      
    }
    
    void onExit(){
      
    }
    
    void init_(){
      if(_nprogram){
        NERROR("NProgram exists");
      }
      
      _nprogram = o_;
      
      NProgram::resetSignalHandlers();
      
#ifndef META_NO_PRECISE
      size_t precision = 256;
      
      if(precision < MPFR_PREC_MIN || precision > MPFR_PREC_MAX){
        NERROR("invalid mPrecision value");
      }
      
      mpfr_set_default_prec(precision);
#endif
      
      nstr h;
      if(!NSys::getEnv("NEU_HOME", h)){
        NERROR("NEU_HOME environment variable is undefined");
      }
    }
    
  private:
    NProgram* o_;
  };
  
} // end namespace neu

NProgram::NProgram(const nvar& args){
  x_ = new NProgram_(this, args);
}

NProgram::NProgram(int& argc, char** argv, const nvar& args){
  x_ = new NProgram_(this, argc, argv, args);
}

NProgram::~NProgram(){
  delete x_;
}

void NProgram::onExit(){
  return x_->onExit();
}

void NProgram::onFatalSignal(){
  NProgram::exit(1);
}

void NProgram::onSigHup(){
  cerr << "NProgram received SIGHUP" << endl;
  
  onFatalSignal();
}

void NProgram::onSigInt(){
  cerr << "NProgram received SIGINT" << endl;
  
  onFatalSignal();
}

void NProgram::onSigQuit(){
  cerr << "NProgram received SIGQUIT" << endl;
  
  onFatalSignal();
}

void NProgram::onSigIll(){
  cerr << "NProgram received SIGILL" << endl;
  
  onFatalSignal();
}

void NProgram::onSigTrap(){
  cerr << "NProgram received SIGTRAP" << endl;
  
  onFatalSignal();
}

void NProgram::onSigAbrt(){
  cerr << "NProgram received SIGABRT" << endl;
  
  onFatalSignal();
}

void NProgram::onSigFpe(){
  cerr << "NProgram received SIGFPE" << endl;
  
  onFatalSignal();
}

void NProgram::onSigBus(){
  cerr << "NProgram received SIGBUS" << endl;
  
  onFatalSignal();
}

void NProgram::onSigSegv(){
  cerr << "NProgram received SIGSEGV" << endl;
  
  onFatalSignal();
}

void NProgram::onSigSys(){
  cerr << "NProgram received SIGSYS" << endl;
  
  onFatalSignal();
}

void NProgram::onSigPipe(){
  cerr << "NProgram received SIGPIPE" << endl;
  
  onFatalSignal();
}

void NProgram::onSigAlrm(){
  cerr << "NProgram received SIGALRM" << endl;
  
  onFatalSignal();
}

void NProgram::onSigTerm(){
  cerr << "NProgram received SIGTERM" << endl;
  
  onFatalSignal();
}

void NProgram::onSigUrg(){
  cerr << "NProgram received SIGURG" << endl;
}

void NProgram::onSigCont(){
  cerr << "NProgram received SIGCONT" << endl;
}

void NProgram::onSigChld(){
  
}

void NProgram::onSigIo(){
  cerr << "NProgram received SIGIO" << endl;
}

void NProgram::onSigXCPU(){
  cerr << "NProgram received SIGXCPU" << endl;
  
  onFatalSignal();
}

void NProgram::onSigXFSz(){
  cerr << "NProgram received SIGXFSZ" << endl;
  
  onFatalSignal();
}

void NProgram::onSigVtAlrm(){
  cerr << "NProgram received SIGVTALRM" << endl;
  
  onFatalSignal();
}

void NProgram::onSigProf(){
  
}

void NProgram::onSigWInch(){
  cerr << "NProgram received SIGWINCH" << endl;
}

void NProgram::onSigUsr1(){
  cerr << "NProgram received SIGUSR1" << endl;
  
  onFatalSignal();
}

void NProgram::onSigUsr2(){
  cerr << "NProgram received SIGUSR2" << endl;
  
  onFatalSignal();
}

void NProgram::resetSignalHandlers(){
  signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGTRAP, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGFPE, signal_handler);
  signal(SIGBUS, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGSYS, signal_handler);
  signal(SIGPIPE, signal_handler);
  signal(SIGALRM, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGURG, signal_handler);
  signal(SIGCONT, signal_handler);
  signal(SIGCHLD, signal_handler);
  signal(SIGIO, signal_handler);
  signal(SIGXCPU, signal_handler);
  signal(SIGXFSZ, signal_handler);
  signal(SIGVTALRM, signal_handler);
  signal(SIGPROF, signal_handler);
  signal(SIGWINCH, signal_handler);
  signal(SIGUSR1, signal_handler);
  signal(SIGUSR2, signal_handler);
}

void NProgram::exit(int status){
  if(_exitMutex.tryLock()){
    if(_nprogram){
      _nprogram->onExit();
    }
    
    _resourceManager->release();
    std::exit(status);
  }
}

NProgram* NProgram::instance(){
  return _nprogram;
}

void NProgram::opt(const nstr& name,
                   const nstr& alias,
                   const nvar& def,
                   bool required,
                   bool multi,
                   const nstr& description){
  if(name.empty()){
    NERROR("empty option name");
  }
  
  if(_optMap.hasKey(name) || (!alias.empty() && _optMap.hasKey(alias))){
    NERROR("duplicate option name: " + name);
  }

  Opt* opt = new Opt;
  opt->name = name;
  opt->alias = alias;
  opt->def = def;
  opt->multi = multi;
  opt->required = required;
  opt->description = description;
  
  _optMap[name] = opt;

  if(!alias.empty()){
    _optMap[alias] = opt;
  }
}

void NProgram::parseArgs(int argc, char** argv, nvar& args){
  _args("bin") = argv[0];
  
  if(_optMap.empty()){
    nstr lastKey;

    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        lastKey = &arg[1];
      }
      else{
        if(lastKey.empty()){
          for(size_t j = i; j < argc; ++j){
            args.pushBack(argv[j]);
          }
          break;
        }
        else{
          args(lastKey) = nvar::fromStr(argv[i]);
        }
      }
    }
  }
  else{
    Opt* lastOpt = 0;
    
    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        nstr key = &arg[1];

        auto itr = _optMap.find(key);
        if(itr == _optMap.end()){
          NERROR("unknown option: " + key);
        }
        
        lastOpt = itr->second;
        const nvar& def = lastOpt->def;
        
        if(def.isBool()){
          nvar& k = args(lastOpt->name);
          if(lastOpt->multi){
            k << true;
          }
          else{
            k = true;
          }
        
          lastOpt = 0;
        }
      }
      else{
        if(lastOpt){
          
          const nvar& def = lastOpt->def;
          nvar v;
          switch(def.type()){
            case nvar::String:
              v = argv[i];
              break;
            case nvar::False:
            case nvar::True:
              v = nvar::fromStr(argv[i]);
              v = v.toBool();
              break;
            case nvar::Rational:
              v = nvar::fromStr(argv[i]);
              v = v.toRat();
              break;
            case nvar::Integer:
              v = nvar::fromStr(argv[i]);
              v = v.toLong();
              break;
            case nvar::Float:
              v = nvar::fromStr(argv[i]);
              v = v.toDouble();
              break;
            default:
              v = nvar::fromStr(argv[i]);
              break;
          }
          
          nvar& k = args(lastOpt->name);
          
          if(lastOpt->multi){
            k << move(v);
          }
          else{
            k = move(v);
          }
          
          lastOpt = 0;
        }
        else{
          for(size_t j = i; j < argc; ++j){
            args.pushBack(argv[j]);
          }
          break;
        }
      }
    }
    
    for(auto& itr : _optMap){
      Opt* o = itr.second;
      if(args.hasKey(o->name)){
        
      }
      else if(o->required){
        NERROR("missing option: " + o->name);
      }
      else{
        args(o->name) = o->def;
      }
    }
  }
}

const nvar& NProgram::args(){
  return _args;
}

nstr NProgram::usage(const nstr& msg){
  stringstream ostr;
  nstr idt = "       ";
  
  ostr << "SYNOPSIS" << endl;
  ostr << idt << msg << endl << endl;
  ostr << "DESCRIPTION" << endl;
  
  for(auto& itr : _optMap){
    Opt* opt = itr.second;
    
    ostr << idt << "-" << opt->name;
    
    const nstr& alias = opt->alias;
    
    if(!alias.empty()){
      ostr << ", -" << alias;
    }
    
    if(!opt->required){
      ostr << "=" << opt->def;
    }
    
    ostr << endl;
    
    ostr << idt << idt << opt->description;
    
    ostr << endl << endl;
  }
  
  return ostr.str();
}

int NProgram::argc = 0;
char** NProgram::argv = 0;
