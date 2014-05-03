#include "Volume.h"

using namespace std;
using namespace neu;

namespace neu{

class Volume_{
public:
  Volume_(Volume* o)
    : o_(o){

  }
  
  ~Volume_(){
    
  }

  Volume_(Volume* o, const Volume_& c)
    : o_(o){
    
  }

  void validate(const nvar& v){
    o_->Real::validate(v);

    if(v < 0){
      NERROR("negative value");
    }
  }

private:
  Volume* o_;
};

} // end namespace neu

Volume::Volume(const Volume& c)
  : Real(c){
  x_ = new class Volume_(this, *c.x_);
}

Volume::Volume(PrototypeFlag* p, const nvar& metadata)
  : Real(p, metadata){
  
}

#ifndef META_GUARD
#include "Volume_meta.h"
#endif

Volume* _Volume_Prototype = 
  new Volume(NConcept::Prototype, Volume::metadata());
