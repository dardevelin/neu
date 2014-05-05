#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NSys.h>
#include <neu/NPLParser.h>
#include <neu/NPLModule.h>
#include <neu/NPQueue.h>

using namespace std;
using namespace neu;

extern "C"
int squareMe(int a){
  return a*a;
}

class Cell : public NPLObject{
public:
  Cell(){
    b = 9;
    c = 29;
  }

  double b;
  double a;
  double c;
};

class CellFunc : public NPLFunc{
public:
  int ret;
  int x;
};

int main(int argc, char** argv){
  NProgram program(argc, argv);

  NPLParser parser;
  nvar code = parser.parseFile("kernel.npl");
  
  //cout << "code is: " << code << endl;
  
  typedef NVector<Cell*> CellVec;
  CellVec cellVec;
  
  typedef NVector<CellFunc*> FuncVec;
  FuncVec funcVec;
  
  for(size_t i = 0; i < 10; ++i){
    cellVec.push_back(new Cell);
    funcVec.push_back(new CellFunc);
  }

  NPLModule module;

  module.compile(code);
  
  CellFunc* f = funcVec[0];
  
  module.getFunc({"Cell", "run", 1}, f);
  
  NPQueue queue;
  for(size_t i = 0; i < funcVec.size(); ++i){
    funcVec[i]->fp = f->fp;
    funcVec[i]->x = i;
    funcVec[i]->o = cellVec[i];
    queue.add(funcVec[i]);
  }

  queue.run();

  for(size_t i = 0; i < funcVec.size(); ++i){
    cout << "ret is: " << funcVec[i]->ret << endl;
  }

  return 0;
}
