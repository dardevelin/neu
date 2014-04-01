#ifndef MANCALA_H
#define MANCALA_H

#include <neu/NConcept.h>

namespace neu{

class Index;
class Count;
class Truth;

/**[
  description: "Mancala",
  enabled: true
]*/
class Mancala : public NConcept{
public:
  Mancala();

  Mancala(PrototypeFlag*, const nvar& metadata);

  Mancala(const Mancala& c);

  virtual ~Mancala();

  virtual Mancala* copy() const{
    return new Mancala(*this);
  }

  virtual Mancala* create() const{
    return new Mancala;
  }

  Mancala& operator=(const nvar& v){
    set(v);
    return *this;
  }

  virtual NFunc handle(const nvar& n, uint32_t flags);
  
  virtual nstr name() const{
    return "Mancala";
  }

  virtual void validate(const nvar& v);

  static nvar metadata(){
    return NClass::getClass("Mancala")->metadata();
  }

  virtual nvar val() const final;

  virtual void set(const nvar& v) final;

  void reset();

  int move(bool first, size_t slot);

  size_t score(bool first) const;

  size_t numPieces(bool first, size_t slot);

  void writeBoard() const;

  /**[
    enabled: true
  ]*/
  Count* numPieces(const Index* i) const;

  /**[
    enabled: true
  ]*/
  Index* slotOffset(const Index* i, const Count* c) const;

  /**[
    enabled: true
  ]*/
  Count* slotDiff(const Index* a, const Index* b) const;

  /**[
    enabled: true
  ]*/
  Truth* isMine(const Index* i) const;

  /**[
    enabled: true
  ]*/
  Truth* truthAnd(const Truth* a, const Truth* b) const;

  /**[
    enabled: true
  ]*/
  Truth* truthOr(const Truth* a, const Truth* b) const;

  /**[
    enabled: true
  ]*/
  Truth* truthNot(const Truth* t) const;

  /**[
    enabled: true
  ]*/
  Truth* countIndexEqual(const Count* c, const Index* i) const;

  /**[
    enabled: true
  ]*/
  Truth* countIndexGreater(const Count* c, const Index* i) const;

  /**[
    enabled: true
  ]*/
  Truth* countIndexLesser(const Count* c, const Index* i) const;

  /**[
    enabled: true
  ]*/
  Truth* countEqual(const Count* a, const Count* b) const;

  /**[
    enabled: true
  ]*/
  Truth* countGreater(const Count* a, const Count* b) const;

  /**[
    enabled: true
  ]*/
  Truth* countLesser(const Count* a, const Count* b) const;

  /**[
    enabled: true
  ]*/
  Count* countAdd(const Count* a, const Count* b) const;

  /**[
    enabled: true
  ]*/
  Count* countSub(const Count* a, const Count* b) const;

private:
  class Mancala_* x_;
};

} // end namespace neu

#endif // GENERIC_H
