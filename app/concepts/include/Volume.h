#ifndef VOLUME_H
#define VOLUME_H

#include "Real.h"

namespace neu{

/**[
  description: "Volume"
]*/
class Volume : public Real{
public:
  Volume();

  Volume(PrototypeFlag*, const nvar& metadata);

  Volume(const Volume& c);

  virtual ~Volume();

  virtual Volume* copy() const{
    return new Volume(*this);
  }

  virtual Volume* create() const{
    return new Volume;
  }

  Volume& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);

  virtual nstr name() const{
    return "Volume";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Volume")->metadata();
  }

private:
  class Volume_* x_;
};

} // end namespace neu

#endif // VOLUME_H

