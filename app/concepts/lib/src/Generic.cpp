#include "Generic.h"

#include <iostream>

using namespace std;
using namespace neu;

namespace neu{

class Generic_{
public:
  Generic_(Generic* o)
    : o_(o){

  }

  Generic_(Generic* o, const Generic_& c)
    : o_(o){
    
  }

  void validate(const nvar& v){
    
  }
  
private:
  Generic* o_;
};
  
} // end namespace neu

Generic::Generic(const Generic& c)
  : Base(c){
  x_ = new class Generic_(this, *c.x_);
}

Generic::Generic(PrototypeFlag* p, const mvar& metadata)
  : Base(p, metadata){
  
}

#ifndef META_GUARD
#include "Generic_meta.h"
#endif

Generic* _Generic_Prototype = 
  new Generic(NConcept::Prototype, Generic::metadata());
