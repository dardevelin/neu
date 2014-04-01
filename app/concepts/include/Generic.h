#ifndef GENERIC_H
#define GENERIC_H

#include "Base.h"

namespace neu{

/**[
  description: "Generic",
  enabled: true
]*/
class Generic : public Base{
public:
  Generic();

  Generic(PrototypeFlag*, const nvar& metadata);

  Generic(const Generic& c);

  virtual ~Generic();

  virtual Generic* copy() const{
    return new Generic(*this);
  }

  virtual Generic* create() const{
    return new Generic;
  }

  Generic& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Generic";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Generic")->metadata();
  }

private:
  class Generic_* x_;
};

} // end namespace neu

#endif // GENERIC_H

