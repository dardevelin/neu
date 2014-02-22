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

  stifle_history(100);

  NObject o;
  //NMObject o;
  NMLParser parser;
  NMLGenerator generator;

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

    try{
      nvar r = o.process(v);

      if(r.some()){
        generator.generate(cout, r);
        cout << endl;
      }
    }
    catch(NError& e){
      cerr << e.msg() << endl;
    }
  }
  
  cout << endl;

  NProgram::exit(0);

  return 0;
}
