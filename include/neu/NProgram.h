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

#ifndef NEU_N_PROGRAM_H
#define NEU_N_PROGRAM_H

#include <neu/nvar.h>

#include <ostream>

namespace neu{
  
  class NProgram{
  public:
    NProgram(int& argc, char** argv, const nvar& args=undef);
    
    NProgram(const nvar& args=undef);
    
    virtual ~NProgram();
    
    virtual void onExit();
    
    static void exit(int status);
    
    static NProgram* instance();
    
    virtual void onFatalSignal();
    
    virtual void onSigHup();
    
    virtual void onSigInt();
    
    virtual void onSigQuit();
    
    virtual void onSigIll();
    
    virtual void onSigTrap();
    
    virtual void onSigAbrt();
    
    virtual void onSigFpe();
    
    virtual void onSigBus();
    
    virtual void onSigSegv();
    
    virtual void onSigSys();
    
    virtual void onSigPipe();
    
    virtual void onSigAlrm();
    
    virtual void onSigTerm();
    
    virtual void onSigUrg();
    
    virtual void onSigCont();
    
    virtual void onSigChld();
    
    virtual void onSigIo();
    
    virtual void onSigXCPU();
    
    virtual void onSigXFSz();
    
    virtual void onSigVtAlrm();
    
    virtual void onSigProf();
    
    virtual void onSigWInch();
    
    virtual void onSigUsr1();
    
    virtual void onSigUsr2();
    
    static void resetSignalHandlers();
    
    static void opt(const nstr& name,
                    const nstr& alias="",
                    const nvar& def=none,
                    bool required=false,
                    bool multi=false,
                    const nstr& description="");

    static void parseArgs(int argc, char** argv, nvar& args);
    
    static const nvar& args();
    
    static nstr usage(const nstr& msg);
    
    static int argc;
    
    static char** argv;

    NProgram& operator=(const NProgram&) = delete;
    NProgram(const NProgram&) = delete;

  private:
    class NProgram_* x_;
  };
  
} // end namespace neu

#endif // NEU_N_PROGRAM_H
