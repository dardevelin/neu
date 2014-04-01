#include "Real.h"

#include "Count.h"

using namespace std;
using namespace neu;

namespace neu{

class Real_{
public:
  Real_(Real* o)
    : o_(o){
    
  }

  Real_(Real* o, const Real_& c)
    : o_(o),
      value_(c.value_){
    
  }

  nvar val(){
    return value_;
  }

  void set(const nvar& v){
    o_->validate(v);
    value_ = v;
    o_->setIn(true);
  }

  void validate(const nvar& v){
    if(!Concept::isNumeric(v)){
      NERROR("Non-numeric value: " + v.toStr());
    }
  }

  double match(const nvar& v, bool full){
    double m = o_->NConcept::match(v, full);

    if(m < 0){
      return m;
    }

    const nvar& attrs = o_->attributes();

    if(attrs.hasKey("delta")){
      if(!v.hasKey("delta") && attrs.delta != undef){
        return -1;
      }
    }
    else if(v.hasKey("delta")){
      return -1;
    }

    if(attrs.hasKey("ratio")){
      if(!v.hasKey("ratio") && attrs.ratio != undef){
        return -1;
      }
    }
    else if(v.hasKey("ratio")){
      return -1;
    }

    if(attrs.hasKey("multiplier")){
      if(!v.hasKey("multiplier") && attrs.multiplier != undef){
        return -1;
      }
    }
    else if(v.hasKey("multiplier")){
      return -1;
    }

    if(attrs.hasKey("coord")){
      if(!v.hasKey("coord") && attrs.coord != undef){
        return -1;
      }
    }
    else if(v.hasKey("coord")){
      return -1;
    }

    return m;
  }

  void setDelta(bool flag){
    if(flag){
      o_->setAttribute_("delta", true);
    }
    else{
      o_->eraseAttribute_("delta");
    }
  }

  void undefDelta(){
    o_->setAttribute_("delta", undef);
  }

  bool getDelta() const{
    return o_->hasAttribute_("delta");
  }

  void setRatio(bool flag){
    if(flag){
      o_->setAttribute_("ratio", true);
    }
    else{
      o_->eraseAttribute_("ratio");
    }
  }

  void undefRatio(){
    o_->setAttribute_("ratio", undef);
  }

  bool getRatio() const{
    return o_->hasAttribute_("ratio");
  }

  void setMultiplier(bool flag){
    if(flag){
      o_->setAttribute_("multiplier", true);
    }
    else{
      o_->eraseAttribute_("multiplier");
    }
  }

  void undefMultiplier(){
    o_->setAttribute_("multiplier", undef);
  }

  bool getMultiplier() const{
    return o_->hasAttribute_("multiplier");
  }

  void setCoord(bool flag){
    if(flag){
      o_->setAttribute_("coord", true);
    }
    else{
      o_->eraseAttribute_("coord");
    }
  }

  void undefCoord(){
    o_->setAttribute_("coord", undef);
  }

  bool getCoord() const{
    return o_->hasAttribute_("coord");
  }

  NConcept* sum() const{
    mvar r = 0;

    for(const nvar& vi : value_){
      r += vi;
    }
    
    Real* ret = o_->create();
    *ret = r;

    return ret;
  }

  NConcept* mean() const{
    if(value_.size() == 0){
      NERROR("empty vector");
    }

    nvar r = 0;
    for(const nvar& vi : value_){
      r += vi;
    }

    r /= value_.size();

    Real* ret = o_->create();
    *ret = r;

    return ret;
  }

  NConcept* max() const{
    if(value_.size() == 0){
      NERROR("empty vector");
    }

    nvar m = mvar::negInf();
    for(const nvar& vi : value_){
      if(vi > m){
        m = vi;
      }
    }

    Real* ret = o_->create();
    *ret = m;

    return ret;
  }

  NConcept* min() const{
    nvar m = nvar::inf();
    for(const nvar& vi : value_){
      if(vi < m){
        m = vi;
      }
    }

    Real* ret = o_->create();
    *ret = m;

    return ret;
  }

  NConcept* setUnion(const NConcept* c) const{
    nvar r = value_;
    r.unite(c->val());

    Real* ret = o_->create();

    nlist& l = r;
    nvec rv(l.begin(), l.end());

    *ret = rv;

    return ret;
  }

  NConcept* setIntersection(const NConcept* c) const{
    mvar r = value_;

    r.intersect(c->val());

    Real* ret = o_->create();

    nlist& l = r;
    nvec rv(l.begin(), l.end());

    *ret = rv;

    return ret;
  }

  NConcept* setComplement(const NConcept* c) const{
    nvar r = value_;
    r.complement(c->val());

    Real* ret = o_->create();

    nlist& l = r;
    nvec rv(l.begin(), l.end());

    *ret = rv;

    return ret;
  }

  NConcept* add(const NConcept* c) const{
    Real* ret = o_->create();

    *ret = value_ + c->val();

    return ret;
  }

  void foo(){
    
  }

  void bar(){
    ++value_;
  }

  Count* length() const{
    const nvar& val = o_->val();
    
    assert(val.hasVector());

    Count* ret = new Count;
    *ret = val.size();

    return ret;
  }

  NConcept* mulCount(const Count* count) const{
    //assert(count->val().isInteger());

    nvar v = o_->val();
    v *= count->val();
    
    Real* ret = o_->create();
    *ret = v;
    return ret;
  }

  NConcept* divCount(const Count* count) const{
    //assert(count->val().isInteger());

    nvar v = o_->val();
    v /= count->val();
    
    Real* ret = o_->create();
    *ret = v;
    return ret;
  }
  
  NConcept* mulRatio(const NConcept* r) const{
    nvar v = o_->val();
    v *= r->val();
    
    Real* ret = o_->create();
    *ret = v;
    return ret;
  }

  NConcept* divRatio(const NConcept* r) const{
    nvar v = o_->val();
    v /= r->val();
    
    Real* ret = o_->create();
    *ret = v;
    return ret;
  }

  NConcept* divToRatio(const NConcept* c) const{
    nvar v = o_->val();
    v /= c->val();
    
    Real* ret = o_->create();
    *ret = v;
    return ret;    
  }

private:
  Real* o_;
  nvar value_;
};

} // end namespace Meta

Real::Real(const Real& c)
  : Concept(c){
  x_ = new class Real_(this, *c.x_);
}

Real::Real(PrototypeFlag* p, const mvar& metadata)
  : Concept(p, metadata){
  
}

#ifndef META_GUARD
#include "Real_meta.h"
#endif

Real* _Real_Prototype = 
  new Real(NConcept::Prototype, Real::metadata());
