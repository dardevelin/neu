#include <iostream>

#include <neu/NProgram.h>
#include <neu/NProc.h>
#include <neu/NSys.h>
#include <neu/NBroker.h>

#include "Foo.h"

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NProcTask task(8);
  Foo foo;

  NBroker broker(&task);
  broker.listen(5255);

  cout << "listening..." << endl;

  broker.distribute(&foo, "Foo", "foo");

  cout << "distributed..." << endl;

  NSys::sleep(1.0);

  nvar auth;
  auth("user") = "nickm";
  auth("pass") = "baz";

  Foo* fr = 
    static_cast<Foo*>(broker.obtain("localhost", 5255, "foo", auth));

  for(size_t i = 0; i < 1000; ++i){
    nvar x = fr->bar();
    cout << "x is: " << x << endl;
  }
  
  nvar y = fr->baz(1000);

  cout << "y is: " << y << endl;

  return 0;
}
