#include <iostream>

#include <neu/NNet.h>
#include <neu/NRandom.h>
#include <neu/nvar.h>

using namespace std;
using namespace neu;

static const double IW = 0.3;
static const double RANGE = 105.0;

int main(int argc, char** argv){
  NNet network;

  NRandom random;

  NNet::Layer* inputLayer = new NNet::Layer(1);
  network.addLayer(inputLayer);
  network.addLayer(NNet::sigmoidFunc, 50, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 1, -IW, IW);
  network.addBias(-IW, IW);

  double eta = 0.001;

  NNet::Neuron* ib = network.bias();

  size_t count = 0;
  size_t inc = 0;
  double minError = nvar::inf();

  NNet::ValueVec xs;
  NNet::ValueVec ys;

  for(size_t j = 0; j < 100; ++j){
    double x = random.uniform(0, 10);
    double y = x*x + 5;
    x /= RANGE;
    y /= RANGE;

    xs.push_back(x);
    ys.push_back(y);
  }

  for(size_t i = 0; i < 1000000; ++i){
    double error = 0;

    for(size_t j = 0; j < xs.size(); ++j){
      ib->input(1.0);
      inputLayer->neuron(0)->input(xs[j]);
      NNet::ValueVec desired;
      desired.push_back(ys[j]);
      
      network.backPropagate(desired, eta);
      error += network.error(desired);
    }

    error = sqrt(error/100.0);

    /*
    if(error < minError){
      eta *= 1.01;
      minError = error;
      cout << "minError: " << minError << endl;
      cout << "inc error: " << error << endl;
    }
    else{
      eta *= 0.99;
      cout << "dec error: " << error << endl;
    }

    cout << "eta is: " << eta << endl;
    */

    if(i % 1000 == 0){
      cout << error << endl;
    }

    if(error < 0.036){
      break;
    }
  }

  for(size_t j = 0; j < 100; ++j){
    double x = random.uniform(0, 10);
    ib->input(1.0);
    inputLayer->neuron(0)->input(x/RANGE);
    cout << "x: " << x << endl;
    cout << "y: " << network.backLayer(0)->neuron(0)->output()*RANGE << 
      endl << endl;
  }

  return 0;
}

