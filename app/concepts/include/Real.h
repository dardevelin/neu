#ifndef REAL_H
#define REAL_H

#include <neu/NConcept.h>

namespace neu{

class Count;

/**[
  description: "Real"
]*/
class Real : public NConcept{
public:
  Real();

  Real(const Real& c);

  Real(PrototypeFlag*, const nvar& metadata);

  virtual ~Real();

  virtual Real* copy() const{
    return new Real(*this);
  }

  virtual Real* create() const{
    return new Real;
  }

  Real& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);

  virtual nstr name() const{
    return "Real";
  }

  virtual nvar val() const final;

  virtual void set(const nvar& v) final;

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Real")->metadata();
  }

  virtual double match(const nvar& v, bool full);

  void setDelta(bool flag);

  bool getDelta() const;

  void undefDelta();

  void setRatio(bool flag);

  bool getRatio() const;

  void undefRatio();

  void setMultiplier(bool flag);

  bool getMultiplier() const;

  void undefMultiplier();

  void setCoord(bool flag);

  bool getCoord() const;

  void undefCoord();

  /**[
    self: [vec:true]
  ]*/
  NConcept* sum() const;

  /**[
    self: [vec:true]
  ]*/
  NConcept* mean() const;

  /**[
    self: [vec:true]
  ]*/
  NConcept* max() const;

  /**[
    self: [vec:true]
  ]*/
  NConcept* min() const;

  /**[
    self: [set:true, vec:true],
    c: [set:true, vec:true],
    ret: [set:true, vec:true]
  ]*/
  NConcept* setUnion(const NConcept* c) const;

  /**[
    self: [set:true, vec:true],
    c: [set:true, vec:true],
    ret: [set:true, vec:true]
  ]*/
  NConcept* setIntersection(const NConcept* c) const;

  /**[
    self: [set:true, vec:true],
    c: [set:true, vec:true],
    ret: [set:true, vec:true]
  ]*/
  NConcept* setComplement(const NConcept* c) const;

  /**[
    enabled: true,
    self: [set:true, vec:true]
  ]*/
  void foo();

  /*[
    enabled: true,
    self: [set:false, vec:false]
  ]*/  
  void bar();

  /**[
    self: [vec:undef],
    c: [vec:undef, takeThis:true],
    post: {
      if(c.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  NConcept* add(const NConcept* c) const;

  /**[
    self: [vec:true]
  ]*/
  Count* length() const;

  /**[
    self: [vec:undef, ratio:undef],
    count: [vec:undef],
    post: {
      if(count.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }

      if(self.hasKey("ratio")){
        ret.ratio = true;
      }
    }
  ]*/
  NConcept* mulCount(const Count* count) const;

  /**[
    self: [vec:undef, ratio:undef],
    count: [vec:undef],
    post: {
      if(count.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }

      if(self.hasKey("ratio")){
        ret.ratio = true;
      }
    }
  ]*/
  NConcept* divCount(const Count* count) const;

  /**[
    self: [vec:undef, ratio:undef],
    r: [ratio:true],
    post: {
      if(r.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  NConcept* mulRatio(const NConcept* r) const;

  /**[
    self: [vec:undef],
    r: [ratio:true],
    post: {
      if(r.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  NConcept* divRatio(const NConcept* r) const;

  /**[
    self: [vec:undef],
    c: [vec:undef],
    ret: [ratio:true],
    post: {
      if(c.hasKey("vec") || self.hasKey("vec")){
        ret.vec = true;
      }
    }
  ]*/
  NConcept* divToRatio(const NConcept* c) const;

private:
  class Real_* x_;
};

} // end namespace Meta

#endif // REAL_H

