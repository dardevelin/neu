/*
 
      ___           ___           ___     
     /\__\         /\  \         /\__\    
    /::|  |       /::\  \       /:/  /    
   /:|:|  |      /:/\:\  \     /:/  /     
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___ 
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  / 
     |::/  /     \:\ \/__/     \:\/:/  /  
     /:/  /       \:\__\        \::/  /   
     \/__/         \/__/         \/__/    
 
 
Neu, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#ifndef NEU_N_NETWORK_H
#define NEU_N_NETWORK_H

#include <neu/NNeuron.h>
#include <neu/NRandom.h>

namespace neu{

class NNetwork{
public:
  typedef NVector<double> ValueVec;

  class Layer{
  public:
    Layer(){

    }

    Layer(size_t size){
      for(size_t i = 0; i < size; ++i){
        addNeuron(new NNeuron);
      }
    }

    Layer(NNeuron::Func* activationFunc, size_t size){
      for(size_t i = 0; i < size; ++i){
        addNeuron(new NNeuron(activationFunc));
      }
    }

    Layer(const NNeuron::Func& activationFunc, size_t size){
      for(size_t i = 0; i < size; ++i){
        addNeuron(new NNeuron(activationFunc));
      }
    }

    ~Layer(){
      for(size_t i = 0; i < neuronVec_.size(); ++i){
        delete neuronVec_[i];
      }
    }

    void addNeuron(NNeuron* neuron){
      neuronVec_.push_back(neuron);
    }

    NNeuron* neuron(size_t i){
      assert(i < neuronVec_.size());

      return neuronVec_[i];
    }

    size_t size() const{
      return neuronVec_.size();
    }

  private:
    typedef NVector<NNeuron*> NeuronVec_;
  
    NeuronVec_ neuronVec_;
  };

  NNetwork()
    : bias_(0){
    random_.timeSeed();
  }

  ~NNetwork(){
    for(size_t i = 0; i < layerVec_.size(); ++i){
      delete layerVec_[i];
    }
  }
  
  void setSeed(int64_t seed){
    random_.setSeed(seed);
  }

  void addLayer(const NNeuron::Func& activationFunc,
                size_t size,
                double wmin,
                double wmax){
    Layer* l = new Layer(activationFunc, size);
    addLayer(l, wmin, wmax);
  }

  void addLayer(NNeuron::Func* activationFunc,
                size_t size,
                double wmin,
                double wmax){
    Layer* l = new Layer(activationFunc, size);
    addLayer(l, wmin, wmax);
  }

  void addLayer(Layer* layer){
    addLayer(layer, 0, 0);
  }

  void addLayer(Layer* layer, double wmin, double wmax){
    Layer* l;

    if(layerVec_.empty()){
      l = 0;
    }
    else{
      l = layerVec_.back();
    }

    layerVec_.push_back(layer);

    if(l){
      connect_(l, layer, wmin, wmax);
    }
  }

  Layer* layer(size_t l){
    assert(l < layerVec_.size());

    return layerVec_[l];
  }

  Layer* backLayer(size_t l){
    return layer(layerVec_.size() - l - 1);
  }

  NNeuron* neuron(size_t i, size_t j){
    Layer* l = layer(i);
    return l->neuron(j);
  }

  void addBias(double wmin, double wmax){
    assert(!bias_ && "Already added bias neuron");

    bias_ = new NNeuron;
    for(size_t i = 1; i < layerVec_.size(); ++i){
      Layer* l = layerVec_[i];
      for(size_t j = 0; j < l->size(); ++j){
        NNeuron* nj = l->neuron(j);
        nj->receive(bias_, random_.uniform(wmin, wmax));
      }
    }
  }

  NNeuron* bias(){
    return bias_;
  }

  size_t numLayers() const{
    return layerVec_.size();
  }

  double error(const ValueVec& desired){
    Layer* l = backLayer(0);

    double error = 0;
    double e;
    
    size_t size = l->size();
    for(size_t j = 0; j < size; ++j){
      e = desired[j] - l->neuron(j)->output();
      e *= e;
      error += e;
    }

    error /= l->size();

    return error;
  }

  void backPropagate(const ValueVec& desired, double eta){
    Layer* l2 = backLayer(0);

    assert(desired.size() == l2->size());

    Layer* l1 = backLayer(1);

    ValueVec d2(l2->size(), 0);

    size_t size2 = l2->size();
    for(size_t j = 0; j < size2; ++j){
      NNeuron* nj = l2->neuron(j);
      double e = desired[j] - nj->output();
      
      const NNeuron::Func& f = nj->activationFunc();
      d2[j] = e * f.deriv(nj->activation());
      
      size_t size1 = l1->size();

      for(size_t i = 0; i < size1; ++i){
        NNeuron* ni = l1->neuron(i);
        double dw = eta * d2[j] * ni->output();
        nj->adjustWeight(ni, dw);
      }
    }

    for(size_t l = 2; l < numLayers(); ++l){
      ValueVec d1(l1->size(), 0);

      for(size_t j = 0; j < l1->size(); ++j){
        NNeuron* nj = l1->neuron(j);
        const NNeuron::Func& f = nj->activationFunc();

        d1[j] = f.deriv(nj->activation());
        double s = 0;
        for(size_t k = 0; k < l2->size(); ++k){
          NNeuron* nk = l2->neuron(k);
          s += nk->weight(nj) * d2[k];
        }
        
        d1[j] *= s;
      }
              
      l2 = l1;
      l1 = backLayer(l);
      d2 = d1;
      
      for(size_t j = 0; j < l2->size(); ++j){
        NNeuron* nj = l2->neuron(j);
        
        for(size_t i = 0; i < l1->size(); ++i){
          NNeuron* ni = l1->neuron(i);
          
          double dw = eta * d2[j] * ni->output();
          nj->adjustWeight(ni, dw);
        }
      }
    }
  }

private:
  void connect_(Layer* l1, Layer* l2, double wmin, double wmax){
    for(size_t i = 0; i < l2->size(); ++i){
      NNeuron* ni = l2->neuron(i);
      
      for(size_t j = 0; j < l1->size(); ++j){
        NNeuron* nj = l1->neuron(j);
        ni->receive(nj, random_.uniform(wmin, wmax));
      }
    }
  }

  typedef NVector<Layer*> LayerVec_;
  
  LayerVec_ layerVec_;
  NNeuron* bias_;
  NRandom random_;
};

} // end namespace neu

#endif // NEU_N_NETWORK_H
