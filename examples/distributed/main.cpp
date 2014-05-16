#include <iostream>

#include <neu/NProgram.h>
#include <neu/NProc.h>
#include <neu/NSys.h>
#include <neu/NBroker.h>

#include "Object.h"

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  // an NProcTask is required to support the required internal
  // concurrency
  NProcTask task(8);
  Object obj;

  // create a broker - in this case we will use the broker for both
  // server and client sides - this is not what would normally be done
  NBroker broker(&task);

  // begin listing on port 5255
  broker.listen(5255);

  cout << "listening..." << endl;

  // distribute the object, specifying its class name and object name
  broker.distribute(&obj, "Object", "obj");

  cout << "distributed..." << endl;

  NSys::sleep(1.0);

  // send any authentication and session data the server-side requires
  nvar auth;
  auth("user") = "nickm";
  auth("pass") = "baz";

  Object* d = 
    static_cast<Object*>(broker.obtain("localhost", 5255, "obj", auth));

  for(size_t i = 0; i < 1000; ++i){
    nvar x = d->bar();
    cout << "x is: " << x << endl;
  }
  
  nvar y = d->baz(1000);

  cout << "y is: " << y << endl;

  nvar count;
  d->getCount(count);
  
  cout << "count is: " << count << endl;

  return 0;
}
