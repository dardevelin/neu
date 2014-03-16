#include <iostream>

#include <neu/NProgram.h>
#include <neu/NMetaGenerator.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  const nvar& args = program.args();

  NMetaGenerator gen;

  for(const nstr& ai : args){
    gen.addFile(ai);
  }
  
  gen.generate(cout);
  
  return 0;
}
