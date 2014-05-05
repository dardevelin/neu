#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram::opt("foo", "f", 0);
  
  NProgram::opt("bar", "b", false);

  NProgram::opt("include", "I", "", "includes", false, true);

  NProgram::opt("flag", "", true);

  NProgram program(argc, argv);

  const nvar& args = program.args();

  cout << "args is: " << args << endl;

  return 0;
}
