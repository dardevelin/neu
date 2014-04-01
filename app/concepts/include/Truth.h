#ifndef TRUTH_H
#define TRUTH_H

#include <neu/NConcept.h>

namespace neu{

/**[
  description: "Truth",
  enabled: true
]*/
class Truth : public NConcept{
public:
  Truth();

  Truth(PrototypeFlag*, const nvar& metadata);

  Truth(const Truth& c);

  virtual ~Truth();

  virtual Truth* copy() const{
    return new Truth(*this);
  }

  virtual Truth* create() const{
    return new Truth;
  }

  Truth& operator=(const mvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Truth";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Truth")->metadata();
  }

  virtual nvar val() const final;

  virtual void set(const nvar& v) final;

  /**[
    enabled: true
  ]*/
  Truth* truthAnd(const Truth* t) const; 

  /*[
    enabled: true
  ]*/
  Truth* truthOr(const Truth* t) const; 
  
  /*[
    enabled: true
  ]*/
  Truth* truthNot() const;   

private:
  class Truth_* x_;

  nvar value_;
};

} // end namespace neu

#endif // GENERIC_H
