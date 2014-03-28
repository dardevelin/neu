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

#ifndef NEU_N_NEURON_H
#define NEU_N_NEURON_H

#include <map>
#include <cmath>
#include <cassert>

#include <neu/nvar.h>

namespace neu{
  
  class NNeuron{
  public:
    class Func{
    public:
      virtual ~Func(){
        
      }
      
      virtual double operator()(double value) const = 0;
      
      virtual double deriv(double value) const = 0;
      
      virtual Func* copy() const = 0;
    };
    
    class LinearFunc : public Func{
    public:
      double operator()(double v) const{
        return v;
      }
      
      double deriv(double v) const{
        return 1;
      }
      
      virtual LinearFunc* copy() const{
        return new LinearFunc(*this);
      }
    };
    
    class SigmoidFunc : public Func{
    public:
      SigmoidFunc(double a)
      : a_(a){
        
      }
      
      double operator()(double v) const{
        return 1.0 / (1.0 + std::exp(-a_ * v));
      }
      
      double deriv(double v) const{
        double d = 1 + std::exp(-a_ * v);
        
        return (a_ * std::exp(-a_ * v))/(d*d);
      }
      
      virtual SigmoidFunc* copy() const{
        return new SigmoidFunc(*this);
      }
      
      double a() const{
        return a_;
      }
      
    private:
      double a_;
    };
    
    class SignumFunc : public Func{
    public:
      SignumFunc(double threshold)
      : threshold_(threshold){
        
      }
      
      double operator()(double v) const{
        return v < threshold_ ? -1.0 : 1.0;
      }
      
      double deriv(double v) const{
        return nvar::nan();
      }
      
      virtual SignumFunc* copy() const{
        return new SignumFunc(*this);
      }
      
    private:
      double threshold_;
    };
    
    NNeuron(const Func& activationFunc)
    : activationFunc_(activationFunc.copy()),
    ownsActivationFunc_(true),
    totalInputs_(0),
    inputsReceived_(0),
    activation_(0),
    output_(nvar::nan()){
      
    }
    
    NNeuron(Func* activationFunc)
    : activationFunc_(activationFunc),
    ownsActivationFunc_(false),
    totalInputs_(0),
    inputsReceived_(0),
    activation_(0),
    output_(nvar::nan()){
      
    }
    
    NNeuron()
    : activationFunc_(linearFunc),
    ownsActivationFunc_(false),
    totalInputs_(0),
    inputsReceived_(0),
    activation_(0),
    output_(nvar::nan()){
      
      receive(0, 1.0);
    }
    
    ~NNeuron(){
      if(ownsActivationFunc_){
        delete activationFunc_;
      }
    }
    
    void input(double value){
      activate_(0, value);
    }
    
    void receive(NNeuron* inputNeuron, double weight){
      auto itr = inputMap_.find(inputNeuron);
      assert(itr == inputMap_.end());
      
      inputMap_.insert({inputNeuron, weight});
      ++totalInputs_;
      
      if(inputNeuron){
        itr = inputNeuron->outputMap_.find(this);
        assert(itr == inputNeuron->outputMap_.end());
        
        inputNeuron->outputMap_.insert({this, 0});
      }
    }
    
    double weight(NNeuron* inputNeuron) const{
      auto itr = inputMap_.find(inputNeuron);
      assert(itr != inputMap_.end());
      
      return itr->second;
    }
    
    void setWeight(NNeuron* inputNeuron, double weight){
      auto itr = inputMap_.find(inputNeuron);
      assert(itr != inputMap_.end());
      
      itr->second = weight;
    }
    
    void adjustWeight(NNeuron* inputNeuron, double dw){
      auto itr = inputMap_.find(inputNeuron);
      assert(itr != inputMap_.end());
      
      itr->second += dw;
    }
    
    size_t inputCount() const{
      return inputMap_.size();
    }
    
    double output() const{
      assert(!std::isnan(output_));
      
      return output_;
    }
    
    void reset(){
      output_ = nvar::nan();
    }
    
    double activation() const{
      return activation_;
    }
    
    const Func& activationFunc() const{
      return *activationFunc_;
    }
    
    static LinearFunc* linearFunc;
    static SigmoidFunc* sigmoidFunc;
    static SignumFunc* signumFunc;
    
  private:
    typedef NMap<NNeuron*, double> NeuronValueMap_;
    
    void activate_(NNeuron* inputNeuron, double value){
      auto itr = inputMap_.find(inputNeuron);
      assert(itr != inputMap_.end());
      
      activation_ += value * itr->second;
      
      ++inputsReceived_;
      
      if(inputsReceived_ == totalInputs_){
        output_ = (*activationFunc_)(activation_);
        
        itr = outputMap_.begin();
        auto itrEnd = outputMap_.end();
        
        while(itr != itrEnd){
          itr->first->activate_(this, output_);
          ++itr;
        }
        
        inputsReceived_ = 0;
        activation_ = 0;
      }
    }  
    
    Func* activationFunc_;
    bool ownsActivationFunc_;
    NeuronValueMap_ inputMap_;
    NeuronValueMap_ outputMap_;
    uint32_t totalInputs_;
    uint32_t inputsReceived_;
    double activation_;
    double output_;
  };
  
} // end namespace neu

#endif // NEU_N_NEURON_H
