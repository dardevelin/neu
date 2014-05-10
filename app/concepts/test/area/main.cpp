#include <iostream>

#include <cmath>

#include <neu/NLib.h>
#include <neu/NCProgram.h>
#include <neu/NRandom.h>

#include "Length.h"
#include "Area.h"
#include "Angle.h"
#include "Count.h"


using namespace std;
using namespace neu;

class Program : public NCProgram{
public:
  Program(int argc, char** argv)
    : NCProgram(argc, argv){
    
  }

  void setup(){
    disableAll();

    enable("Real", "divCount", true);
    enable("Angle", "sin", true);
    enable("Real", "mulCount", true);
    enable("Real", "divRatio", true);
    enable("Length", "mulToArea", true);
    enable("Area", "mulLengthRatio", true);
    
    t.setConst(true);
    t2.setConst(true);

    c.setConst(true);
    c = 2;

    n.setConst(true);

    l.setConst(true);

    addInput("t", &t);
    addInput("t2", &t2);
    addInput("c", &c);
    addInput("n", &n);
    addInput("l", &l);
    addOutput("areaOut", &areaOut);
  }

  nvar evaluate(){
    NRandom rng;
    rng.timeSeed();

    try{
      double error = 0;

      for(size_t i = 0; i < 10; ++i){
        double length = rng.uniform(1, 100);

        l = length;

        for(size_t ni = 3; ni < 10; ++ni){
          n = ni;

          double theta = 2*M_PI/ni;
          t = theta;
          t2 = theta/2;

          double rv = length/(2*sin(theta/2));

          double da = 0.5*ni*rv*rv*sin(theta);

          if(!run()){
            return undef;
          }
 
          double ei = areaOut.val() - da;
          ei *= ei;
          error += ei;
        }
      }

      if(error < 0.0001){
        return nvar::inf();
      }

      return 1/error;
    }
    catch(NError& e){
      return undef;
    }
  }

private:
  Length l;
  Angle t;
  Angle t2;
  Count c;
  Count n;
  Area areaOut;
};

int main(int argc, char** argv){
  Program program(argc, argv);

  program.init();
  program.start();

  return 0;
}
