#include "Count.h"

#include <iostream>

using namespace std;
using namespace neu;

namespace neu{

class Count_{
public:
  Count_(Count* o)
    : o_(o){

  }

  Count_(Count* o, const Count_& c)
    : o_(o){
    
  }

  void validate(const nvar& v){
    if(!NConcept::isInteger(v, false)){
      NERROR("invalid value: " + v.toStr());
    }
  }
  
private:
  Count* o_;
};
  
} // end namespace Meta

Count::Count(const Count& c)
  : Integer(c){
  x_ = new class Count_(this, *c.x_);
}

Count::Count(PrototypeFlag* p, const mvar& metadata)
  : Integer(p, metadata){
  
}

#ifndef META_GUARD
#include "Count_meta.h"
#endif

Count* _Count_Prototype =
  new Count(NConcept::Prototype, Count::metadata());

