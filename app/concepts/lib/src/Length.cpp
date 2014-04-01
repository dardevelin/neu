#include "Length.h"

#include <iostream>

#include "Area.h"

using namespace std;
using namespace neu;

namespace neu{

class Length_{
public:
  Length_(Length* o)
    : o_(o){

  }

  Length_(Length* o, const Length_& c)
    : o_(o){
    
  }

  Area* mulToArea(const Length* length) const{
    Area* area = new Area;
    *area = length->val()*o_->val();

    return area;
  }

  void validate(const nvar& v){
    if(!NConcept::isNumeric(v, false)){
      NERROR("invalid value: " + v.toStr());
    }
  }
  
private:
  Length* o_;
};
  
} // end namespace Meta

Length::Length(const Length& c)
  : Real(c){
  x_ = new class Length_(this, *c.x_);
}

Length::Length(PrototypeFlag* p, const nvar& metadata)
  : Real(p, metadata){
  
}

#ifndef META_GUARD
#include "Length_meta.h"
#endif

Length* _Length_Prototype = 
  new Length(NConcept::Prototype, Length::metadata());
