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

#include <iostream>

#include <neu/global.h>
#include <neu/NObject.h>
#include <neu/NMObject.h>
#include <neu/NProgram.h>
#include <neu/NMLParser.h>
#include <neu/NMLGenerator.h>

#include <editline/readline.h>

using namespace std;
using namespace neu;

class Program : public NProgram{
public:
  Program(int argc, char** argv)
  : NProgram(argc, argv){
    
  }
  
  void onSigInt(){
    cout << endl;
    NProgram::exit(0);
  }
};

int main(int argc, char** argv){
  Program program(argc, argv);

  program.argDefault("help", false,
                     "Display usage message");

  nvar args = program.args();
    
  if(args["help"]){
    cout << program.usage("neu [OPTION]... [NML FILE]...");
    program.exit(0);
  }

  NObject o;
  //NMObject o;
  NMLParser parser;
  NMLGenerator generator;

  for(size_t i = 0; i < args.size(); ++i){
    nvar n = parser.parseFile(args[i]);

    cout << "n is: " << n << endl;

    if(!n.some()){
      NProgram::exit(1);
    }

    nvar r = o.process(n);

    cout << "r is: " << r << endl;
  }

  stifle_history(100);

  for(;;){
    char* line = readline(">>> ");
    if(!line){
      break;
    }

    nstr code = line;
    code.strip();

    if(!code.empty()){
      add_history(line);
    }

    free(line);

    code += ";\n";

    nvar v = parser.parse(code);

    cout << "<<< " << v << endl;

    //try{
      nvar r = o.process(v);

      if(r.some()){
        generator.generate(cout, r);
        cout << endl;
      }
      //}
      //catch(NError& e){
      //cerr << e.msg() << endl;
      //}
  }
  
  cout << endl;

  NProgram::exit(0);

  return 0;
}
