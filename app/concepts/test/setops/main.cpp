#include <iostream>

#include "Length.h"
#include <neu/NCProgram.h>

using namespace std;
using namespace neu;

class Program : public NCProgram{
public:
  Program(int argc, char** argv)
    : NCProgram(argc, argv){
    
  }

  void setup(){
    // describe the inputs
    a.setConst(true);
    a.setVec(true);
    a.setSet(true);
    
    b.setConst(true);
    b.setVec(true);
    b.setSet(true);
    
    d.setConst(true);
    d.setVec(true);
    d.setSet(true);
    
    f.setConst(true);
    f.setVec(true);
    f.setSet(true);
    
    // describe the outputs
    g.setVec(true);
    g.setSet(true);

    // add the variables to the code generator
    addInput("a", &a);
    addInput("b", &b);
    addInput("d", &d);
    addInput("f", &f);
    addOutput("g", &g);

    // inputs
    a = {1,2,3};
    b = {4,5,6};
    f = {6};
    d = {1,2,5,6,7,8};
  }

  nvar evaluate(){
    if(!run()){
      return undef; 
    }

    const nvar& v = g.val();

    // evaluate the fitness
    double fitness = 0;
    for(size_t i = 0; i < v.size(); ++i){
      if(v[i] == 1 || v[i] == 2 || v[i] == 5){
        fitness += 10;
      }
      else{
        fitness -= 1;
      }
    }

    if(fitness == 30){
      return nvar::inf();
    }

    return fitness;
  }

private:
  Length a;
  Length b;
  Length d;
  Length f;
  Length g;
};

int main(int argc, char** argv){
  Program program(argc, argv);

  program.init();
  program.start();
  
  return 0;
}
