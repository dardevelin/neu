#ifndef INDEX_H
#define INDEX_H

#include "Count.h"

namespace neu{

/**[
  description: "Index",
  enabled: true
]*/
class Index : public Count{
public:
  Index();

  Index(PrototypeFlag*, const nvar& metadata);

  Index(const Index& c);

  virtual ~Index();

  virtual Index* copy() const{
    return new Index(*this);
  }

  virtual Index* create() const{
    return new Index;
  }

  Index& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Index";
  }

  static nvar metadata(){
    return NClass::getClass("Index")->metadata();
  }

private:
  class Index_* x_;
};

} // end namespace Meta

#endif // GENERIC_H
