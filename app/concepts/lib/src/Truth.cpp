#include "Truth.h"

#include <iostream>

using namespace std;
using namespace neu;

namespace neu{

class Truth_{
public:
  Truth_(Truth* o)
    : o_(o){

  }

  Truth_(Truth* o, const Truth_& c)
    : o_(o),
      value_(c.value_){
    
  }

  void validate(const nvar& v){
    if(!NConcept::isNumeric(v)){
      NERROR("Non-numeric value: " + v.toStr());
    }
  }

  nvar val(){
    return value_;
  }

  void set(const nvar& v){
    o_->validate(v);
    value_ = v;
    o_->setIn(true);
  }

  Truth* truthAnd(const Truth* t) const{
    Truth* ret = o_->create();

    *ret = value_ * t->val();

    return ret;
  } 

  Truth* truthOr(const Truth* t) const{
    Truth* ret = o_->create();

    *ret = nvar::max(value_, t->val());

    return ret;
  }
  
  Truth* truthNot() const{
    Truth* ret = o_->create();

    *ret = 1 - value_;

    return ret;
  }

private:
  Truth* o_;
  nvar value_;
};
  
} // end namespace neu

Truth::Truth(const Truth& c)
  : Concept(c){
  x_ = new class Truth_(this, *c.x_);
}

Truth::Truth(PrototypeFlag* p, const nvar& metadata)
  : Concept(p, metadata){
  
}

#ifndef META_GUARD
#include "Truth_meta.h"
#endif

Truth* _Truth_Prototype =
  new Truth(NConcept::Prototype, Truth::metadata());

