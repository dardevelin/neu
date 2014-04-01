#include "Mancala.h"

#include <iostream>

#include "Truth.h"
#include "Index.h"
#include "Count.h"

using namespace std;
using namespace neu;

namespace neu{

class Mancala_{
public:
  Mancala_(Mancala* o)
    : o_(o){
    reset();
  }

  Mancala_(Mancala* o, const Mancala_& c)
    : o_(o){
    
  }

  void validate(const mvar& v){
    
  }

  nvar val(){
    NERROR("method not supported");
  }

  void set(const nvar& v){
    NERROR("method not supported");
  }  

  void reset(){
    for(size_t i = 0; i < 12; ++i){
      slots_[i] = 4;
    }
    store_[0] = 0;
    store_[1] = 0;
  }

  int move(bool first, size_t slot){
    if(slot < 1 || slot > 6){
      NERROR("invalid slot: " + mstr::toStr(slot));
    }

    size_t s = realSlot(first, slot);

    size_t n = slots_[s];

    if(n == 0){
      NERROR("Mancala::move: invalid move");
    }

    slots_[s] = 0;

    bool last;

    for(;;){
      if(s == 0){
        if(first){
          --n;
          ++store_[0];
          last = true;
        }
        s = 11;
      }
      else if(s == 6){
        if(!first){
          --n;
          ++store_[1];
          last = true;
        }
        --s;
      }
      else{
        --s;
      }

      if(n == 0){
        break;
      }

      --n;
      ++slots_[s];
      last = false;

      if(n == 0){
        if(slots_[s] == 1){
          if(s <= 5){
            if(first){
              size_t s2 = 11 - s;
              size_t n2 = slots_[s2];
              slots_[s2] = 0;
              slots_[s] = 0;
              store_[0] += n2 + 1;
            }
          }
          else if(!first){
            size_t s2 = 11 - s;
            size_t n2 = slots_[s2];
            slots_[s2] = 0;
            slots_[s] = 0;
            store_[1] += n2 + 1;
          }
        }

        break;
      }
    }

    size_t ft = 0;

    for(size_t i = 1; i <= 6; ++i){
      size_t s = realSlot(true, i);
      ft += slots_[s];
    }

    if(ft == 0){
      for(size_t i = 1; i <= 6; ++i){
        size_t s = realSlot(false, i);
        store_[1] += slots_[s];
        slots_[s] = 0;
      }

      return 2;
    }

    size_t ot = 0;

    for(size_t i = 1; i <= 6; ++i){
      size_t s = realSlot(false, i);
      ot += slots_[s];
    }    

    if(ot == 0){
      for(size_t i = 1; i <= 6; ++i){
        size_t s = realSlot(true, i);
        store_[0] += slots_[s];
        slots_[s] = 0;
      }

      return 2;
    }

    if(last){
      return 1;
    }

    return 0;
  }

  size_t score(bool first) const{
    if(first){
      return store_[0];
    }
    else{
      return store_[1];
    }
  }

  size_t numPieces(bool first, size_t slot){
    if(slot < 1 || slot > 6){
      NERROR("invalid slot: " + mstr::toStr(slot));
    }

    return slots_[realSlot(first, slot)];
  }

  size_t realSlot(bool first, size_t slot) const{
    if(first){
      return slot - 1;
    }
    else{
      return slot + 5;
    }
  }

  void writeBoard(){
    for(size_t i = 1; i <= 6; ++i){
      size_t s = realSlot(false, i);
      cout << slots_[s] << " ";
    }
    cout << ": " << store_[1] << endl;

    for(size_t i = 6; i > 0; --i){
      size_t s = realSlot(true, i);
      cout << slots_[s] << " ";
    }
    cout << ": " << store_[0] << endl;  
  }

  Count* numPieces(const Index* i) const{
    size_t iv = i->val();

    if(iv > 12){
      NERROR("invalid slot: " + mstr::toStr(iv));
    }

    Count* ret = new Count;

    if(iv == 0){
      *ret = 0;
      return ret;
    }

    size_t s;
    if(iv > 6){
      s = realSlot(false, iv - 6);
    }
    else{
      s = realSlot(true, iv);
    }

    *ret = slots_[s];

    return ret;
  }

  Index* slotOffset(const Index* i, const Count* c) const{
    size_t iv = i->val();

    if(iv > 12){
      NERROR("invalid slot: " + mstr::toStr(iv));
    }

    size_t cv = c->val();

    if(cv > 48){
      NERROR("count too high: " + mstr::toStr(cv));
    }

    while(cv != 0){
      if(iv == 0){
        iv = 12;
      }
      else{
        --iv;
      }
      --cv;
    }

    Index* ret = new Index;
    *ret = iv;

    return ret;
  }

  Count* slotDiff(const Index* a, const Index* b) const{
    size_t av = a->val();

    if(av > 12){
      NERROR("invalid slot: " + mstr::toStr(av));
    }

    size_t bv = b->val();

    if(bv > 12){
      NERROR("invalid slot: " + mstr::toStr(bv));
    }

    size_t d = 0;

    while(av != bv){
      if(av == 0){
        av = 12;
      }
      else{
        --av;
      }
      ++d;
    }

    Count* ret = new Count;
    *ret = d;

    return ret;
  }

  Truth* isMine(const Index* i) const{
    size_t iv = i->val();

    if(iv > 12){
      NERROR("invalid slot: " + mstr::toStr(iv));
    }
    
    Truth* ret = new Truth;

    if(iv > 6){
      *ret = 0.0;
    }
    else{
      *ret = 1.0;
    }

    return ret;
  }

  Truth* truthAnd(const Truth* a, const Truth* b) const{
    Truth* ret = new Truth;

    *ret = a->val() * b->val();

    return ret;
  }

  Truth* truthOr(const Truth* a, const Truth* b) const{
    Truth* ret = new Truth;

    *ret = mvar::max(a->val(), b->val());

    return ret;
  }

  Truth* truthNot(const Truth* t) const{
    Truth* ret = new Truth;

    *ret = 1.0 - t.val();

    return ret;
  }

  Truth* countIndexEqual(const Count* c, const Index* i) const{
    Truth* ret = new Truth;

    if(c->val() == i->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Truth* countIndexGreater(const Count* c, const Index* i) const{
    Truth* ret = new Truth;

    if(c->val() > i->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Truth* countIndexLesser(const Count* c, const Index* i) const{
    Truth* ret = new Truth;

    if(c->val() < i->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Truth* countEqual(const Count* a, const Count* b) const{
    Truth* ret = new Truth;

    if(a->val() == b->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Truth* countGreater(const Count* a, const Count* b) const{
    Truth* ret = new Truth;

    if(a->val() > b->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Truth* countLesser(const Count* a, const Count* b) const{
    Truth* ret = new Truth;

    if(a->val() < b->val()){
      *ret = 1.0;
    }
    else{
      *ret = 0;
    }

    return ret;
  }

  Count* countAdd(const Count* a, const Count* b) const{
    Count* ret = new Count;

    *ret = a->val() + b->val();

    return ret;
  }

  Count* countSub(const Count* a, const Count* b) const{
    Count* ret = new Count;

    *ret = a->val() - b->val();

    return ret;
  }

private:
  Mancala* o_;
  size_t slots_[12];
  size_t store_[2]; 
};
  
} // end namespace Meta

Mancala::Mancala(const Mancala& c)
  : Concept(c){
  x_ = new class Mancala_(this, *c.x_);
}

Mancala::Mancala(PrototypeFlag* p, const nvar& metadata)
  : Concept(p, metadata){
  
}

#ifndef META_GUARD
#include "Mancala_meta.h"
#endif

Mancala* _Mancala_Prototype =
  new Mancala(NConcept::Prototype, Mancala::metadata());

