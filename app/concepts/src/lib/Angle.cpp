#include "Angle.h"

#include <iostream>

#include "Length.h"

using namespace std;
using namespace neu;

namespace neu{

class Angle_{
public:
  Angle_(Angle* o)
    : o_(o){

  }

  Angle_(Angle* o, const Angle_& c)
    : o_(o){
    
  }

  void validate(const nvar& v){
    
  }
  
  Length* sin() const{
    const nvar& val = o_->val();

    Length* length = new Length;
    *length = nvar::sin(val);

    return length;
  }

  Length* cos() const{
    const nvar& val = o_->val();

    Length* length = new Length;
    *length = nvar::cos(val);

    return length;
  }

private:
  Angle* o_;
};
  
} // end namespace Meta

Angle::Angle(const Angle& c)
  : Real(c){
  x_ = new class Angle_(this, *c.x_);
}

Angle::Angle(PrototypeFlag* p, const nvar& metadata)
  : Real(p, metadata){
  
}

#ifndef META_GUARD
#include "Angle_meta.h"
#endif

Angle* _Angle_Prototype =
  new Angle(NConcept::Prototype, Angle::metadata());

