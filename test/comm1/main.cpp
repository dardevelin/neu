#include <iostream>
#include <deque>
#include <list>

#include <neu/nvar.h>
#include <neu/NSocket.h>
#include <neu/NProgram.h>
#include <neu/NListener.h>
#include <neu/NSys.h>
#include <neu/NSocket.h>
#include <neu/NListener.h>
#include <neu/NProc.h>
#include <neu/NCommunicator.h>

using namespace std;
using namespace neu;

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NListener listener;
  NProcTask task(8);
  listener.listen(5255);
  NSocket* socket = listener.accept();
  NCommunicator c(&task);
  c.setSocket(socket);

  for(;;){
    nvar msg;
    c.receive(msg);

    cout << "msg is: " << msg << endl;

    ++msg["count"];
    c.send(msg);
  }

  NSys::sleep(100);

  return 0;
}
