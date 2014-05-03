#include "Integer.h"

#include <iostream>

using namespace std;
using namespace neu;

namespace neu{

class Integer_{
public:
  Integer_(Integer* o)
    : o_(o){

  }

  Integer_(Integer* o, const Integer_& c)
    : o_(o){
    
  }

  void validate(const nvar& v){
    
  }
  
private:
  Integer* o_;
};
  
} // end namespace neu

Integer::Integer(const Integer& c)
  : Real(c){
  x_ = new class Integer_(this, *c.x_);
}

Integer::Integer(PrototypeFlag* p, const nvar& metadata)
  : Real(p, metadata){
  
}

#ifndef META_GUARD
#include "Integer_meta.h"
#endif

Integer* _Integer_Prototype =
  new Integer(NConcept::Prototype, Integer::metadata());

