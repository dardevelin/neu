#include "Foo.h"

using namespace std;
using namespace neu;

namespace neu{

class Foo_{
public:
  Foo_(Foo* o)
  : o_(o),
    count_(0){

  }

  ndist bar(){
    return count_++;
  }

  ndist baz(int x){
    return count_ += x;
  }

private:
  Foo* o_;
  int count_;
};

} // end namespace neu

Foo::Foo(NBroker* broker)
  : NObject(broker){
  x_ = new Foo_(this);
}

#ifndef META_GUARD
#include "Foo_meta.h"
#endif

