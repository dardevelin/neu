#ifndef COUNT_H
#define COUNT_H

#include "Integer.h"

#include <neu/NClass.h>

namespace neu{

/**[
  description: "Count",
  enabled: true
]*/
class Count : public Integer{
public:
  Count();

  Count(PrototypeFlag*, const nvar& metadata);

  Count(const Count& c);

  virtual ~Count();

  virtual Count* copy() const{
    return new Count(*this);
  }

  virtual Count* create() const{
    return new Count;
  }

  Count& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Count";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Count")->metadata();
  }

private:
  class Count_* x_;
};

} // end namespace neu

#endif // GENERIC_H
