#include <iostream>

#include <neu/NNet.h>
#include <neu/NNModule.h>
#include <neu/NSys.h>
#include <neu/NPLVector.h>

using namespace std;
using namespace neu;

static const double IW = 0.3;
static const double RANGE = 105.0;

int main(int argc, char** argv){
  NNet network;
  network.setSeed(0);

  NNet::Layer* inputLayer = new NNet::Layer(2);
  network.addLayer(inputLayer);
  network.addLayer(NNet::sigmoidFunc, 100, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 100, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 100, -IW, IW);
  network.addLayer(NNet::sigmoidFunc, 2, -IW, IW);

  NNet::Layer* outputLayer = network.backLayer(0);

  double t1 = NSys::now();
  inputLayer->neuron(0)->input(0.25);
  inputLayer->neuron(1)->input(0.70);
  double dt = NSys::now() - t1;
  //cout << "c++ time is: " << dt << endl;

  cout << "output[0]= " << outputLayer->neuron(0)->output() << endl;
  cout << "output[1]= " << outputLayer->neuron(1)->output() << endl;

  NNModule module;
  t1 = NSys::now();
  module.compile("test", network, 8);
  dt = NSys::now() - t1;
  //cout << "compile time is: " << dt << endl;

  typedef NPLVector<double, 2> Vec2;

  Vec2 inputs;
  inputs[0] = 0.25;
  inputs[1] = 0.70;

  Vec2 outputs;

  t1 = NSys::now();
  module.run("test", &inputs, &outputs);
  dt = NSys::now() - t1;
  //cout << "run time is: " << dt << endl;

  cout << "outputs: " << outputs << endl;

  module.remove("test");

  return 0;
}
