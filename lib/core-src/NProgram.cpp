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
#include <neu/NMLParser.h>

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
        _program->onSigHup();
        break;
      }
      case SIGINT:
      {
        _program->onSigInt();
        break;
      }
      case SIGQUIT:
      {
        _program->onSigQuit();
        break;
      }
      case SIGILL:
      {
        _program->onSigIll();
        break;
      }
      case SIGTRAP:
      {
        _program->onSigTrap();
        break;
      }
      case SIGABRT:
      {
        _program->onSigAbrt();
        break;
      }
      case SIGFPE:
      {
        _program->onSigFpe();
        break;
      }
      case SIGBUS:
      {
        _program->onSigBus();
        break;
      }
      case SIGSEGV:
      {
        _program->onSigSegv();
        break;
      }
      case SIGSYS:
      {
        _program->onSigSys();
        break;
      }
      case SIGPIPE:
      {
        _program->onSigPipe();
        break;
      }
      case SIGALRM:
      {
        _program->onSigAlrm();
        break;
      }
      case SIGTERM:
      {
        _program->onSigTerm();
        break;
      }
      case SIGURG:
      {
        _program->onSigUrg();
        break;
      }
      case SIGCONT:
      {
        _program->onSigCont();
        break;
      }
      case SIGCHLD:
      {
        _program->onSigChld();
        break;
      }
      case SIGIO:
      {
        _program->onSigIo();
        break;
      }
      case SIGXCPU:
      {
        _program->onSigXCPU();
        break;
      }
      case SIGXFSZ:
      {
        _program->onSigXFSz();
        break;
      }
      case SIGVTALRM:
      {
        _program->onSigVtAlrm();
        break;
      }
      case SIGPROF:
      {
        _program->onSigProf();
        break;
      }
      case SIGWINCH:
      {
        _program->onSigWInch();
        break;
      }
      case SIGUSR1:
      {
        _program->onSigUsr1();
        break;
      }
      case SIGUSR2:
      {
        _program->onSigUsr2();
        break;
      }
    }
  }
  
  enum BuiltinKey{
    BKEY_improper,
    BKEY_packBlockSize,
    BKEY_tempPath,
    BKEY_timeout,
    BKEY_name,
    BKEY_home,
    BKEY_abort
  };
  
  typedef NMap<nstr, BuiltinKey> BuiltinMap;
  
  BuiltinMap _builtinMap;
  
  class Opt{
  public:
    nstr key;
    nstr alias;
    nvar def;
    bool required;
    nstr description;
    bool multi;
  };
  
  typedef NMap<nstr, Opt*> OptMap;
  OptMap _optMap;
  OptMap _builtinOptMap;
  nvar _args;
  
} // end namespace

namespace neu{
  
  using namespace std;
  
  class NProgram_{
  public:
    NProgram_(NProgram* program, const nvar& args)
    : o_(program){

      init();
      _args = args;
      
      setBuiltins();
      setDefaults();
    }
    
    NProgram_(NProgram* program, int& argc, char** argv, const nvar& args)
    : o_(program){

      NProgram::argc = argc;
      NProgram::argv = argv;

      init();
      NProgram::parseArgs(argc, argv, _args);
      merge(_args, args);
      
      setBuiltins();
      
      nstr p = _name + "_conf.nml";
      if(NSys::exists(p)){
        nvar c;
        parseConfig(p, c);
        merge(_args, c);
        setBuiltins();
      }
      else{
        p = _home + "/conf/" + _name + "_conf.nml";
        if(NSys::exists(p)){
          nvar c;
          parseConfig(p, c);
          merge(_args, c);
          setBuiltins();
        }
      }
      
      setDefaults();
    }
    
    ~NProgram_(){
      
    }
    
    void merge(nvar& args, const nvar& readArgs){
      if(!args.hasMap() || !readArgs.hasMap()){
        return;
      }
      
      args.merge(readArgs);
      
      nmap& m = args;
      const nmap& rm = readArgs;
      
      for(auto& itr : m){
        const nvar& k = itr.first;

        if(!k.isSymbol()){
          continue;
        }

        auto mitr = rm.find(k);
        if(mitr == rm.end()){
          continue;
        }
        
        const nvar& rv = mitr->second;
        
        nvar& v = itr.second;
        
        if(!v.hasSequence()){
          continue;
        }
        
        const nstr& ks = k;
        
        if(ks[0] == '_'){
          auto oitr = _builtinOptMap.find(ks);
          if(oitr == _builtinOptMap.end()){
            continue;
          }
          Opt* opt = oitr->second;
          if(opt->multi){
            if(rv.hasSequence()){
              v.append(rv);
            }
            else{
              v.pushBack(rv);
            }
          }
        }
        else{
          auto oitr = _optMap.find(ks);
          if(oitr == _optMap.end()){
            continue;
          }
          Opt* opt = oitr->second;
          if(opt->multi){
            if(rv.hasSequence()){
              v.append(rv);
            }
            else{
              v.pushBack(rv);
            }
          }
        }
      }
    }

    void parseConfig(const nstr& path, nvar& config){
      stringstream estr;
      NMLParser parser;
      parser.setErrorStream(estr);
      config = parser.parseFile(path);
      
      if(config == none){
        nstr err = estr.str();
        NERROR("error parsing config file: " + path + ": " + err);
      }
      
      if(!config.hasMap()){
        return;
      }
      
      nvec keys;
      config.keys(keys);
      for(const nvar& k : keys){
        if(!k.isSymbol()){
          continue;
        }
        
        const nstr& ks = k;
        if(ks[0] == '_'){
          auto itr = _builtinOptMap.find(ks);
          if(itr != _builtinOptMap.end()){
            Opt* opt = itr->second;
            if(opt->key != ks){
              nvar v = move(config[k]);
              config.erase(k);
              config(opt->key) = move(v);
            }
          }
        }
        else{
          auto itr = _optMap.find(ks);
          if(itr != _optMap.end()){
            Opt* opt = itr->second;
            if(opt->key != ks){
              nvar v = move(config[k]);
              config.erase(k);
              config(opt->key) = move(v);
            }
          }
        }
      }
    }
    
    void onExit(){
      
    }

    void builtinOpt(BuiltinKey key,
                    const nstr& name,
                    const nstr& alias,
                    const nvar& def,
                    const nstr& description="",
                    bool multi=false){
      
      nstr k = "_" + name;
      
      Opt* opt = new Opt;
      opt->key = k;
      opt->alias = alias;
      opt->def = def;
      opt->multi = multi;
      opt->required = false;
      opt->description = description;
      
      _builtinOptMap[name] = opt;
      
      if(!alias.empty()){
        _builtinOptMap[alias] = opt;
      }
      
      _builtinMap[k] = key;
    }

    void setBuiltins(){
      if(!_args.hasMap()){
        return;
      }
      
      nmap& m = _args;
      
      for(auto& itr : m){
        const nvar& k = itr.first;
        const nvar& v = itr.second;
        
        if(!k.isSymbol()){
          continue;
        }
        
        const nstr& ks = k.str();
        
        if(ks[0] != '_'){
          continue;
        }
        
        auto bitr = _builtinMap.find(ks);
        if(bitr == _builtinMap.end()){
          continue;
        }
        
        BuiltinKey key = bitr->second;
        switch(key){
          case BKEY_improper:
            _improper = v;
            break;
          case BKEY_packBlockSize:
            _packBlockSize = v;
            break;
          case BKEY_tempPath:
            _tempPath = v;
            break;
          case BKEY_timeout:
            _timeout = v;
            break;
          case BKEY_name:
            _name = v;
            break;
          case BKEY_home:
            _home = v;
            break;
          case BKEY_abort:
            _abort = v;
            break;
        }
      }
    }
    
    void setDefaults(){
      if(_tempPath.empty()){
        _tempPath = _home + "/scratch";
      }
      
      for(auto& itr : _optMap){
        Opt* o = itr.second;
        
        if(!_args.hasKey(o->key)){
          if(o->required){
            NERROR("missing option: " + o->key);
          }
          else if(o->multi){
            _args(o->key).touchVector();
          }
          else{
            _args(o->key) = o->def;
          }
        }
      }
    }
    
    void init(){
      if(_program){
        NERROR("NProgram exists");
      }
      
      _program = o_;
      
      NProgram::resetSignalHandlers();
      
#ifndef META_NO_PRECISE
      size_t precision = 256;
      
      if(precision < MPFR_PREC_MIN || precision > MPFR_PREC_MAX){
        NERROR("invalid mPrecision value");
      }
      
      mpfr_set_default_prec(precision);
#endif
      
      NSys::getEnv("NEU_HOME", _home);

      builtinOpt(BKEY_home, "home", "", _home);
      builtinOpt(BKEY_name, "name", "", _name);
      builtinOpt(BKEY_abort, "abort", "", _abort);
      builtinOpt(BKEY_improper, "improper", "", _improper);
      builtinOpt(BKEY_packBlockSize, "packBlockSize", "", _packBlockSize);
      builtinOpt(BKEY_tempPath, "tempPath", "", _tempPath);
      builtinOpt(BKEY_timeout, "timeout", "", _timeout);
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
    if(_program){
      _program->onExit();
    }
    
    _resourceManager->release();
    std::exit(status);
  }
}

NProgram* NProgram::instance(){
  return _program;
}

void NProgram::opt(const nstr& name,
                   const nstr& alias,
                   const nvar& def,
                   const nstr& description,
                   bool required,
                   bool multi){
  if(name.empty()){
    NERROR("empty option name");
  }
  
  if(_optMap.hasKey(name) || (!alias.empty() && _optMap.hasKey(alias))){
    NERROR("duplicate option name: " + name);
  }

  Opt* opt = new Opt;
  opt->key = name;
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
  _name = NSys::basename(argv[0]);
  
  if(_optMap.empty()){
    nstr lastKey;

    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        if(!lastKey.empty()){
          NERROR("expected a value at arg: " + nvar(i));
        }
        
        lastKey = &arg[1];

        if(lastKey.empty()){
          nstr text;
        
          for(size_t j = i; j < args; ++j){
            if(j > 0){
              text += " ";
            }
            text += argv[j];
          }
          
          args("text") = move(text);
          break;
        }
        else if(lastKey[0] == '_'){
          lastKey[0] = '_';
        }
      }
      else if(lastKey.empty()){
        for(size_t j = i; j < argc; ++j){
          args.pushBack(argv[j]);
        }
        break;
      }
      else{
        args(lastKey) = nvar::fromStr(argv[i]);
        lastKey = "";
      }
    }
  }
  else{
    Opt* lastOpt = 0;
    
    for(size_t i = 1; i < argc; ++i){
      char* arg = argv[i];
      
      if(arg[0] == '-'){
        if(lastOpt){
          NERROR("expected a value at arg: " + nvar(i));
        }
        
        nstr key = &arg[1];

        if(key.empty()){
          nstr text;
          
          for(size_t j = i; j < args; ++j){
            if(j > 0){
              text += " ";
            }
            text += argv[j];
          }
          
          args("text") = move(text);
          break;
        }
        else if(key[0] == '-'){
          auto itr = _builtinOptMap.find(&arg[2]);
          if(itr == _builtinOptMap.end()){
            key[0] = '_';
          }
          else{
            lastOpt = itr->second;
          }
        }
        
        if(!lastOpt){
          auto itr = _optMap.find(key);
          if(itr == _optMap.end()){
            NERROR("unknown option: " + key);
          }
          
          lastOpt = itr->second;
        }

        const nvar& def = lastOpt->def;
        
        if(def.isBool()){
          if(lastOpt->multi){
            args(lastOpt->key) << true;
          }
          else{
            args(lastOpt->key) = true;
          }
        
          lastOpt = 0;
        }
      }
      else if(lastOpt){
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
        
        if(lastOpt->multi){
          args(lastOpt->key) << move(v);
        }
        else{
          args(lastOpt->key) = move(v);
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
  
  NMap<Opt*, bool> m;
  
  for(auto& itr : _optMap){
    Opt* opt = itr.second;

    if(m.hasKey(opt)){
      continue;
    }
    
    m[opt] = true;
    
    ostr << idt << "-" << opt->key;
    
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
