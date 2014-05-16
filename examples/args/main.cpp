// This brief example demonstrates configuration and option handling
// as well as additional features of NProgram, it will read from an
// optional configuration file named "args_conf.nml" looking first in
// the current working directory, then in $NEU_HOME/conf giving
// precedence first to passed command-line options, then key/values in
// the configuration file, then to option defaults specified with
// NProgram::opt().

#include <iostream>
#include <sstream>

#include <neu/nvar.h>
#include <neu/NProgram.h>

using namespace std;
using namespace neu;

// we are not required to create an NProgram subclass - we can use
// NProgram by default, but more complex programs usually need to define
// signal handlers and other functionality
class Program : public NProgram{
public:
  Program(int argc, char** argv) : NProgram(argc, argv) {}

  // UNIX signal handler
  void handleSignal(int signal, bool fatal){
    if(fatal){
      switch(signal){
      case SIG_INT:
        // silently exit;
        exit(0);
      default:
        cout << "got a fatal signal: " << signalName(signal) << endl;
        exit(1);        
      }
    }
    // we don't care about non-fatal signals
  }
};

int main(int argc, char** argv){
  // options must be specified before the constructor to the NProgram
  // is called - unless we call NProgram's empty constructor - then
  // later we call one of the NProgram::init() methods to process the
  // options.

  //  a non-required option, will be triggered if -help or -h is
  //  passed, defaults to false
  Program::opt("help", "h", false);

  // a non-required option phase with alias p typed to a float (i.e:
  // double) whose default value is 3.0
  Program::opt("phase", "p", 3.0);

  // a non-required, untyped option, unaliased option - will be
  // parsed to the appropriate type using NML
  Program::opt("level");

  // a non-required option include whose type will be a vector of
  // strings, because the last parameter multi was specified as true
  Program::opt("include", "I", "", "Include paths", false, true);

  // a required option with no default whose type is float (i.e: double)
  Program::opt("depth", "d", 0.0, "Depth", true);

  //  a boolean value which defaults to true, we can set it to false
  //  by passing -no-verbose on the command line, or including
  //  verbose: false in the config
  Program::opt("verbose", "v", true);

  // this is the point at which the options and configuration are
  // processed
  Program program(argc, argv);

  // obtain the processed options
  const nvar& args = program.args();

  cout << "args is: " << args << endl;

  if(args["help"]){
    // automatically generate a usage message from the specified
    // options above
    cout << program.usage("args [OPTIONS] <FILES>") << endl;
    program.exit(0);
  }

  // the remaining arguments after the options are added to the vector
  // portion of the args
  if(args.empty()){
    cerr << "No files were specified." << endl;
    cerr << program.usage("args [OPTIONS] <FILES>") << endl;
    program.exit(0);
  }

  if(args["verbose"]){
    cout << "the files are: " << args.vec() << endl;
    cout << "the phase is: " << args["phase"] << endl;
    cout << "the level is: " << args["level"] << endl;
    cout << "includes are: " << args["include"] << endl;
    cout << "depth is: " << args["depth"] << endl;
  }

  // do something with the args

  return 0;
}
