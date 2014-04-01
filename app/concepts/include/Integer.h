#ifndef INTEGER_H
#define INTEGER_H

#include "Real.h"

#include <neu/NClass.h>

namespace neu{

/**[
  description: "Integer",
  enabled: true
]*/
class Integer : public Real{
public:
  Integer();

  Integer(PrototypeFlag*, const nvar& metadata);

  Integer(const Integer& c);

  virtual ~Integer();

  virtual Integer* copy() const{
    return new Integer(*this);
  }

  virtual Integer* create() const{
    return new Integer;
  }

  Integer& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Integer";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Integer")->metadata();
  }

private:
  class Integer_* x_;
};

} // end namespace neu

#endif // GENERIC_H
