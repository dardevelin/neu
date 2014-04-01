#ifndef LENGTH_H
#define LENGTH_H

#include "Real.h"

#include <iostream>

#include <neu/NClass.h>

namespace neu{

class Area;

/**[
  description: "Length",
  enabled: true
]*/
class Length : public Real{
public:
  Length();

  Length(PrototypeFlag*, const nvar& metadata);

  Length(const Length& c);

  virtual ~Length();

  virtual Length* copy() const{
    return new Length(*this);
  }

  virtual Length* create() const{
    return new Length;
  }

  Length& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Length";
  }

  /**[
    self: [vec:undef],
    l: [vec:undef, takeThis:true],
    post: {
      if(l.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  Area* mulToArea(const Length* l) const;

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Length")->metadata();
  }

private:
  class Length_* x_;
};

} // end namespace neu

#endif // LENGTH_H

