#include <iostream>

#include <neu/nvar.h>
#include <neu/NProgram.h>
#include <neu/NSys.h>
#include <neu/NPLParser.h>
#include <neu/NPLModule.h>
#include <neu/NPQueue.h>

using namespace std;
using namespace neu;

class Cell : public NPLObject{
public:
  Cell(){
    
  }

  nvar x;
};

class CellFunc : public NPLFunc{
public:
  int y;
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

  if(!module.compile(code)){
    cerr << "failed to compile" << endl;
    NProgram::exit(1);
  }
  
  CellFunc* f = funcVec[0];
  
  module.getFunc({"Cell", "run", 1}, f);
  
  NPQueue queue;
  for(size_t i = 0; i < funcVec.size(); ++i){
    funcVec[i]->fp = f->fp;
    funcVec[i]->y = 10;
    funcVec[i]->o = cellVec[i];
    queue.add(funcVec[i]);
  }

  queue.run();

  for(size_t i = 0; i < cellVec.size(); ++i){
    cout << "x is: " << cellVec[i]->x << endl;
  }

  nvar x;

  return 0;
}
