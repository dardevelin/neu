// this example demonstrates how multilayer feed-forward neural
// network can be trained using on backpropagation on the C++ side,
// not shown here: the network after it is trained, can be compiled
// for high performance concurrent execution using NNModule.

// we will train the network to approximate the function y = x^2 + 5

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

  // use a single input
  NNet::Layer* inputLayer = new NNet::Layer(1);
  network.addLayer(inputLayer);

  // add a hidden layer of 50 neurons
  network.addLayer(NNet::sigmoidFunc, 50, -IW, IW);

  // one output
  network.addLayer(NNet::sigmoidFunc, 1, -IW, IW);
  network.addBias(-IW, IW);

  double eta = 0.1;

  NNet::Neuron* ib = network.bias();

  size_t count = 0;
  size_t inc = 0;
  double minError = nvar::inf();

  // the vector of inputs
  NNet::ValueVec xs;
  
  // the vector of outputs
  NNet::ValueVec ys;

  // construct the training set
  for(size_t j = 0; j < 100; ++j){
    double x = random.uniform(0, 10);
    double y = x*x + 5;
    x /= RANGE;
    y /= RANGE;

    xs.push_back(x);
    ys.push_back(y);
  }

  // train the network with back-propagation
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

    cout << "RMS: " << error << endl;

    if(error < 0.036){
      break;
    }
  }

  // test the network on some inputs which were not in the training set
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

