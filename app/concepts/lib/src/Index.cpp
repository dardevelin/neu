#include "Index.h"

#include <iostream>

using namespace std;
using namespace neu;

namespace neu{

class Index_{
public:
  Index_(Index* o)
    : o_(o){

  }

  Index_(Index* o, const Index_& c)
    : o_(o){
    
  }

private:
  Index* o_;
};
  
} // end namespace neu

Index::Index(const Index& c)
  : Count(c){
  x_ = new class Index_(this, *c.x_);
}

Index::Index(PrototypeFlag* p, const mvar& metadata)
  : Count(p, metadata){
  
}

#ifndef META_GUARD
#include "Index_meta.h"
#endif

Index* _Index_Prototype =
  new Index(NConcept::Prototype, Index::metadata());

