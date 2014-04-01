#ifndef ANGLE_H
#define ANGLE_H

#include "Real.h"

#include <neu/NClass.h>

namespace neu{

class Length;

/**[
  description: "Angle",
  enabled: true
]*/
class Angle : public Real{
public:
  Angle();

  Angle(PrototypeFlag*, const nvar& metadata);

  Angle(const Angle& c);

  virtual ~Angle();

  virtual Angle* copy() const{
    return new Angle(*this);
  }

  virtual Angle* create() const{
    return new Angle;
  }

  Angle& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Angle";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Angle")->metadata();
  }

 /**[
    self: [vec:undef],
    ret: [ratio:true],
    post: {
      if(self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  Length* sin() const;

  /**[
    self: [vec:undef],
    ret: [ratio:true],
    post: {
      if(self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  Length* cos() const;

private:
  class Angle_* x_;
};

} // end namespace neu

#endif // GENERIC_H
