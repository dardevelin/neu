#include "Area.h"

#include "Length.h"
#include "Volume.h"

using namespace neu;

namespace neu{

class Area_{
public:
  Area_(Area* o)
    : o_(o){
    
  }

  Area_(Area* o, const Area_& c)
    : o_(o){
    
  }

  Volume* mulToVolume(const Length* length) const{
    Volume* volume = new Volume;
    volume->set(length->val()*o_->val());
    return volume;
  }

  Area* mulLengthRatio(const Length* l) const{
    nvar v = o_->val();
    v *= l->val();
    
    Area* ret = new Area;
    *ret = v;
    return ret;
  }

  void validate(const nvar& v){
    o_->Real::validate(v);

    if(v < 0){
      NERROR("negative value");
    }
  }

private:
  Area* o_;
};

} // end namespace Meta

Area::Area(const Area& c)
  : Real(c){
  x_ = new class Area_(this, *c.x_);
}

Area::Area(PrototypeFlag* p, const nvar& metadata)
  : Real(p, metadata){
  
}

#ifndef META_GUARD
#include "Area_meta.h"
#endif

Area* _Area_Prototype = 
  new Area(NConcept::Prototype, Area::metadata());
