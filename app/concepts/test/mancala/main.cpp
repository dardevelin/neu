#include <iostream>

#include <cmath>

#include <neu/NRandom.h>
#include <neu/NCProgram.h>

#include "Mancala.h"
#include "Count.h"
#include "Index.h"
#include "Truth.h"

using namespace std;
using namespace neu;

static const size_t MATCHES = 50;

class Program : public NCProgram{
public:
  Program(int argc, char** argv)
    : NCProgram(argc, argv){
    
  }

  void setup(){
    disableAll();

    enable("Mancala", true);

    mancala.setConst(true);

    slot.setConst(true);
    slot.setWeight(100);

    index0.setConst(true);
    index0.setStatic(true);
    index0 = 0;

    index1.setConst(true);
    index1.setStatic(true);
    index1 = 1;

    index2.setConst(true);
    index2.setStatic(true);
    index2 = 2;

    index3.setConst(true);
    index3.setStatic(true);
    index3 = 3;

    index4.setConst(true);
    index4.setStatic(true);
    index4 = 4;

    index5.setConst(true);
    index5.setStatic(true);
    index5 = 5;

    index6.setConst(true);
    index6.setStatic(true);
    index6 = 6;

    index7.setConst(true);
    index7.setStatic(true);
    index7 = 7;

    index8.setConst(true);
    index8.setStatic(true);
    index8 = 8;

    index9.setConst(true);
    index9.setStatic(true);
    index9 = 9;

    index10.setConst(true);
    index10.setStatic(true);
    index10 = 10;

    index11.setConst(true);
    index11.setStatic(true);
    index11 = 11;

    index12.setConst(true);
    index12.setStatic(true);
    index12 = 12;

    count0.setConst(true);
    count0.setStatic(true);
    count0 = 0;

    count1.setConst(true);
    count1.setStatic(true);
    count1 = 1;

    addInput("slot", &slot);
    addInput("index0", &index0);
    addInput("index1", &index1);
    addInput("index2", &index2);
    addInput("index3", &index3);
    addInput("index4", &index4);
    addInput("index5", &index5);
    addInput("index6", &index6);
    addInput("index7", &index7);
    addInput("index8", &index8);
    addInput("index9", &index9);
    addInput("index10", &index10);
    addInput("index11", &index11);
    addInput("index12", &index12);
    addInput("count0", &count0);
    addInput("count1", &count1);
    addInput("mancala", &mancala);
    addOutput("truthOut", &truthOut);
  }

  nvar evaluate(){
    NRandom random;
    random.timeSeed();

    try{
      size_t totalFitness = 0;

      for(size_t i = 0; i < MATCHES; ++i){
        mancala.reset();

        bool done = false;

        for(;;){
          double mt = nvar::negInf();
          size_t mj;

          for(size_t j = 1; j <= 6; ++j){
            if(mancala.numPieces(true, j) > 0){
              slot = j;
              if(!run()){
                return undef;
              }
              double t = truthOut.val();
              if(t > mt){
                mt = t;
                mj = j;
              }
            }
          }

          int r = mancala.move(true, mj);

          if(r == 2){
            done = true;
            break;
          }
          else if(r == 0){
            for(;;){
              size_t j;
              
              for(;;){
                j = random.equilikely(1, 6);
                
                if(mancala.numPieces(false, j) > 0){
                  break;
                }
              }
              
              int r = mancala.move(false, j);

              if(r == 2){
                done = true;
                break;
              }
              else if(r == 0){
                break;
              }
            }
          }

          if(done){
            break;
          }
        }

        totalFitness += mancala.score(true);
      }

      return double(totalFitness)/MATCHES;
    }
    catch(NError& e){
      return undef;
    }
  }

private:
  Index slot;
  Index index0;
  Index index1;
  Index index2;
  Index index3;
  Index index4;
  Index index5;
  Index index6;
  Index index7;
  Index index8;
  Index index9;
  Index index10;
  Index index11;
  Index index12;
  Count count0;
  Count count1;
  Truth truthOut;
  Mancala mancala;
};

int main(int argc, char** argv){
  Program program(argc, argv);

  program.init();
  program.start();

  return 0;
}
