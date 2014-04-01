#ifndef AREA_H
#define AREA_H

#include "Real.h"

namespace Meta{

class Volume;
class Length;

/**[
  description: "Area",
  description: "Area2"
]*/
class Area : public Real{
public:
  Area();

  Area(PrototypeFlag*, const nvar& metadata);

  Area(const Area& c);

  virtual ~Area();

  virtual Area* copy() const{
    return new Area(*this);
  }

  virtual Area* create() const{
    return new Area;
  }

  Area& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);

  virtual nstr name() const{
    return "Area";
  }

  virtual void validate(const mvar& v);

  /**[
    description: "Do it"
  ]*/
  Volume* mulToVolume(const Length* length) const;

  /**[
    self: [vec:undef],
    l: [ratio:true],
    post: {
      if(l.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  Area* mulLengthRatio(const Length* l) const;

  static nvar metadata(){
    return NClass::getClass("Area")->metadata();
  }

private:
  class Area_* x_;
};

} // end namespace Meta

#endif // AREA_H

