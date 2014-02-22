/*================================= Neu =================================
 
 Copyright (c) 2013-2014, Andrometa LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
 
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 
 3. Neither the name of the copyright holder nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 =======================================================================*/

#include <neu/nvar.h>

#include <cstdlib>
#include <cmath>
#include <cassert>

#include <neu/global.h>
#include <neu/compress.h>
#include <neu/NObject.h>
#include <neu/NSys.h>

using namespace std;
using namespace neu;

namespace neu{

  const nvar undef;
  const nvar none(nvar::None, nvar::Head());
  const nvec _emptyVec;

  const nvar::CopyFlag* nvar::Copy = 0;
  const nvar::PointerFlag* nvar::PointerType = 0;
  const nvar::SymbolFlag* nvar::SymbolType = 0;
  const nvar::FunctionFlag* nvar::FunctionType = 0;
  const nvar::HeadSequenceFlag* nvar::HeadSequenceType = 0;
  const nvar::HeadMapFlag* nvar::HeadMapType = 0;
  const nvar::SequenceMapFlag* nvar::SequenceMapType = 0;
  const nvar::HeadSequenceMapFlag* nvar::HeadSequenceMapType = 0;

} // end namespace Meta

namespace{

  static const nvar::Type Pack0 =             254;
  static const nvar::Type Pack1 =             253;
  static const nvar::Type Pack2 =             252;
  static const nvar::Type Pack3 =             251;
  static const nvar::Type Pack4 =             250;
  static const nvar::Type Pack5 =             249;
  static const nvar::Type Pack6 =             248;
  static const nvar::Type Pack7 =             247;
  static const nvar::Type Pack8 =             246;
  static const nvar::Type Pack9 =             245;
  static const nvar::Type Pack10 =            244;
  static const nvar::Type PackInt8 =          243;
  static const nvar::Type PackInt16 =         242;
  static const nvar::Type PackInt32 =         241;
  static const nvar::Type PackFloat32 =       240;
  static const nvar::Type PackShortString =   239;
  static const nvar::Type PackLongString =    238;
  static const nvar::Type PackLongSymbol =    237;
  static const nvar::Type PackSmallVector =   236;
  static const nvar::Type PackShortVector =   235;
  static const nvar::Type PackLongVector =    234;
  static const nvar::Type PackShortList =     233;
  static const nvar::Type PackLongList =      232;
  static const nvar::Type PackLongFunction =  231;
  static const nvar::Type PackShortMap =      230;
  static const nvar::Type PackLongMap =       229;
  static const nvar::Type PackShortMultimap = 228;
  static const nvar::Type PackLongMultimap =  227;

} // end namespace

void nvar::streamOutput_(ostream& ostr, bool concise) const{
  switch(t_){
  case None:
    ostr << "none";
    break;
  case Undefined:
    ostr << "undef";
    break;
  case False:
    ostr << "false";
    break;
  case True:
    ostr << "true";
    break;
  case Integer:
    ostr << h_.i;
    break;
  case Rational:
    ostr << *h_.r;
    break;
  case Float:
    ostr << h_.d;
    break;
  case Real:
    ostr << *h_.x;
    break;
  case Symbol:
    ostr << *h_.s;
    break;
  case StringPointer:
  case String:
    ostr << "\"" << *h_.s << "\"";
    break;
  case Binary:
    ostr << "<<BINARY:" << h_.s->length() << ">>";
    break;
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    ostr << h_.o;
    break;
  case RawPointer:
    ostr << h_.p;
    break;
  case Vector:{
    ostr << "[";
    bool first = true;
    streamOutputVector_(ostr, *h_.v, first, concise); 
    ostr << "]";
    break;
  }
  case List:{
    ostr << "(";
    bool first = true;
    streamOutputList_(ostr, *h_.l, first, concise); 
    ostr << ")";
    break;
  }
  case Function:{
    ostr << h_.f->f << "(";

    bool first = true;
    const nvec& args = h_.f->v;
    for(const nvar& a : args){
      if(first){
        first = false;
      }
      else{
        ostr << ",";
      }
      a.streamOutput_(ostr, concise);
    }

    if(h_.f->m){
      streamOutputMap_(ostr, *h_.f->m, first, concise);
    }

    ostr << ")";
    break;
  }
  case HeadSequence:{
    bool first = false;
    if(h_.hs->s->t_ == Vector){
      ostr << "[:";
      h_.hs->h->streamOutput_(ostr, concise);
      streamOutputVector_(ostr, *h_.hs->s->h_.v, first, concise);
      ostr << "]";
    }
    else{
      ostr << "(:";
      h_.hs->h->streamOutput_(ostr, concise);
      streamOutputList_(ostr, *h_.hs->s->h_.l, first, concise);        
      ostr << ")";
    }
    break;
  }
  case Map:{
    stringstream sstr;
    bool first = true;
    bool found = streamOutputMap_(sstr, *h_.m, first, concise); 
    if(found){
      ostr << "[";
      ostr << sstr.str();
      ostr << "]";
    }
    else{
      ostr << "undef";
    }
    break;
  }
  case Multimap:{
    stringstream sstr;
    bool first = true;
    bool found = streamOutputMultimap_(sstr, *h_.mm, first, concise); 
    if(found){
      ostr << "[|";
      ostr << sstr.str();
      ostr << "|]";
    }
    else{
      ostr << "undef";
    }
    break;
  }
  case HeadMap:{
    bool first = false;
    bool mm;

    bool found;

    if(h_.hm->m->t_ == Map){
      mm = false;
      stringstream sstr;
      found = streamOutputMap_(sstr, *h_.hm->m->h_.m, first, concise);
      if(found){
        ostr << "[:";
        h_.hm->h->streamOutput_(ostr, concise);
        ostr << sstr.str();
      }
      else{
        h_.hm->h->streamOutput_(ostr, concise);
      }
    }
    else{
      mm = true;

      stringstream sstr;
      found = streamOutputMultimap_(sstr, *h_.hm->m->h_.mm, first, concise);
      if(found){
        ostr << "[|:";
        h_.hm->h->streamOutput_(ostr, concise);
        ostr << sstr.str();
      }
      else{
        h_.hm->h->streamOutput_(ostr, concise);
      }
    }

    if(found){
      if(mm){
        ostr << "|]";
      }
      else{
        ostr << "]";
      }
    }
    break;
  }
  case SequenceMap:{
    bool first = true;
    bool vec = h_.sm->s->t_ == Vector;
    bool mm =  h_.sm->m->t_ == Multimap;

    if(vec){
      if(mm){
        ostr << "[|";
      }
      else{
        ostr << "[";
      }

      streamOutputVector_(ostr, *h_.sm->s->h_.v, first, concise);
    }
    else{
      if(mm){
        ostr << "(|";
      }
      else{
        ostr << "(";
      }

      streamOutputList_(ostr, *h_.sm->s->h_.l, first, concise); 
    }

    if(mm){
      streamOutputMultimap_(ostr, *h_.sm->m->h_.mm, first, concise);
    }
    else{
      streamOutputMap_(ostr, *h_.sm->m->h_.m, first, concise);
    }

    if(vec){
      if(mm){
        ostr << "|]";
      }
      else{
        ostr << "]";
      }
    }
    else{
      if(mm){
        ostr << "|)";
      }
      else{
        ostr << ")";
      }
    }
    break;
  }
  case HeadSequenceMap:{
    bool first = false;
    bool vec = h_.sm->s->t_ == Vector;
    bool mm =  h_.sm->m->t_ == Multimap;

    if(vec){
      if(mm){
        ostr << "[|:";
      }
      else{
        ostr << "[:";
      }
      h_.hsm->h->streamOutput_(ostr, concise);
      streamOutputVector_(ostr, *h_.sm->s->h_.v, first, concise);
    }
    else{
      if(mm){
        ostr << "(|:";
      }
      else{
        ostr << "(:";
      }

      h_.hsm->h->streamOutput_(ostr, concise);
      streamOutputList_(ostr, *h_.sm->s->h_.l, first, concise); 
    }

    if(mm){
      streamOutputMultimap_(ostr, *h_.sm->m->h_.mm, first, concise);
    }
    else{
      streamOutputMap_(ostr, *h_.sm->m->h_.m, first, concise);
    }

    if(vec){
      if(mm){
        ostr << "|]";
      }
      else{
        ostr << "]";
      }
    }
    else{
      if(mm){
        ostr << "|)";
      }
      else{
        ostr << ")";
      }
    }
    break;
  }
  case Reference:
    h_.ref->v->streamOutput_(ostr, concise);
    break;
  case Pointer:
    h_.vp->streamOutput_(ostr, concise);
    break;
  }
}

nvar::nvar(const nvar& x, const CopyFlag*)
  : t_(x.t_){
  switch(t_){
  case Integer:
    h_.i = x.h_.i;
    break;
  case Float:
    h_.d = x.h_.d;
    break;
  case Rational:
    h_.r = new nrat(*x.h_.r);
    break;
  case Real:
    h_.x = new nreal(*x.h_.x);
    break;
  case String:
  case Binary:
  case Symbol:
    h_.s = new nstr(*x.h_.s);
    break;
  case StringPointer:
    h_.s = x.h_.s;
    break;
  case RawPointer:
    h_.p = x.h_.p;
    break;
  case LocalObject:
    h_.o = x.h_.o->clone();
    break;
  case ObjectPointer:
    h_.o = x.h_.o;
    break;
  case SharedObject:
    h_.o = x.h_.o;
    h_.o->ref();
    break;
  case Vector:{
    h_.v = new nvec;

    nvec& v = *h_.v;
    const nvec& xv = *x.h_.v;
    size_t size = xv.size();
    v.reserve(size);

    for(size_t i = 0; i < size; ++i){
      v.emplace_back(nvar(xv[i], Copy));
    }

    break;
  }
  case List:{
    h_.l = new nlist;
    nlist& l = *h_.l;
    const nlist& xl = *x.h_.l;

    for(auto& itr : xl){
      l.emplace_back(nvar(itr, Copy));
    }

    break;
  }
  case Function:{
    h_.f = new CFunction(x.h_.f->f);
    
    nvec& v = h_.f->v;
    const nvec& xv = x.h_.f->v;
    size_t size = xv.size();
    v.reserve(size);    

    for(size_t i = 0; i < size; ++i){
      v.emplace_back(nvar(xv[i], Copy));
    }

    if(x.h_.f->m){
      nmap* m = new nmap;
      h_.f->m = m;

      const nmap& xm = *x.h_.f->m;
      
      for(auto& itr : xm){
        m->emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
      }
    }

    break;
  }
  case HeadSequence:
    h_.hs = 
      new CHeadSequence(new nvar(*x.h_.hs->h, Copy),
                        new nvar(*x.h_.hs->s, Copy));
    break;
  case Map:{
    h_.m = new nmap;
    nmap& m = *h_.m;
    const nmap& xm = *x.h_.m;

    for(auto& itr : xm){
      m.emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
    }

    break;
  }
  case Multimap:{
    h_.mm = new nmmap;
    nmmap& mm = *h_.mm;
    const nmmap& xmm = *x.h_.mm;

    for(auto& itr : xmm){
      mm.emplace(nvar(itr.first, Copy), nvar(itr.second, Copy));
    }

    break;
  }
  case HeadMap:
    h_.hm = 
      new CHeadMap(new nvar(*x.h_.hm->h, Copy),
                   new nvar(*x.h_.hm->m, Copy));
    break;
  case SequenceMap:
    h_.sm = 
      new CSequenceMap(new nvar(*x.h_.sm->s, Copy),
                       new nvar(*x.h_.sm->m, Copy));
    break;
  case HeadSequenceMap:
    h_.hsm = 
      new CHeadSequenceMap(new nvar(*x.h_.hsm->h, Copy),
                           new nvar(*x.h_.hsm->s, Copy),
                           new nvar(*x.h_.hsm->m, Copy));
    break;
  case Reference:
    h_.ref = new CReference(new nvar(*x.h_.ref->v, Copy));
    break;
  case Pointer:
    h_.vp = h_.vp;
    break;
  }
}

nvar& nvar::operator=(const nvar& x){
  switch(t_){
  case None:
    switch(x.t_){
    case None:
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Undefined:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case False:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case True:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Integer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Integer:
      h_.i = x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Rational:
    switch(x.t_){
    case None:
      delete h_.r;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.r;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.r;
      t_ = False;
      return *this;
    case True:
      delete h_.r;
      t_ = True;
      return *this;
    case Rational:
      *h_.r = *x.h_.r;
      return *this;
    case Real:
      delete h_.r;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.r;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.r;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.r;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.r;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.r;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.r;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.r;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.r;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.r;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.r;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.r;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.r;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.r;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.r;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Float:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Float:
      h_.d = x.h_.d;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this; 
  case Real:
    switch(x.t_){
    case None:
      delete h_.x;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.x;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.x;
      t_ = False;
      return *this;
    case True:
      delete h_.x;
      t_ = True;
      return *this;
    case Real:
      *h_.x = *x.h_.x;
      return *this;
    case Rational:
      delete h_.x;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.x;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.x;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.x;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.x;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.x;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.x;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.x;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.x;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.x;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.x;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.x;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.x;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.x;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.x;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case String:
  case Binary:
  case Symbol:
    switch(x.t_){
    case None:
      delete h_.s;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.s;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.s;
      t_ = False;
      return *this;
    case True:
      delete h_.s;
      t_ = True;
      return *this;
    case Rational:
      delete h_.s;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.s;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      *h_.s = *x.h_.s;
      return *this;
    case LocalObject:
      delete h_.s;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.s;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.s;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.s;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.s;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.s;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.s;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.s;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.s;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.s;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.s;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.s;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.s;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case StringPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case RawPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case RawPointer:
      h_.p = x.h_.p;
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case ObjectPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case ObjectPointer:
      h_.o = x.h_.o;
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Vector:
    switch(x.t_){
    case None:
      delete h_.v;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.v;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.v;
      t_ = False;
      return *this;
    case True:
      delete h_.v;
      t_ = True;
      return *this;
    case Rational:
      delete h_.v;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.v;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.v;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.v;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.v;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      *h_.v = *x.h_.v;
      return *this;
    case List:
      delete h_.v;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.v;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.v;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.v;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.v;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.v;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.v;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.v;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.v;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.v;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case List:
    switch(x.t_){
    case None:
      delete h_.l;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.l;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.l;
      t_ = False;
      return *this;
    case True:
      delete h_.l;
      t_ = True;
      return *this;
    case Rational:
      delete h_.l;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.l;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.l;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.l;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.l;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.l;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      *h_.l = *x.h_.l;
      return *this;
    case Function:
      delete h_.l;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.l;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.l;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.l;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.l;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.l;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.l;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.l;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.l;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Function:
    switch(x.t_){
    case None:
      delete h_.f;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.f;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.f;
      t_ = False;
      return *this;
    case True:
      delete h_.f;
      t_ = True;
      return *this;
    case Rational:
      delete h_.f;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.f;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.f;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.f;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.f;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.f;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.f;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      h_.f->f = x.h_.f->f;
      h_.f->v = x.h_.f->v;
      return *this;
    case HeadSequence:
      delete h_.f;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.f;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.f;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.f;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.f;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.f;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.f;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.f;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Map:
    switch(x.t_){
    case None:
      delete h_.m;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.m;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.m;
      t_ = False;
      return *this;
    case True:
      delete h_.m;
      t_ = True;
      return *this;
    case Rational:
      delete h_.m;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.m;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.m;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.m;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.m;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.m;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.l;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.m;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.m;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      *h_.m = *x.h_.m;
      return *this;
    case Multimap:
      delete h_.m;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.m;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.m;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.m;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.m;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.m;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Multimap:
    switch(x.t_){
    case None:
      delete h_.mm;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.mm;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.mm;
      t_ = False;
      return *this;
    case True:
      delete h_.mm;
      t_ = True;
      return *this;
    case Rational:
      delete h_.mm;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.mm;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.mm;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.mm;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.mm;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.mm;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.l;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.mm;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.mm;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.mm;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      *h_.mm = *x.h_.mm;
      return *this;
    case HeadMap:
      delete h_.mm;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.mm;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.mm;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.mm;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.mm;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Reference:
    switch(x.t_){
    case None:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = None;
      return *this;
    case Undefined:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Undefined;
      return *this;
    case False:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = False;
      return *this;
    case True:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = True;
      return *this;
    case Rational:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      if(h_.ref->deref()){
        delete h_.ref;
      }
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
 case Pointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    case Pointer:
      h_.vp = x.h_.vp;
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case LocalObject:
    delete h_.o;
    break;
  case SharedObject:
    if(h_.o->deref()){
      delete h_.o;
    }
    break;
  case HeadSequence:
    h_.hs->dealloc();
    delete h_.hs;
    break;
  case HeadMap:
    h_.hm->dealloc();
    delete h_.hm;
    break;
  case SequenceMap:
    h_.sm->dealloc();
    delete h_.sm;
    break;
  case HeadSequenceMap:
    h_.hsm->dealloc();
    delete h_.hsm;
    break;
  }

  t_ = x.t_;

  switch(t_){
  case None:
  case Undefined:
  case False:
  case True:
    return *this;
  case Rational:
    h_.r = new nrat(*x.h_.r);
    return *this;
  case Real:
    h_.x = new nreal(*x.h_.x);
    return *this;
  case String:
  case Binary:
  case Symbol:
    h_.s = new nstr(*x.h_.s);
    return *this;
  case LocalObject:
    h_.o = x.h_.o->clone();
    return *this;
  case SharedObject:
    h_.o = x.h_.o;
    h_.o->ref();
    return *this;
  case Vector:
    h_.v = new nvec(*x.h_.v);
    return *this;
  case List:
    h_.l = new nlist(*x.h_.l);
    return *this;
  case Function:
    h_.f = x.h_.f->clone();
    return *this;
  case HeadSequence:
    h_.hs = x.h_.hs->clone();
    return *this;
  case Map:
    h_.m = new nmap(*x.h_.m);
    return *this;
  case Multimap:
    h_.mm = new nmmap(*x.h_.mm);
    return *this;
  case HeadMap:
    h_.hm = x.h_.hm->clone();
    return *this;
  case SequenceMap:
    h_.sm = x.h_.sm->clone();
    return *this;
  case HeadSequenceMap:
    h_.hsm = x.h_.hsm->clone();
    return *this;
  case Reference:
    h_.ref = x.h_.ref;
    h_.ref->ref();
    return *this;
  default:
    h_.i = x.h_.i;
    return *this;
  }

  return *this;
}

nvar& nvar::set(const nvar& x){
  switch(t_){
  case None:
    switch(x.t_){
    case None:
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Undefined:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
 case False:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case True:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Integer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Integer:
      h_.i = x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Rational:
    switch(x.t_){
    case None:
      delete h_.r;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.r;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.r;
      t_ = False;
      return *this;
    case True:
      delete h_.r;
      t_ = True;
      return *this;
    case Rational:
      *h_.r = *x.h_.r;
      return *this;
    case Real:
      delete h_.r;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.r;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.r;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.r;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.r;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.r;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.r;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.r;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.r;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.r;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.r;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.r;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.r;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.r;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.r;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Float:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Float:
      h_.d = x.h_.d;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this; 
  case Real:
    switch(x.t_){
    case None:
      delete h_.x;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.x;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.x;
      t_ = False;
      return *this;
    case True:
      delete h_.x;
      t_ = True;
      return *this;
    case Real:
      *h_.x = *x.h_.x;
      return *this;
    case Rational:
      delete h_.x;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.x;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.x;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.x;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.x;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.x;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.x;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.x;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.x;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.x;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.x;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.x;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.x;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.x;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.x;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case String:
  case Binary:
    switch(x.t_){
    case None:
      delete h_.s;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.s;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.s;
      t_ = False;
      return *this;
    case True:
      delete h_.s;
      t_ = True;
      return *this;
    case Rational:
      delete h_.s;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.s;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      *h_.s = *x.h_.s;
      return *this;
    case LocalObject:
      delete h_.s;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.s;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.s;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.s;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.s;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.s;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.s;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.s;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.s;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.s;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.s;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.s;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.s;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case StringPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case RawPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case RawPointer:
      h_.p = x.h_.p;
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case ObjectPointer:
    switch(x.t_){
    case None:
      t_ = None;
      return *this;
    case Undefined:
      t_ = Undefined;
      return *this;
    case False:
      t_ = False;
      return *this;
    case True:
      t_ = True;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case ObjectPointer:
      h_.o = x.h_.o;
      return *this;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Vector:
    switch(x.t_){
    case None:
      delete h_.v;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.v;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.v;
      t_ = False;
      return *this;
    case True:
      delete h_.v;
      t_ = True;
      return *this;
    case Rational:
      delete h_.v;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.v;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.v;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.v;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.v;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      *h_.v = *x.h_.v;
      return *this;
    case List:
      delete h_.v;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.v;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.v;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.v;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.v;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.v;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.v;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.v;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.v;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.v;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case List:
    switch(x.t_){
    case None:
      delete h_.l;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.l;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.l;
      t_ = False;
      return *this;
    case True:
      delete h_.l;
      t_ = True;
      return *this;
    case Rational:
      delete h_.l;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.l;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.l;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.l;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.l;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.l;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      *h_.l = *x.h_.l;
      return *this;
    case Function:
      delete h_.l;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.l;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.l;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      delete h_.l;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.l;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.l;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.l;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.l;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.l;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Set");
    f->v.push_back(*this);
    f->v.push_back(nvar(x, Copy));
    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Map:
    switch(x.t_){
    case None:
      delete h_.m;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.m;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.m;
      t_ = False;
      return *this;
    case True:
      delete h_.m;
      t_ = True;
      return *this;
    case Rational:
      delete h_.m;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.m;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.m;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.m;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.m;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.m;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.l;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.m;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.m;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      *h_.m = *x.h_.m;
      return *this;
    case Multimap:
      delete h_.m;
      t_ = Multimap;
      h_.mm = new nmmap(*x.h_.mm);
      return *this;
    case HeadMap:
      delete h_.m;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.m;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.m;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.m;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.m;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Multimap:
    switch(x.t_){
    case None:
      delete h_.mm;
      t_ = None;
      return *this;
    case Undefined:
      delete h_.mm;
      t_ = Undefined;
      return *this;
    case False:
      delete h_.mm;
      t_ = False;
      return *this;
    case True:
      delete h_.mm;
      t_ = True;
      return *this;
    case Rational:
      delete h_.mm;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      return *this;
    case Real:
      delete h_.mm;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      return *this;
    case String:
    case Binary:
    case Symbol:
      delete h_.mm;
      t_ = x.t_;
      h_.s = new nstr(*x.h_.s);
      return *this;
    case LocalObject:
      delete h_.mm;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      return *this;
    case SharedObject:
      delete h_.mm;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      return *this;
    case Vector:
      delete h_.mm;
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      return *this;
    case List:
      delete h_.l;
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      return *this;
    case Function:
      delete h_.mm;
      t_ = Function;
      h_.f = x.h_.f->clone();
      return *this;
    case HeadSequence:
      delete h_.mm;
      t_ = HeadSequence;
      h_.hs = x.h_.hs->clone();
      return *this;
    case Map:
      delete h_.mm;
      t_ = Map;
      h_.m = new nmap(*x.h_.m);
      return *this;
    case Multimap:
      *h_.mm = *x.h_.mm;
      return *this;
    case HeadMap:
      delete h_.mm;
      t_ = HeadMap;
      h_.hm = x.h_.hm->clone();
      return *this;
    case SequenceMap:
      delete h_.mm;
      t_ = SequenceMap;
      h_.sm = x.h_.sm->clone();
      return *this;
    case HeadSequenceMap:
      delete h_.mm;
      t_ = HeadSequenceMap;
      h_.hsm = x.h_.hsm->clone();
      return *this;
    case Reference:
      delete h_.mm;
      t_ = Reference;
      h_.ref = x.h_.ref;
      h_.ref->ref();
      return *this;
    default:
      delete h_.mm;
      t_ = x.t_;
      h_.i = x.h_.i;
      return *this;
    }
    return *this;
  case Reference:
    return h_.ref->v->set(x);
  case Pointer:
    return h_.vp->set(x);
  case LocalObject:
    delete h_.o;
    break;
  case SharedObject:
    if(h_.o->deref()){
      delete h_.o;
    }
    break;
  case HeadSequence:
    h_.hs->dealloc();
    delete h_.hs;
    break;
  case HeadMap:
    h_.hm->dealloc();
    delete h_.hm;
    break;
  case SequenceMap:
    h_.sm->dealloc();
    delete h_.sm;
    break;
  case HeadSequenceMap:
    h_.hsm->dealloc();
    delete h_.hsm;
    break;
  }

  t_ = x.t_;

  switch(t_){
  case None:
  case Undefined:
  case False:
  case True:
    return *this;
  case Rational:
    h_.r = new nrat(*x.h_.r);
    return *this;
  case Real:
    h_.x = new nreal(*x.h_.x);
    return *this;
  case String:
  case Binary:
  case Symbol:
    h_.s = new nstr(*x.h_.s);
    return *this;
  case LocalObject:
    h_.o = x.h_.o->clone();
    return *this;
  case SharedObject:
    h_.o = x.h_.o;
    h_.o->ref();
    return *this;
  case Vector:
    h_.v = new nvec(*x.h_.v);
    return *this;
  case List:
    h_.l = new nlist(*x.h_.l);
    return *this;
  case Function:
    h_.f = x.h_.f->clone();
    return *this;
  case HeadSequence:
    h_.hs = x.h_.hs->clone();
    return *this;
  case Map:
    h_.m = new nmap(*x.h_.m);
    return *this;
  case Multimap:
    h_.mm = new nmmap(*x.h_.mm);
    return *this;
  case HeadMap:
    h_.hm = x.h_.hm->clone();
    return *this;
  case SequenceMap:
    h_.sm = x.h_.sm->clone();
    return *this;
  case HeadSequenceMap:
    h_.hsm = x.h_.hsm->clone();
    return *this;
  case Reference:
    h_.ref = x.h_.ref;
    h_.ref->ref();
    return *this;
  default:
    h_.i = x.h_.i;
    return *this;
  }

  return *this;
}

nvar& nvar::operator+=(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.i += x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(h_.i);
      *h_.r += *x.h_.r;
      if(h_.r->denominator() == 1){
        t_ = Integer;
        nrat* r = h_.r;
        h_.i = r->numerator();
        delete r;
        return *this;
      }
      return *this;
    case Float:
      t_ = Float;
      h_.d = h_.i;
      h_.d += x.h_.d;
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(h_.i);
      *h_.x += *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(h_.i);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v += *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v += *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v += *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v += *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.r += x.h_.i;
      return *this;
    case Rational:
      *h_.r += *x.h_.r;
      if(h_.r->denominator() == 1){
        nrat* r = h_.r;
        h_.i = h_.r->numerator();
        t_ = Integer;
        delete r;
      }
      return *this;
    case Float:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x.h_.d;
      return *this;
    }
    case Real:{
      nreal* y = new nreal(*h_.r);
      delete h_.r;
      t_ = Real;
      *y += *x.h_.x;
      h_.x = y;
      return *this;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(*h_.r);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.r);
      *v += *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
      *v += *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
      *v += *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *v += *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.d += x.h_.i;
      return *this;
    case Rational:
      h_.d += x.h_.r->toDouble();
      return *this;
    case Float:
      h_.d += x.h_.d;
      return *this;
    case Real:
      h_.d += x.h_.x->toDouble();
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(h_.d);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.d);
      *v += *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
      *v += *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
      *v += *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
      *v += *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.x += x.h_.i;
      return *this;
    case Rational:
      *h_.x += *x.h_.r;
      return *this;
    case Float:{
      double d = h_.x->toDouble();
      d += x.h_.d;
      delete h_.x;
      h_.d = d;
      t_ = Float;
    }
    case Real:
      *h_.x += *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.push_back(*h_.x);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.x);
      *v += *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
      *v += *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
      *v += *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *v += *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case String:
  case StringPointer:
    switch(x.t_){
    case String:
    case StringPointer:
      *h_.s += *x.h_.s;
      return *this;
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Add");
    f->v.emplace_back(new nvar(*this, Copy));
    f->v.emplace_back(nvar(x, Copy));
    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.v += x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v += *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.v += *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:
      *h_.v += *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.v += *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hs->s += x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hs->s += *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hs->s += *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:
      *h_.hs->s += *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hs->s += *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h += x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.sm->s += x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.sm->s += *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.sm->s += *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:
      *h_.sm->s += *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.sm->s += *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hsm->s += x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Add");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      *h_.hsm->s += *x.h_.v;
      return *this;
    }
    case HeadSequence:
      *h_.hsm->s += *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this += *x.h_.hm->h;
    case SequenceMap:
      *h_.hsm->s += *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s += *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this += *x.h_.ref->v;
    case Pointer:
      return *this += *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    *h_.ref->v += x;
    return *this;
  case Pointer:
    *h_.vp += x;
    return *this;
  default:
    cout << "type is: " << int(t_) << endl;
    NERROR("left operand is invalid");
  }
}

nvar nvar::operator+(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.i + x.h_.i;
    case Rational:{
      Head h;
      h.r = new nrat(h_.i);
      *h.r += *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.i + x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(h_.i);
      *h.x += *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v += *x.h_.v;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v += *x.h_.hs->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v += *x.h_.sm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v += *x.h_.hsm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this + *x.h_.ref->v;        
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.r = new nrat(*h_.r);
      *h.r += x.h_.i;
      return nvar(Rational, h);
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r += *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = h_.r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.r->toDouble() + x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.r);
      *h.x += *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      *h.v += *x.h_.v;
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      *h.v += *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      *h.v += *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *h.v += *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.d + x.h_.i;
    case Rational:
      return h_.d + x.h_.r->toDouble();
    case Float:
      return h_.d + x.h_.d;
    case Real:
      return h_.d + x.h_.x->toDouble();
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      *h.v += *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      *h.v += *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      *h.v += *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      *h.v += *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.x = new nreal(*h_.x);
      *h.x += x.h_.i;
      return nvar(Real, h);
    case Rational:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x += *x.h_.r;
      return nvar(Real, h);
    }
    case Float:
      return h_.x->toDouble() + x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x += *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      *h.v += *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      *h.v += *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      *h.v += *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *h.v += *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case String:
  case StringPointer:
    switch(x.t_){
    case String:
    case StringPointer:{
      Head h;
      h.s = new nstr(*h_.s);
      *h.s += *x.h_.s;
      return nvar(String, h);
    }
    case HeadMap:
      return *this + *x.h_.hm->h;
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Add");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.v + x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v + *x.h_.v;
    case HeadSequence:
      return *h_.v + *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:
      return *h_.v + *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v + *x.h_.hsm->s;
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s + x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s + *x.h_.v;
    case HeadSequence:
      return *h_.hs->s + *x.h_.hs->s;
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s + *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s + *x.h_.hsm->s;
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h + x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s + x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s + *x.h_.v;
    case HeadSequence:
      return *h_.sm->s + *x.h_.hs->s;
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s + *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s + *x.h_.hsm->s;
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s + x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Add");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s + *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s + *x.h_.hs->s;
    case HeadMap:
      return *this + *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s + *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s + *x.h_.hsm->s;
    case Reference:
      return *this + *x.h_.ref->v;
    case Pointer:
      return *this + *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v + x;
  case Pointer:
    return *h_.vp + x;
  default:
    NERROR("left operand is invalid");
  }  
}

nvar& nvar::operator-=(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.i -= x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(h_.i);
      *h_.r -= *x.h_.r;
      if(h_.r->denominator() == 1){
        t_ = Integer;
        nrat* r = h_.r;
        h_.i = r->numerator();
        delete r;
        return *this;
      }
      return *this;
    case Float:
      t_ = Float;
      h_.d = h_.i;
      h_.d -= x.h_.d;
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(h_.i);
      *h_.x -= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(h_.i);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v -= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v -= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v -= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v -= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.r -= x.h_.i;
      return *this;
    case Rational:
      *h_.r -= *x.h_.r;
      if(h_.r->denominator() == 1){
        nrat* r = h_.r;
        h_.i = h_.r->numerator();
        t_ = Integer;
        delete r;
      }
      return *this;
    case Float:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x.h_.d;
      return *this;
    }
    case Real:{
      nreal* y = new nreal(*h_.r);
      delete h_.r;
      t_ = Real;
      *y -= *x.h_.x;
      h_.x = y;
      return *this;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(*h_.r);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.r);
      *v -= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
      *v -= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
      *v -= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *v -= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.d -= x.h_.i;
      return *this;
    case Rational:
      h_.d -= x.h_.r->toDouble();
      return *this;
    case Float:
      h_.d -= x.h_.d;
      return *this;
    case Real:
      h_.d -= x.h_.x->toDouble();
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(h_.d);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.d);
      *v -= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
      *v -= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
      *v -= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
      *v -= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.x -= x.h_.i;
      return *this;
    case Rational:
      *h_.x -= *x.h_.r;
      return *this;
    case Float:{
      double d = h_.x->toDouble();
      d -= x.h_.d;
      delete h_.x;
      h_.d = d;
      t_ = Float;
    }
    case Real:
      *h_.x -= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.push_back(*h_.x);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.x);
      *v -= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
      *v -= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
      *v -= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *v -= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Sub");
    f->v.emplace_back(new nvar(*this, Copy));
    f->v.emplace_back(nvar(x, Copy));

    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.v -= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v -= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.v -= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:
      *h_.v -= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.v -= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hs->s -= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      *h_.hs->s -= *x.h_.v;
      return *this;
    }
    case HeadSequence:
      *h_.hs->s -= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:
      *h_.hs->s -= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hs->s -= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h -= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.sm->s -= x;
      return *this;
    case Vector:
      *h_.sm->s -= *x.h_.v;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case HeadSequence:
      *h_.sm->s -= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:
      *h_.sm->s -= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.sm->s -= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hsm->s -= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Sub");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hsm->s -= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hsm->s -= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this -= *x.h_.hm->h;
    case SequenceMap:
      *h_.hsm->s -= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s -= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this -= *x.h_.ref->v;
    case Pointer:
      return *this -= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    *h_.ref->v -= x;
    return *this;
  case Pointer:
    *h_.vp -= x;
    return *this;
  default:
    NERROR("left operand is invalid");
  }
}

nvar nvar::operator-(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.i - x.h_.i;
    case Rational:{
      Head h;
      h.r = new nrat(h_.i);
      *h.r -= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.i - x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(h_.i);
      *h.x -= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v -= *x.h_.v;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v -= *x.h_.hs->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v -= *x.h_.sm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v -= *x.h_.hsm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.r = new nrat(*h_.r);
      h.r -= x.h_.i;
      return nvar(Rational, h);
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r -= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = h_.r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.r->toDouble() - x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.r);
      *h.x -= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      *h.v -= *x.h_.v;
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      *h.v -= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      *h.v -= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *h.v -= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.d - x.h_.i;
    case Rational:
      return h_.d - x.h_.r->toDouble();
    case Float:
      return h_.d - x.h_.d;
    case Real:
      return h_.d - x.h_.x->toDouble();
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      *h.v -= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      *h.v -= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      *h.v -= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      *h.v -= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.x = new nreal(*h_.x);
      h.x -= x.h_.i;
      return nvar(Real, h);
    case Rational:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x -= *x.h_.r;
      return nvar(Real, h);
    }
    case Float:
      return h_.x->toDouble() - x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x -= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      *h.v -= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      *h.v -= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      *h.v -= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *h.v -= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Sub");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.v - x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v - *x.h_.v;
    case HeadSequence:
      return *h_.v - *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:
      return *h_.v - *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v - *x.h_.hsm->s;
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s - x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s - *x.h_.v;
    case HeadSequence:
      return *h_.hs->s - *x.h_.hs->s;
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s - *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s - *x.h_.hsm->s;
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h - x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s - x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s - *x.h_.v;
    case HeadSequence:
      return *h_.sm->s - *x.h_.hs->s;
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s - *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s - *x.h_.hsm->s;
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s - x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Sub");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s - *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s - *x.h_.hs->s;
    case HeadMap:
      return *this - *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s - *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s - *x.h_.hsm->s;
    case Reference:
      return *this - *x.h_.ref->v;
    case Pointer:
      return *this - *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v - x;
  case Pointer:
    return *h_.vp - x;
  default:
    NERROR("left operand is invalid");
  }  
}

nvar& nvar::operator*=(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.i *= x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(h_.i);
      *h_.r *= *x.h_.r;
      if(h_.r->denominator() == 1){
        t_ = Integer;
        nrat* r = h_.r;
        h_.i = r->numerator();
        delete r;
        return *this;
      }
      return *this;
    case Float:
      t_ = Float;
      h_.d = h_.i;
      h_.d *= x.h_.d;
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(h_.i);
      *h_.x *= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(h_.i);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v *= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v *= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v *= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v *= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.r *= x.h_.i;
      return *this;
    case Rational:
      *h_.r *= *x.h_.r;
      if(h_.r->denominator() == 1){
        nrat* r = h_.r;
        h_.i = h_.r->numerator();
        t_ = Integer;
        delete r;
      }
      return *this;
    case Float:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x.h_.d;
      return *this;
    }
    case Real:{
      nreal* y = new nreal(*h_.r);
      delete h_.r;
      t_ = Real;
      *y *= *x.h_.x;
      h_.x = y;
      return *this;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(*h_.r);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.r);
      *v *= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
      *v *= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
      *v *= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *v *= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.d *= x.h_.i;
      return *this;
    case Rational:
      h_.d *= x.h_.r->toDouble();
      return *this;
    case Float:
      h_.d *= x.h_.d;
      return *this;
    case Real:
      h_.d *= x.h_.x->toDouble();
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(h_.d);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.d);
      *v *= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
      *v *= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
      *v *= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
      *v *= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.x *= x.h_.i;
      return *this;
    case Rational:
      *h_.x *= *x.h_.r;
      return *this;
    case Float:{
      double d = h_.x->toDouble();
      d *= x.h_.d;
      delete h_.x;
      h_.d = d;
      t_ = Float;
    }
    case Real:
      *h_.x *= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.push_back(*h_.x);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.x);
      *v *= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
      *v *= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
      *v *= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *v *= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Mul");
    f->v.emplace_back(new nvar(*this, Copy));
    f->v.emplace_back(nvar(x, Copy));

    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.v *= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v *= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.v *= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:
      *h_.v *= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.v *= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hs->s *= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hs->s *= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hs->s *= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:
      *h_.hs->s *= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hs->s *= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h *= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.sm->s *= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.sm->s *= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.sm->s *= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:
      *h_.sm->s *= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.sm->s *= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hsm->s *= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mul");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hsm->s *= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hsm->s *= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this *= *x.h_.hm->h;
    case SequenceMap:
      *h_.hsm->s *= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s *= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this *= *x.h_.ref->v;
    case Pointer:
      return *this *= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    *h_.ref->v *= x;
    return *this;
  case Pointer:
    *h_.vp *= x;
    return *this;
  default:
    NERROR("left operand is invalid");
  }
}

nvar nvar::operator*(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.i * x.h_.i;
    case Rational:{
      Head h;
      h.r = new nrat(h_.i);
      *h.r *= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.i * x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(h_.i);
      *h.x *= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v *= *x.h_.v;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v *= *x.h_.hs->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v *= *x.h_.sm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v *= *x.h_.hsm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      cout << "x.t_ is: " << int(x.t_) << endl;
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.r = new nrat(*h_.r);
      *h.r *= x.h_.i;
      return nvar(Rational, h);
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r *= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = h_.r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.r->toDouble() * x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.r);
      *h.x *= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      *h.v *= *x.h_.v;
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      *h.v *= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      *h.v *= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *h.v *= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.d * x.h_.i;
    case Rational:
      return h_.d * x.h_.r->toDouble();
    case Float:
      return h_.d * x.h_.d;
    case Real:
      return h_.d * x.h_.x->toDouble();
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      *h.v *= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      *h.v *= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      *h.v *= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      *h.v *= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.x = new nreal(*h_.x);
      *h.x *= x.h_.i;
      return nvar(Real, h);
    case Rational:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x *= *x.h_.r;
      return nvar(Real, h);
    }
    case Float:
      return h_.x->toDouble() * x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x *= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      *h.v *= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      *h.v *= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      *h.v *= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *h.v *= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Mul");
    h.f->v.push_back(new nvar(*this, Copy));
    h.f->v.push_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.v * x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v * *x.h_.v;
    case HeadSequence:
      return *h_.v * *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:
      return *h_.v * *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v * *x.h_.hsm->s;
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s * x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s * *x.h_.v;
    case HeadSequence:
      return *h_.hs->s * *x.h_.hs->s;
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s * *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s * *x.h_.hsm->s;
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h * x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s = x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s * *x.h_.v;
    case HeadSequence:
      return *h_.sm->s * *x.h_.hs->s;
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s * *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s * *x.h_.hsm->s;
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s * x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mul");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s * *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s * *x.h_.hs->s;
    case HeadMap:
      return *this * *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s * *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s * *x.h_.hsm->s;
    case Reference:
      return *this * *x.h_.ref->v;
    case Pointer:
      return *this * *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v * x;
  case Pointer:
    return *h_.vp * x;
  default:
    NERROR("left operand is invalid");
  }  
}

nvar& nvar::operator/=(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.i /= x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(h_.i);
      *h_.r /= *x.h_.r;
      if(h_.r->denominator() == 1){
        t_ = Integer;
        nrat* r = h_.r;
        h_.i = r->numerator();
        delete r;
        return *this;
      }
      return *this;
    case Float:
      t_ = Float;
      h_.d = h_.i;
      h_.d /= x.h_.d;
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(h_.i);
      *h_.x /= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(h_.i);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v /= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v /= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v /= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v /= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.r /= x.h_.i;
      return *this;
    case Rational:
      *h_.r /= *x.h_.r;
      if(h_.r->denominator() == 1){
        nrat* r = h_.r;
        h_.i = h_.r->numerator();
        t_ = Integer;
        delete r;
      }
      return *this;
    case Float:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = d + x.h_.d;
      return *this;
    }
    case Real:{
      nreal* y = new nreal(*h_.r);
      delete h_.r;
      t_ = Real;
      *y /= *x.h_.x;
      h_.x = y;
      return *this;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(*h_.r);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.r);
      *v /= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
      *v /= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
      *v /= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *v /= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.d /= x.h_.i;
      return *this;
    case Rational:
      h_.d /= x.h_.r->toDouble();
      return *this;
    case Float:
      h_.d /= x.h_.d;
      return *this;
    case Real:
      h_.d /= x.h_.x->toDouble();
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(h_.d);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.d);
      *v /= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
      *v /= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
      *v /= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
      *v /= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.x /= x.h_.i;
      return *this;
    case Rational:
      *h_.x /= *x.h_.r;
      return *this;
    case Float:{
      double d = h_.x->toDouble();
      d /= x.h_.d;
      delete h_.x;
      h_.d = d;
      t_ = Float;
    }
    case Real:
      *h_.x /= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.push_back(*h_.x);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.x);
      *v /= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
      *v /= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
      *v /= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *v /= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Div");
    f->v.emplace_back(new nvar(*this, Copy));
    f->v.emplace_back(nvar(x, Copy));

    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.v /= x;
      return *this;
    case Vector:
      *h_.v /= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.v /= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:
      *h_.v /= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.v /= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hs->s /= x;
      return *this;
    case Vector:
      *h_.hs->s /= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hs->s /= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:
      *h_.hs->s /= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hs->s /= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h /= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.sm->s /= x;
      return *this;
    case Vector:
      *h_.sm->s /= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.sm->s /= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:
      *h_.sm->s /= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.sm->s /= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hsm->s /= x;
      return *this;
    case Vector:
      *h_.hsm->s /= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hsm->s /= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this /= *x.h_.hm->h;
    case SequenceMap:
      *h_.hsm->s /= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s /= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this /= *x.h_.ref->v;
    case Pointer:
      return *this /= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    *h_.ref->v /= x;
    return *this;
  case Pointer:
    *h_.vp /= x;
    return *this;
  default:
    NERROR("left operand is invalid");
  }
}

nvar nvar::operator/(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.i / x.h_.i;
    case Rational:{
      Head h;
      h.r = new nrat(h_.i);
      *h.r /= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.i / x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(h_.i);
      *h.x /= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v /= *x.h_.v;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v /= *x.h_.hs->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v /= *x.h_.sm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v /= *x.h_.hsm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.r = new nrat(*h_.r);
      *h.r /= x.h_.i;
      return nvar(Rational, h);
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r /= *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = h_.r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return h_.r->toDouble() / x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.r);
      *h.x /= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      *h.v /= *x.h_.v;
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      *h.v /= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      *h.v /= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *h.v /= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.d / x.h_.i;
    case Rational:
      return h_.d / x.h_.r->toDouble();
    case Float:
      return h_.d / x.h_.d;
    case Real:
      return h_.d / x.h_.x->toDouble();
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      *h.v /= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      *h.v /= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      *h.v /= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      *h.v /= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.x = new nreal(*h_.x);
      *h.x /= x.h_.i;
      return nvar(Real, h);
    case Rational:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x /= *x.h_.r;
      return nvar(Real, h);
    }
    case Float:
      return h_.x->toDouble() / x.h_.d;
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x /= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Div");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      *h.v /= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      *h.v /= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      *h.v /= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *h.v /= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Div");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.v / x;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      Head h;
      h.f = f;
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v / *x.h_.v;
    case HeadSequence:
      return *h_.v / *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:
      return *h_.v / *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v / *x.h_.hsm->s;
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s / x;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      Head h;
      h.f = f;
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s / *x.h_.v;
    case HeadSequence:
      return *h_.hs->s / *x.h_.hs->s;
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s / *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s / *x.h_.hsm->s;
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h / x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s / x;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      Head h;
      h.f = f;
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s / *x.h_.v;
    case HeadSequence:
      return *h_.sm->s / *x.h_.hs->s;
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s / *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s / *x.h_.hsm->s;
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s / x;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Div");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      Head h;
      h.f = f;
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s / *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s / *x.h_.hs->s;
    case HeadMap:
      return *this / *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s / *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s / *x.h_.hsm->s;
    case Reference:
      return *this / *x.h_.ref->v;
    case Pointer:
      return *this / *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v / x;
  case Pointer:
    return *h_.vp / x;
  default:
    NERROR("left operand is invalid");
  }  
}

nvar& nvar::operator%=(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.i %= x.h_.i;
      return *this;
    case Rational:
      t_ = Rational;
      h_.r = new nrat(h_.i);
      *h_.r = *h_.r - *h_.r / *x.h_.r;
      if(h_.r->denominator() == 1){
        t_ = Integer;
        nrat* r = h_.r;
        h_.i = r->numerator();
        delete r;
        return *this;
      }
    case Float:
      t_ = Float;
      h_.d = fmod(h_.i, x.h_.d);
      return *this;
    case Real:
      t_ = Real;
      h_.x = new nreal(h_.i);
      *h_.x %= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(h_.i);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v %= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v %= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v %= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v %= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.r = *h_.r - *h_.r / x.h_.i;
      return *this;
    case Rational:
      *h_.r = *h_.r - *h_.r / *x.h_.r;
      if(h_.r->denominator() == 1){
        nrat* r = h_.r;
        h_.i = h_.r->numerator();
        t_ = Integer;
        delete r;
      }
      return *this;
    case Float:{
      double d = h_.r->toDouble();
      delete h_.r;
      t_ = Float;
      h_.d = fmod(d, x.h_.d);
      return *this;
    }
    case Real:{
      nreal* y = new nreal(*h_.r);
      delete h_.r;
      t_ = Real;
      *y %= *x.h_.x;
      h_.x = y;
      return *this;
    }
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(*h_.r);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.r);
      *v %= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.r);
      *v %= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.r);
      *v %= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *v %= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      h_.d = fmod(h_.d, static_cast<double>(x.h_.i));
      return *this;
    case Rational:
      h_.d = fmod(h_.d, x.h_.r->toDouble());
      return *this;
    case Float:
      h_.d = fmod(h_.d, x.h_.d);
      return *this;
    case Real:
      h_.d = fmod(h_.d, x.h_.x->toDouble());
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(h_.d);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.d);
      *v %= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.d);
      *v %= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.d);
      *v %= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.d);
      *v %= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      *h_.x %= x.h_.i;
      return *this;
    case Rational:
      *h_.x %= *x.h_.r;
      return *this;
    case Float:{
      double d = h_.x->toDouble();
      delete h_.x;
      h_.d = fmod(d, x.h_.d);
      t_ = Float;
      break;
    }
    case Real:
      *h_.x %= *x.h_.x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.push_back(*h_.x);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), *h_.x);
      *v %= *x.h_.v;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), *h_.x);
      *v %= *x.h_.hs->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), *h_.x);
      *v %= *x.h_.sm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *v %= *x.h_.hsm->s;
      h_.v = v;
      t_ = Vector;
      return *this;
    }
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    CFunction* f = new CFunction("Mod");
    f->v.emplace_back(new nvar(*this), Copy);
    f->v.emplace_back(nvar(x, Copy));

    t_ = Function;
    h_.f = f;
    return *this;
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.v %= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.v %= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.v %= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:
      *h_.v %= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.v %= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hs->s %= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hs->s %= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hs->s %= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:
      *h_.hs->s %= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hs->s %= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h %= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.sm->s %= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.sm->s %= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.sm->s %= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:
      *h_.sm->s %= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.sm->s %= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      *h_.hsm->s %= x;
      return *this;
    case Symbol:
    case Function:{
      CFunction* f = new CFunction("Mod");
      f->v.emplace_back(*this);
      f->v.push_back(new nvar(x, Copy));
      t_ = Function;
      h_.f = f;
      return *this;
    }
    case Vector:
      *h_.hsm->s %= *x.h_.v;
      return *this;
    case HeadSequence:
      *h_.hsm->s %= *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this %= *x.h_.hm->h;
    case SequenceMap:
      *h_.hsm->s %= *x.h_.sm->s;
      return *this;
    case HeadSequenceMap:
      *h_.hsm->s %= *x.h_.hsm->s;
      return *this;
    case Reference:
      return *this %= *x.h_.ref->v;
    case Pointer:
      return *this %= *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    *h_.ref->v %= x;
    return *this;
  case Pointer:
    *h_.vp %= x;
    return *this;
  default:
    NERROR("left operand is invalid");
  }
}

nvar nvar::operator%(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return h_.i % x.h_.i;
    case Rational:{
      Head h;
      h.r = new nrat(h_.i);
      *h.r = *h.r - *h.r / *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return fmod(h_.i, x.h_.d);
    case Real:{
      Head h;
      h.x = new nreal(h_.i);
      *h.x %= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      *v %= *x.h_.v;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      *v %= *x.h_.hs->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      *v %= *x.h_.sm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      *v %= *x.h_.hsm->s;
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r = *h.r - *h.r / x.h_.i;
      return nvar(Rational, h);
    }
    case Rational:{
      Head h;
      h.r = new nrat(*h_.r);
      *h.r = *h.r - *h.r / *x.h_.r;
      if(h.r->denominator() == 1){
        nrat* r = h.r;
        h.i = h_.r->numerator();
        delete r;
        return nvar(Integer, h);
      }
      return nvar(Rational, h);
    }
    case Float:
      return fmod(h_.r->toDouble(), x.h_.d);
    case Real:{
      Head h;
      h.x = new nreal(*h_.r);
      *h.x %= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      *h.v %= *x.h_.v;
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      *h.v %= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      *h.v %= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      *h.v %= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return fmod(h_.d, static_cast<double>(x.h_.i));
    case Rational:
      return fmod(h_.d, x.h_.r->toDouble());
    case Float:
      return fmod(h_.d, x.h_.d);
    case Real:
      return fmod(h_.d, x.h_.x->toDouble());
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      *h.v %= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      *h.v %= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      *h.v %= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      *h.v %= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      Head h;
      h.x = new nreal(*h_.x);
      *h.x %= x.h_.i;
      return nvar(Real, h);
    case Rational:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x %= *x.h_.r;
      return nvar(Real, h);
    }
    case Float:
      return fmod(h_.x->toDouble(), x.h_.d);
    case Real:{
      Head h;
      h.x = new nreal(*h_.x);
      *h.x %= *x.h_.x;
      return nvar(Real, h);
    }
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      *h.v %= *x.h_.v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      *h.v %= *x.h_.hs->s;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      *h.v %= *x.h_.sm->s;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      *h.v %= *x.h_.hsm->s;
      return nvar(Vector, h);
    }
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Mod");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.v % x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v % *x.h_.v;
    case HeadSequence:
      return *h_.v % *x.h_.hs->s;
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:
      return *h_.v % *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v % *x.h_.hsm->s;
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s % x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s % *x.h_.v;
    case HeadSequence:
      return *h_.hs->s % *x.h_.hs->s;
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s % *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s % *x.h_.hsm->s;
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h % x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s % x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s % *x.h_.v;
    case HeadSequence:
      return *h_.sm->s % *x.h_.hs->s;
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s % *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s % *x.h_.hsm->s;
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s % x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Mod");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s % *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s % *x.h_.hs->s;
    case HeadMap:
      return *this % *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s % *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s % *x.h_.hsm->s;
    case Reference:
      return *this % *x.h_.ref->v;
    case Pointer:
      return *this % *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v % x;
  case Pointer:
    return *h_.vp % x;
  default:
    NERROR("left operand is invalid");
  }  
}

bool nvar::less(const nvar& x) const{
  switch(t_){
  case None:
    switch(x.t_){
    case None:
      return false;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Undefined:
    switch(x.t_){
    case None:
    case Undefined:
      return false;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case False:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case True:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.i < x.h_.i;
    case Rational:
      return h_.i < *x.h_.r;
    case Float:
      return h_.i < x.h_.d;
    case Real:
      return h_.i < x.h_.x->toLong();
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return *h_.r < x.h_.i;
    case Rational:
      return *h_.r < *x.h_.r;
    case Float:
      return *h_.r < x.h_.d;
    case Real:
      return *h_.r < *x.h_.x;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return h_.d < x.h_.i;
    case Rational:
      return h_.d < *x.h_.r;
    case Float:
      return h_.d < x.h_.d;
    case Real:
      return h_.d < x.h_.x->toDouble();
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
      return false;
    case Integer:
      return *h_.x < x.h_.i;
    case Rational:
      return *h_.x < nreal(*x.h_.r);
    case Float:
      return *h_.x < x.h_.d;
    case Real:
      return *h_.x < *x.h_.x;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case StringPointer:
  case String:
  case Binary:
  case Symbol:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
      return false;
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
      return *h_.s < *x.h_.s;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case RawPointer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p < x.h_.o;
    case RawPointer:
      return h_.p < x.h_.p;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o < x.h_.o;
    case RawPointer:
      return h_.o < x.h_.p;
    case HeadSequence:
      return less(*x.h_.hs->h);
    case HeadMap:
      return less(*x.h_.hm->h);
    case HeadSequenceMap:
      return less(*x.h_.hsm->h);
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
      return false;
    case Vector:{
      int y = h_.v->size() - x.h_.v->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.v < *x.h_.v;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case List:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
      return false;
    case List:{
      int y = h_.l->size() - x.h_.l->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.l < *x.h_.l;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Function:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
      return false;
    case Function:{
      int y = h_.f->v.size() - x.h_.f->v.size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return h_.f->f < x.h_.f->f;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case HeadSequence:
    return h_.hs->h->less(x);
  case Map:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case Function:
    case HeadSequence:
      return false;
    case Map:{
      int y = h_.m->size() - x.h_.m->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.m < *x.h_.m;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case Multimap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case Function:
    case HeadSequence:
    case Map:
      return false;
    case Multimap:{
      int y = h_.mm->size() - x.h_.mm->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.mm < *x.h_.mm;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }
  case HeadMap:
    return h_.hm->h->less(x);
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Symbol:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case Function:
    case HeadSequence:
    case Map:
    case Multimap:
    case HeadMap:
      return false;
    case SequenceMap:{
      int y = h_.sm->s->size() - x.h_.sm->s->size(); 
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.sm->s < *x.h_.sm->s;
    }
    case Reference:
      return less(*x.h_.ref->v);
    case Pointer:
      return less(*x.h_.vp);
    default:
      return true;
    }     
  case HeadSequenceMap:
    return h_.hsm->h->less(x);
  case Reference:
    return h_.ref->v->less(x);
  case Pointer:
    return h_.vp->less(x);
  default:
    return false;
  }
}

nvar nvar::operator<(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case False:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
      return false;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case True:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return h_.i < x.h_.i;
    case Rational:
      return h_.i < *x.h_.r;
    case Float:
      return h_.i < x.h_.d;
    case Real:
      return h_.i < x.h_.x->toLong();
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return *h_.r < x.h_.i;
    case Rational:
      return *h_.r < *x.h_.r;
    case Float:
      return *h_.r < x.h_.d;
    case Real:
      return *h_.r < *x.h_.x;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return h_.d < x.h_.i;
    case Rational:
      return h_.d < *x.h_.r;
    case Float:
      return h_.d < x.h_.d;
    case Real:
      return h_.d < x.h_.x->toDouble();
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return *h_.x < x.h_.i;
    case Rational:
      return *h_.x < nreal(*x.h_.r);
    case Float:
      return *h_.x < x.h_.d;
    case Real:
      return *h_.x < *x.h_.x;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
      return false;
    case Binary:
    case String:
    case StringPointer:
      return *h_.s < *x.h_.s;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case RawPointer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p < x.h_.o;
    case RawPointer:
      return h_.p < x.h_.p;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o < x.h_.o;
    case RawPointer:
      return h_.o < x.h_.p;
    case HeadSequence:
      return *this < *x.h_.hs->h;
    case HeadMap:
      return *this < *x.h_.hm->h;
    case HeadSequenceMap:
      return *this < *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Symbol:
  case Function:
    return nfunc("LT") << new nvar(*this, Copy) << nvar(x, Copy);
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
      return false;
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Vector:{
      int y = h_.v->size() - x.h_.v->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.v < *x.h_.v;
    }
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case List:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
      return false;
    case List:{
      int y = h_.l->size() - x.h_.l->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.l < *x.h_.l;
    }
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case HeadSequence:
    return *h_.hs->h < x;
  case Map:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
      return false;
    case Map:{
      int y = h_.m->size() - x.h_.m->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.m < *x.h_.m;
    }
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case Multimap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
      return false;
    case Multimap:{
      int y = h_.mm->size() - x.h_.mm->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.mm < *x.h_.mm;
    }
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }
  case HeadMap:
    return *h_.hm->h < x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
    case Multimap:
    case HeadMap:
      return false;
    case SequenceMap:{
      int y = h_.sm->s->size() - x.h_.sm->s->size(); 
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.sm->s < *x.h_.sm->s;
    }
    case Symbol:
    case Function:
      return nfunc("LT") << *this << new nvar(x, Copy);
    case Reference:
      return *this < *x.h_.ref->v;
    case Pointer:
      return *this < *x.h_.vp;
    default:
      return true;
    }     
  case HeadSequenceMap:
    return *h_.hsm->h < x;
  case Reference:
    return *h_.ref->v < x;
  case Pointer:
    return *h_.vp < x;
  default:
    return false;
  }
}

nvar nvar::operator<=(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case False:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case True:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
      return false;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return h_.i <= x.h_.i;
    case Rational:
      return h_.i <= *x.h_.r;
    case Float:
      return h_.i <= x.h_.d;
    case Real:
      return h_.i <= x.h_.x->toLong();
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return *h_.r <= x.h_.i;
    case Rational:
      return *h_.r <= *x.h_.r;
    case Float:
      return *h_.r <= x.h_.d;
    case Real:
      return *h_.r <= *x.h_.x;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return h_.d <= x.h_.i;
    case Rational:
      return h_.d <= *x.h_.r;
    case Float:
      return h_.d <= x.h_.d;
    case Real:
      return h_.d <= x.h_.x->toDouble();
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return false;
    case Integer:
      return *h_.x <= x.h_.i;
    case Rational:
      return *h_.x <= nreal(*x.h_.r);
    case Float:
      return *h_.x <= x.h_.d;
    case Real:
      return *h_.x <= *x.h_.x;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
      return false;
    case Binary:
    case String:
    case StringPointer:
      return *h_.s <= *x.h_.s;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case RawPointer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p <= x.h_.o;
    case RawPointer:
      return h_.p <= x.h_.p;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return false;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o <= x.h_.o;
    case RawPointer:
      return h_.o <= x.h_.p;
    case HeadSequence:
      return *this <= *x.h_.hs->h;
    case HeadMap:
      return *this <= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this <= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
      return false;
    case Vector:{
      int y = h_.v->size() - x.h_.v->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.v <= *x.h_.v;
    }
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case List:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
      return false;
    case List:{
      int y = h_.l->size() - x.h_.l->size();

      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
      
      return *h_.l <= *x.h_.l;
    }
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Symbol:
  case Function:
    return nfunc("LE") << new nvar(*this, Copy) << nvar(x, Copy);
  case HeadSequence:
    return *h_.hs->h <= x;
  case Map:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
      return false;
    case Map:{
      int y = h_.m->size() - x.h_.m->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.m <= *x.h_.m;
    }
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case Multimap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
      return false;
    case Multimap:{
      int y = h_.mm->size() - x.h_.mm->size();
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.mm <= *x.h_.mm;
    }
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }
  case HeadMap:
    return *h_.hm->h <= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
    case Multimap:
    case HeadMap:
      return false;
    case SequenceMap:{
      int y = h_.sm->s->size() - x.h_.sm->s->size(); 
         
      if(y < 0){
        return true;
      }
      else if(y > 0){
        return false;
      }
         
      return *h_.sm->s <= *x.h_.sm->s;
    }
    case Symbol:
    case Function:
      return nfunc("LE") << *this << new nvar(x, Copy);
    case Reference:
      return *this <= *x.h_.ref->v;
    case Pointer:
      return *this <= *x.h_.vp;
    default:
      return true;
    }     
  case HeadSequenceMap:
    return *h_.hsm->h <= x;
  case Reference:
    return *h_.ref->v <= x;
  case Pointer:
    return *h_.vp <= x;
  default:
    return false;
  }
}

nvar nvar::operator>(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case False:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case True:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
      return true;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return h_.i > x.h_.i;
    case Rational:
      return h_.i > *x.h_.r;
    case Float:
      return h_.i > x.h_.d;
    case Real:
      return h_.i > x.h_.x->toLong();
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return *h_.r > x.h_.i;
    case Rational:
      return *h_.r > *x.h_.r;
    case Float:
      return *h_.r > x.h_.d;
    case Real:
      return *h_.r > *x.h_.x;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return h_.d > x.h_.i;
    case Rational:
      return h_.d > *x.h_.r;
    case Float:
      return h_.d > x.h_.d;
    case Real:
      return h_.d > x.h_.x->toDouble();
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return *h_.x > x.h_.i;
    case Rational:
      return *h_.x > nreal(*x.h_.r);
    case Float:
      return *h_.x > x.h_.d;
    case Real:
      return *h_.x > *x.h_.x;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
      return true;
    case Binary:
    case String:
    case StringPointer:
      return *h_.s > *x.h_.s;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case RawPointer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return true;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p > x.h_.o;
    case RawPointer:
      return h_.p > x.h_.p;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return true;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o > x.h_.o;
    case RawPointer:
      return h_.o > x.h_.p;
    case HeadSequence:
      return *this > *x.h_.hs->h;
    case HeadMap:
      return *this > *x.h_.hm->h;
    case HeadSequenceMap:
      return *this > *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
      return true;
    case Vector:{
      int y = h_.v->size() - x.h_.v->size();

      if(y > 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
      
      return *h_.v > *x.h_.v;
    }
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case List:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
      return true;
    case List:{
      int y = h_.l->size() - x.h_.l->size();

      if(y > 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
      
      return *h_.l > *x.h_.l;
    }
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Symbol:
  case Function:
    return nfunc("GT") << new nvar(*this, Copy) << nvar(x, Copy);
  case HeadSequence:
    return *h_.hs->h > x;
  case Map:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
      return true;
    case Map:{
      int y = h_.m->size() - x.h_.m->size();
         
      if(y > 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.m > *x.h_.m;
    }
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case Multimap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
      return true;
    case Multimap:{
      int y = h_.mm->size() - x.h_.mm->size();
         
      if(y > 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.mm > *x.h_.mm;
    }
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }
  case HeadMap:
    return *h_.hm->h > x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
    case Multimap:
    case HeadMap:
      return true;
    case SequenceMap:{
      int y = h_.sm->s->size() - x.h_.sm->s->size(); 
         
      if(y > 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.sm->s > *x.h_.sm->s;
    }
    case Symbol:
    case Function:
      return nfunc("GT") << *this << new nvar(x, Copy);
    case Reference:
      return *this > *x.h_.ref->v;
    case Pointer:
      return *this > *x.h_.vp;
    default:
      return false;
    }     
  case HeadSequenceMap:
    return *h_.hsm->h > x;
  case Reference:
    return *h_.ref->v > x;
  case Pointer:
    return *h_.vp > x;
  default:
    return true;
  }
}

nvar nvar::operator>=(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case False:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
      return true;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case True:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return h_.i >= x.h_.i;
    case Rational:
      return h_.i >= *x.h_.r;
    case Float:
      return h_.i >= x.h_.d;
    case Real:
      return h_.i >= x.h_.x->toLong();
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return *h_.r >= x.h_.i;
    case Rational:
      return *h_.r >= *x.h_.r;
    case Float:
      return *h_.r >= x.h_.d;
    case Real:
      return *h_.r >= *x.h_.x;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return h_.d >= x.h_.i;
    case Rational:
      return h_.d >= *x.h_.r;
    case Float:
      return h_.d >= x.h_.d;
    case Real:
      return h_.d >= x.h_.x->toDouble();
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
      return true;
    case Integer:
      return *h_.x >= x.h_.i;
    case Rational:
      return *h_.x >= nreal(*x.h_.r);
    case Float:
      return *h_.x >= x.h_.d;
    case Real:
      return *h_.x >= *x.h_.x;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
      return true;
    case Binary:
    case String:
    case StringPointer:
      return *h_.s >= *x.h_.s;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case RawPointer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return true;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p >= x.h_.o;
    case RawPointer:
      return h_.p >= x.h_.p;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
      return true;
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o >= x.h_.o;
    case RawPointer:
      return h_.o >= x.h_.p;
    case HeadSequence:
      return *this >= *x.h_.hs->h;
    case HeadMap:
      return *this >= *x.h_.hm->h;
    case HeadSequenceMap:
      return *this >= *x.h_.hsm->h;
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
      return true;
    case Vector:{
      int y = h_.v->size() - x.h_.v->size();

      if(y < 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
      
      return *h_.v >= *x.h_.v;
    }
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case List:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
      return true;
    case List:{
      int y = h_.l->size() - x.h_.l->size();

      if(y < 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
      
      return *h_.l >= *x.h_.l;
    }
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Symbol:
  case Function:
    return nfunc("GE") << new nvar(*this, Copy) << nvar(x, Copy);
  case HeadSequence:
    return *h_.hs->h >= x;
  case Map:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
      return true;
    case Map:{
      int y = h_.m->size() - x.h_.m->size();
         
      if(y < 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.m >= *x.h_.m;
    }
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case Multimap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
      return true;
    case Multimap:{
      int y = h_.mm->size() - x.h_.mm->size();
         
      if(y < 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.mm >= *x.h_.mm;
    }
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }
  case HeadMap:
    return *h_.hm->h >= x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case False:
    case True:
    case Integer:
    case Rational:
    case Float:
    case Real:
    case Binary:
    case String:
    case StringPointer:
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
    case RawPointer:
    case Vector:
    case List:
    case HeadSequence:
    case Map:
    case Multimap:
    case HeadMap:
      return true;
    case SequenceMap:{
      int y = h_.sm->s->size() - x.h_.sm->s->size(); 
         
      if(y < 0){
        return false;
      }
      else if(y > 0){
        return true;
      }
         
      return *h_.sm->s >= *x.h_.sm->s;
    }
    case Symbol:
    case Function:
      return nfunc("GE") << *this << new nvar(x, Copy);
    case Reference:
      return *this >= *x.h_.ref->v;
    case Pointer:
      return *this >= *x.h_.vp;
    default:
      return false;
    }     
  case HeadSequenceMap:
    return *h_.hsm->h >= x;
  case Reference:
    return *h_.ref->v >= x;
  case Pointer:
    return *h_.vp >= x;
  default:
    return true;
  }
}

nvar nvar::operator==(const nvar& x) const{
  switch(t_){
  case None:
    switch(x.t_){
    case None:
      return true;
    default:
      return false;
    }
  case Undefined:
    switch(x.t_){
    case Undefined:
      return true;
    default:
      return false;
    }
  case False:
    switch(x.t_){
    case False:
      return true;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case True:
    switch(x.t_){
    case True:
      return true;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Integer:
    switch(x.t_){
    case Integer:
      return h_.i == x.h_.i;
    case Rational:
      return h_.i == *x.h_.r;
    case Float:
      return h_.i == x.h_.d;
    case Real:
      return h_.i == x.h_.x->toLong();
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Rational:
    switch(x.t_){
    case Integer:
      return *h_.r == x.h_.i;
    case Rational:
      return *h_.r == *x.h_.r;
    case Float:
      return *h_.r == x.h_.d;
    case Real:
      return *h_.r == *x.h_.x;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Float:
    switch(x.t_){
    case Integer:
      return h_.d == x.h_.i;
    case Rational:
      return h_.d == x.h_.r->toDouble();
    case Float:
      return h_.d == x.h_.d;
    case Real:
      return h_.d == x.h_.x->toDouble();
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Real:
    switch(x.t_){
    case Integer:
      return *h_.x == x.h_.i;
    case Rational:
      return *h_.x == nreal(*x.h_.r);
    case Float:
      return *h_.x == x.h_.d;
    case Real:
      return *h_.x == *x.h_.x;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case Binary:
    case String:
    case StringPointer:
      return *h_.s == *x.h_.s;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case RawPointer:
    switch(x.t_){
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p == x.h_.o;
    case RawPointer:
      return h_.p == x.h_.p;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o == x.h_.o;
    case RawPointer:
      return h_.o == x.h_.p;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Vector:
    switch(x.t_){
    case Vector:
      return *h_.v == *x.h_.v;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case List:
    switch(x.t_){
    case List:
      return *h_.l == *x.h_.l;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Symbol:
  case Function:
    return nfunc("EQ") << new nvar(*this, Copy) << nvar(x, Copy);
  case HeadSequence:
    switch(x.t_){
    case HeadSequence:
      return *h_.hs->h == *x.h_.hs->h && *h_.hs->s == *x.h_.hs->s;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Map:
    switch(x.t_){
    case Map:
      return *h_.m == *x.h_.m;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case Multimap:
    switch(x.t_){
    case Multimap:
      return *h_.mm == *x.h_.mm;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case HeadMap:
    switch(x.t_){
    case HeadMap:
      return *h_.hm->h == *x.h_.hm->h && *h_.hm->m == *x.h_.hm->m;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }
  case SequenceMap:
    switch(x.t_){
    case SequenceMap:
      return *h_.sm->s == *x.h_.sm->s && *h_.sm->m == *x.h_.sm->m;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    }     
  case HeadSequenceMap:
    switch(x.t_){
    case HeadSequenceMap:
      return *h_.hsm->h == *x.h_.hsm->h && 
        *h_.hsm->s == *x.h_.hsm->s && 
        *h_.hsm->m == *x.h_.hsm->m;
    case Symbol:
    case Function:
      return nfunc("EQ") << *this << new nvar(x, Copy);
    case Reference:
      return *this == *x.h_.ref->v;
    case Pointer:
      return *this == *x.h_.vp;
    default:
      return false;
    } 
  case Reference:
    return *h_.ref->v == x;
  case Pointer:
    return *h_.vp == x;
  default:
    return false;
  }
}

nvar nvar::operator!=(const nvar& x) const{
  switch(t_){
  case None:
    switch(x.t_){
    case None:
      return false;
    default:
      return true;
    }
  case Undefined:
    switch(x.t_){
    case Undefined:
      return false;
    default:
      return true;
    }
  case False:
    switch(x.t_){
    case False:
      return false;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case True:
    switch(x.t_){
    case False:
      return True;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Integer:
    switch(x.t_){
    case Integer:
      return h_.i != x.h_.i;
    case Rational:
      return h_.i != *x.h_.r;
    case Float:
      return h_.i != x.h_.d;
    case Real:
      return h_.i != x.h_.x->toLong();
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Rational:
    switch(x.t_){
    case Integer:
      return *h_.r != x.h_.i;
    case Rational:
      return *h_.r != *x.h_.r;
    case Float:
      return *h_.r != x.h_.d;
    case Real:
      return *h_.r != *x.h_.x;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Float:
    switch(x.t_){
    case Integer:
      return h_.d != x.h_.i;
    case Rational:
      return h_.d != *x.h_.r;
    case Float:
      return h_.d != x.h_.d;
    case Real:
      return h_.d != x.h_.x->toDouble();
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Real:
    switch(x.t_){
    case Integer:
      return *h_.x != x.h_.i;
    case Rational:
      return *h_.x != nreal(*x.h_.r);
    case Float:
      return *h_.x != x.h_.d;
    case Real:
      return *h_.x != *x.h_.x;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case StringPointer:
  case String:
  case Binary:
    switch(x.t_){
    case Binary:
    case String:
    case StringPointer:
      return *h_.s != *x.h_.s;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case RawPointer:
    switch(x.t_){
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.p != x.h_.o;
    case RawPointer:
      return h_.p != x.h_.p;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case ObjectPointer:
    case LocalObject:
    case SharedObject:
      return h_.o != x.h_.o;
    case RawPointer:
      return h_.o != x.h_.p;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Vector:
    switch(x.t_){
    case Vector:
      return *h_.v != *x.h_.v;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case List:
    switch(x.t_){
    case List:
      return *h_.l != *x.h_.l;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Symbol:
  case Function:
    return nfunc("NE") << new nvar(*this, Copy) << nvar(x, Copy);
  case HeadSequence:
    switch(x.t_){
    case HeadSequence:
      return *h_.hs->h != *x.h_.hs->h && *h_.hs->s != *x.h_.hs->s;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Map:
    switch(x.t_){
    case Map:
      return *h_.m != *x.h_.m;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case Multimap:
    switch(x.t_){
    case Multimap:
      return *h_.mm != *x.h_.mm;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case HeadMap:
    switch(x.t_){
    case HeadMap:
      return *h_.hm->h != *x.h_.hm->h && *h_.hm->m != *x.h_.hm->m;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }
  case SequenceMap:
    switch(x.t_){
    case SequenceMap:
      return *h_.sm->s != *x.h_.sm->s && *h_.sm->m != *x.h_.sm->m;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    }     
  case HeadSequenceMap:
    switch(x.t_){
    case HeadSequenceMap:
      return *h_.hsm->h != *x.h_.hsm->h && 
        *h_.hsm->s != *x.h_.hsm->s && 
        *h_.hsm->m != *x.h_.hsm->m;
    case Symbol:
    case Function:
      return nfunc("NE") << *this << new nvar(x, Copy);
    case Reference:
      return *this != *x.h_.ref->v;
    case Pointer:
      return *this != *x.h_.vp;
    default:
      return true;
    } 
  case Reference:
    return *h_.ref->v != x;
  case Pointer:
    return *h_.vp != x;
  default:
    return true;
  }
}

nvar nvar::operator&&(const nvar& x) const{
  switch(t_){
  case None:
  case False:
  case Undefined:
    return false;
  case True:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case Integer:
      return x.h_.i != 0;
    case Rational:
      return *x.h_.r != 0;
    case Float:
      return x.h_.d != 0.0;
    case Real:
      return *x.h_.x != 0;
    case Function:
    case Symbol:
    case Vector:
    case HeadSequence:
    case HeadMap:
    case SequenceMap:
    case HeadSequenceMap:
      return *this;
    case Reference:
      return *x.h_.ref->v;        
    case Pointer:
      return *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return h_.i != 0;
    case Integer:
      return h_.i != 0 && x.h_.i != 0;
    case Rational:
      return h_.i != 0 && *x.h_.r != 0;
    case Float:
      return h_.i != 0 && x.h_.d != 0.0;
    case Real:
      return h_.i != 0 && *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      v->andBy(*x.h_.v);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      v->andBy(*x.h_.hs->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      v->andBy(*x.h_.sm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      v->andBy(*x.h_.hsm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this && *x.h_.ref->v;        
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *h_.r != 0;
    case Integer:
      return *h_.r != 0 && x.h_.i != 0;
    case Rational:
      return *h_.r != 0 && *x.h_.r != 0;
    case Float:
      return *h_.r != 0 && x.h_.d != 0.0;
    case Real:
      return *h_.r != 0 && *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      h.v->andBy(*x.h_.v);
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      h.v->andBy(x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      h.v->andBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      h.v->andBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return h_.d != 0.0;
    case Integer:
      return h_.d != 0.0 && x.h_.i != 0;
    case Rational:
      return h_.d != 0.0 && *x.h_.r != 0;
    case Float:
      return h_.d != 0.0 && x.h_.d != 0.0;
    case Real:
      return h_.d != 0.0 && *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      h.v->andBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      h.v->andBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      h.v->andBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      h.v->andBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *h_.x != 0;
    case Integer:
      return *h_.x != 0 && x.h_.i != 0;
    case Rational:
      return *h_.x != 0 && *x.h_.r != 0;
    case Float:
      return *h_.x != 0.0 && x.h_.d != 0.0;
    case Real:
      return *h_.x != 0 && *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      h.v->andBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      h.v->andBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      h.v->andBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      h.v->andBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case String:
  case StringPointer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return !h_.s->empty();
    case Integer:
      return !h_.s->empty() && x.h_.i != 0;
    case Rational:
      return !h_.s->empty() && *x.h_.r != 0;
    case Float:
      return !h_.s->empty() && x.h_.d != 0.0;
    case Real:
      return !h_.s->empty() && *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*h_.s);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      h.v->andBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      h.v->andBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      h.v->andBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      h.v->andBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("And");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *this;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return h_.v && x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v && *x.h_.v;
    case HeadSequence:
      return *h_.v && *x.h_.hs->s;
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:
      return *h_.v && *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v && *x.h_.hsm->s;
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *this;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s && x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s && *x.h_.v;
    case HeadSequence:
      return *h_.hs->s && *x.h_.hs->s;
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s && *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s && *x.h_.hsm->s;
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadMap:
    return *h_.hm->h && x;
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *this;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s && x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s && *x.h_.v;
    case HeadSequence:
      return *h_.sm->s && *x.h_.hs->s;
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s && *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s && *x.h_.hsm->s;
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return *this;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s && x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("And");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s && *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s && *x.h_.hs->s;
    case HeadMap:
      return *this && *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s && *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s && *x.h_.hsm->s;
    case Reference:
      return *this && *x.h_.ref->v;
    case Pointer:
      return *this && *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v && x;
  case Pointer:
    return *h_.vp && x;
  default:
    NERROR("left operand is invalid");
  }  
}

nvar nvar::operator||(const nvar& x) const{
  switch(t_){
  case None:
  case Undefined:
  case False:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return false;
    case True:
      return true;
    case Integer:
      return x.h_.i != 0;
    case Rational:
      return *x.h_.r != 0;
    case Float:
      return x.h_.d != 0.0;
    case Real:
      return *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(undef);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), undef);
      v->orBy(*x.h_.v);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), undef);
      v->orBy(*x.h_.hs->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), undef);
      v->orBy(*x.h_.sm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), undef);
      v->orBy(*x.h_.hsm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;        
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case True:
    return true;
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return h_.i != 0;
    case True:
      return true;
    case Integer:
      return h_.i != 0 || x.h_.i != 0;
    case Rational:
      return h_.i != 0 || *x.h_.r != 0;
    case Float:
      return h_.i != 0 || x.h_.d != 0.0;
    case Real:
      return h_.i != 0 || *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      nvec* v = new nvec(x.h_.v->size(), h_.i);
      v->orBy(*x.h_.v);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequence:{
      nvec* v = new nvec(x.h_.hs->s->size(), h_.i);
      v->orBy(*x.h_.hs->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:{
      nvec* v = new nvec(x.h_.sm->s->size(), h_.i);
      v->orBy(*x.h_.sm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      nvec* v = new nvec(x.h_.hsm->s->size(), h_.i);
      v->orBy(*x.h_.hsm->s);
      Head h;
      h.v = v;
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;        
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return *h_.r != 0;
    case True:
      return true;
    case Integer:
      return *h_.r != 0 || x.h_.i != 0;
    case Rational:
      return *h_.r != 0 || *x.h_.r != 0;
    case Float:
      return *h_.r != 0 || x.h_.d != 0.0;
    case Real:
      return *h_.r != 0 || *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.r);
      h.v->orBy(*x.h_.v);
      return nvar(Vector, h); 
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.r);
      h.v->orBy(x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.r);
      h.v->orBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.r);
      h.v->orBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return h_.d != 0.0;
    case True:
      return true;
    case Integer:
      return h_.d != 0.0 || x.h_.i != 0;
    case Rational:
      return h_.d != 0.0 || *x.h_.r != 0;
    case Float:
      return h_.d != 0.0 || x.h_.d != 0.0;
    case Real:
      return h_.d != 0.0 || *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), h_.d);
      h.v->orBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), h_.d);
      h.v->orBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), h_.d);
      h.v->orBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), h_.d);
      h.v->orBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return *h_.x != 0;
    case True:
      return true;
    case Integer:
      return *h_.x != 0 || x.h_.i != 0;
    case Rational:
      return *h_.x != 0 || *x.h_.r != 0;
    case Float:
      return h_.d != 0.0 || x.h_.d != 0.0;
    case Real:
      return *h_.x != 0 || *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      h.v->orBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      h.v->orBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      h.v->orBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      h.v->orBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }  
  case String:
  case StringPointer:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return !h_.s->empty();
    case True:
      return true;
    case Integer:
      return !h_.s->empty() || x.h_.i != 0;
    case Rational:
      return !h_.s->empty() || *x.h_.r != 0;
    case Float:
      return !h_.s->empty() || x.h_.d != 0.0;
    case Real:
      return !h_.s->empty() || *x.h_.x != 0;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*h_.s);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:{
      Head h;
      h.v = new nvec(x.h_.v->size(), *h_.x);
      h.v->orBy(*x.h_.v);
      return nvar(Vector, h);
    }
    case HeadSequence:{
      Head h;
      h.v = new nvec(x.h_.hs->s->size(), *h_.x);
      h.v->orBy(*x.h_.hs->s);
      return nvar(Vector, h);
    }
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:{
      Head h;
      h.v = new nvec(x.h_.sm->s->size(), *h_.x);
      h.v->orBy(*x.h_.sm->s);
      return nvar(Vector, h);
    }
    case HeadSequenceMap:{
      Head h;
      h.v = new nvec(x.h_.hsm->s->size(), *h_.x);
      h.v->orBy(*x.h_.hsm->s);
      return nvar(Vector, h);
    }
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Or");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case Vector:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return h_.v;
    case True:
      return true;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return h_.v || x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.v || *x.h_.v;
    case HeadSequence:
      return *h_.v || *x.h_.hs->s;
      return *this;
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:
      return *h_.v || *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.v || *x.h_.hsm->s;
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequence:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return *h_.hs->s;
    case True:
      return true;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hs->s || x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hs->s || *x.h_.v;
    case HeadSequence:
      return *h_.hs->s || *x.h_.hs->s;
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:
      return *h_.hs->s || *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hs->s || *x.h_.hsm->s;
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case SequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return *h_.sm->s;
    case True:
      return true;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.sm->s || x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.sm->s || *x.h_.v;
    case HeadSequence:
      return *h_.sm->s || *x.h_.hs->s;
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:
      return *h_.sm->s || *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.sm->s || *x.h_.hsm->s;
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case HeadSequenceMap:
    switch(x.t_){
    case None:
    case Undefined:
    case False:
      return *h_.hsm->s;
    case True:
      return true;
    case Integer:
    case Rational:
    case Float:
    case Real:
      return *h_.hsm->s || x;
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Or");
      h.f->v.push_back(*this);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case Vector:
      return *h_.hsm->s || *x.h_.v;
    case HeadSequence:
      return *h_.hsm->s || *x.h_.hs->s;
    case HeadMap:
      return *this || *x.h_.hm->h;
    case SequenceMap:
      return *h_.hsm->s || *x.h_.sm->s;
    case HeadSequenceMap:
      return *h_.hsm->s || *x.h_.hsm->s;
    case Reference:
      return *this || *x.h_.ref->v;
    case Pointer:
      return *this || *x.h_.vp;
    default:
      NERROR("invalid operands");
    }
  case Reference:
    return *h_.ref->v || x;
  case Pointer:
    return *h_.vp || x;
  default:
    NERROR("left operand is invalid");
  }  
}

void nvar::append(const nvar& x){
  switch(t_){
  case None:
  case Undefined:
    switch(x.t_){
    case Vector:
      t_ = Vector;
      h_.v = new nvec(*x.h_.v);
      break;
    case List:
      t_ = List;
      h_.l = new nlist(*x.h_.l);
      break;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case False:
  case True:
  case Integer:
  case Rational:
  case Float:
  case Real:
  case Symbol:
  case String:
  case StringPointer:
  case Binary:
  case RawPointer:
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    switch(x.t_){
    case Vector:{
      Head h;
      h.v = new nvec(*x.h_.v);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case List:{
      Head h;
      h.l = new nlist(*x.h_.l);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(List, h));
      t_ = HeadSequence;
      break;
    }
    case Function:{
      Head h;
      h.v = new nvec(x.h_.f->v);
      h_.hs = new CHeadSequence(new nvar(t_, h_), new nvar(Vector, h));
      t_ = HeadSequence;
      break;
    }
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case Vector:
    switch(x.t_){
    case Vector:
      h_.v->append(*x.h_.v);
      break;
    case List:
      h_.v->insert(h_.v->begin(), x.h_.l->begin(), x.h_.l->end());
      break;
    case Function:
      h_.v->append(x.h_.f->v);
      break;
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case List:
    switch(x.t_){
    case Vector:
      h_.l->insert(h_.l->begin(), x.h_.v->begin(), x.h_.v->end());
      break;
    case List:
      h_.l->append(*x.h_.l);
      break;
    case Function:
      h_.l->insert(h_.l->begin(), x.h_.f->v.begin(), x.h_.f->v.end());
      break;
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case Function:
    switch(x.t_){
    case Vector:
      h_.f->v.append(*x.h_.v);
      break;
    case List:
      h_.f->v.insert(h_.v->begin(), x.h_.l->begin(), x.h_.l->end());
      break;
    case Function:
      h_.f->v.append(x.h_.f->v);
      break;
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case HeadSequence:
    h_.hs->s->append(x);
    break;
  case Map:
    switch(x.t_){
    case Vector:{
      Head hv;
      hv.v = new nvec(*x.h_.v);

      Head hm;
      hm.m = h_.m;

      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
      t_ = SequenceMap;
      break;
    }
    case List:{
      Head hl;
      hl.l = new nlist(*x.h_.l);

      Head hm;
      hm.m = h_.m;

      h_.sm = new CSequenceMap(new nvar(List, hl), new nvar(Map, hm));
      t_ = SequenceMap;
      break;
    }
    case Function:{
      Head hv;
      hv.v = new nvec(x.h_.f->v);

      Head hm;
      hm.m = h_.m;

      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Map, hm));
      t_ = SequenceMap;
      break;
    }
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case Multimap:
    switch(x.t_){
    case Vector:{
      Head hv;
      hv.v = new nvec(*x.h_.v);

      Head hm;
      hm.mm = h_.mm;

      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
      t_ = SequenceMap;
      break;
    }
    case List:{
      Head hl;
      hl.l = new nlist(*x.h_.l);

      Head hm;
      hm.mm = h_.mm;

      h_.sm = new CSequenceMap(new nvar(List, hl), new nvar(Multimap, hm));
      t_ = SequenceMap;
      break;
    }
    case Function:{
      Head hv;
      hv.v = new nvec(x.h_.f->v);

      Head hm;
      hm.mm = h_.mm;

      h_.sm = new CSequenceMap(new nvar(Vector, hv), new nvar(Multimap, hm));
      t_ = SequenceMap;
      break;
    }
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case HeadMap:
    switch(x.t_){
    case Vector:{
      Head hv;
      hv.v = new nvec(*x.h_.v);

      h_.hsm = 
        new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
      t_ = HeadSequenceMap;
      break;
    }
    case List:{
      Head hl;
      hl.l = new nlist(*x.h_.l);

      h_.hsm = 
        new CHeadSequenceMap(h_.hm->h, new nvar(List, hl), h_.hm->m);
      t_ = HeadSequenceMap;
      break;
    }
    case Function:{
      Head hv;
      hv.v = new nvec(x.h_.f->v);

      h_.hsm = 
        new CHeadSequenceMap(h_.hm->h, new nvar(Vector, hv), h_.hm->m);
      t_ = HeadSequenceMap;
      break;
    }
    case HeadSequence:
      append(*x.h_.hs->s);
      break;
    case SequenceMap:
      append(*x.h_.sm->s);
      break;
    case HeadSequenceMap:
      append(*x.h_.hsm->s);
      break;
    case Reference:
      append(*x.h_.ref->v);
      break;
    case Pointer:
      append(*x.h_.vp);
      break;
    }
    break;
  case SequenceMap:
    h_.sm->s->append(x);
    break;
  case HeadSequenceMap:
    h_.hsm->s->append(x);
    break;
  case Reference:
    h_.ref->v->append(x);
    break;
  case Pointer:
    h_.vp->append(x);
    break;
  }
}

void nvar::setHead(const nvar& x){
  switch(t_){
  case Rational:
    switch(x.t_){
    case Rational:
      *h_.r = *x.h_.r;
      break;
    case Real:
      delete h_.r;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      delete h_.r;
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      delete h_.r;
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      delete h_.r;
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      delete h_.r;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.r;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.r;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.r;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case Real:
    switch(x.t_){
    case Rational:
      delete h_.x;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      *h_.x = *x.h_.x;
      break;
    case Symbol:
      delete h_.x;
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      delete h_.x;
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      delete h_.x;
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      delete h_.x;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.x;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.x;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.x;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }       
  case Symbol:
    switch(x.t_){
    case Rational:
      delete h_.s;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      delete h_.s;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      *h_.s = *x.h_.s;
      break;
    case Binary:
      t_ = Binary;
      *h_.s = *x.h_.s;
      break;
    case String:
      t_ = String;
      *h_.s = *x.h_.s;
      break;
    case LocalObject:
      delete h_.s;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.s;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.s;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.s;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case String:
    switch(x.t_){
    case Rational:
      delete h_.s;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      delete h_.s;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      t_ = Symbol;
      *h_.s = *x.h_.s;
      break;
    case Binary:
      t_ = Binary;
      *h_.s = *x.h_.s;
      break;
    case String:
      *h_.s = *x.h_.s;
      break;
    case LocalObject:
      delete h_.s;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.s;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.s;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.s;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case Binary:
    switch(x.t_){
    case Rational:
      delete h_.s;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      delete h_.s;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      t_ = Symbol;
      *h_.s = *x.h_.s;
      break;
    case Binary:
      *h_.s = *x.h_.s;
      break;
    case String:
      t_ = String;
      *h_.s = *x.h_.s;
      break;
    case LocalObject:
      delete h_.s;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.s;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.s;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.s;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case LocalObject:
    switch(x.t_){
    case Rational:
      delete h_.o;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      delete h_.o;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      delete h_.o;
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      delete h_.o;
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      delete h_.o;
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      delete h_.o;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.o;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      delete h_.o;
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.o;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case SharedObject:
    switch(x.t_){
    case Rational:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      if(h_.o->deref()){
        delete h_.o;
      }
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.o;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case Vector:{
    nvar* h = new nvar;
    h->setHead(x);

    Head hv;
    hv.v = h_.v;
 
    t_ = HeadSequence;
    h_.hs = new CHeadSequence(h, new nvar(Vector, hv));
    break;
  }
  case List:{
    nvar* h = new nvar;
    h->setHead(x);

    Head hl;
    hl.l = h_.l;
 
    t_ = HeadSequence;
    h_.hs = new CHeadSequence(h, new nvar(List, hl));
    break;
  }
  case Function:
    switch(x.t_){
    case Rational:
      delete h_.f;
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      delete h_.f;
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      delete h_.f;
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      delete h_.f;
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      delete h_.f;
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      delete h_.f;
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      delete h_.f;
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      h_.f->f = x.h_.f->f;
      h_.f->v = x.h_.f->v;
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      delete h_.f;
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  case HeadSequence:
    h_.hs->h->setHead(x);
    break;
  case Map:{
    nvar* h = new nvar;
    h->setHead(x);

    Head hm;
    hm.m = h_.m;
    
    t_ = HeadMap;
    h_.hm = new CHeadMap(h, new nvar(Map, hm));
    break;
  }
  case Multimap:{
    nvar* h = new nvar;
    h->setHead(x);

    Head hm;
    hm.mm = h_.mm;
    
    t_ = HeadMap;
    h_.hm = new CHeadMap(h, new nvar(Multimap, hm));
    break;
  }
  case HeadMap:
    h_.hm->h->setHead(x);
    break;
  case SequenceMap:{
    nvar* h = new nvar;
    h->setHead(x);

    t_ = HeadSequenceMap;
    h_.hsm = new CHeadSequenceMap(h, h_.sm->s, h_.sm->m);
    break;
  }
  case HeadSequenceMap:
    h_.hsm->h->setHead(x);
    break;
  case Pointer:
    h_.vp->setHead(x);
    break;
  case Reference:
    h_.ref->v->setHead(x);
    break;
  default:
    switch(x.t_){
    case Rational:
      t_ = Rational;
      h_.r = new nrat(*x.h_.r);
      break;
    case Real:
      t_ = Real;
      h_.x = new nreal(*x.h_.x);
      break;
    case Symbol:
      t_ = Symbol;
      h_.s = new nstr(*x.h_.s);
      break;
    case Binary:
      t_ = Binary;
      h_.s = new nstr(*x.h_.s);
      break;
    case String:
      t_ = String;
      h_.s = new nstr(*x.h_.s);
      break;
    case LocalObject:
      t_ = LocalObject;
      h_.o = x.h_.o->clone();
      break;
    case SharedObject:
      t_ = SharedObject;
      h_.o = x.h_.o;
      h_.o->ref();
      break;
    case Function:
      t_ = Function;
      h_.f = x.h_.f->clone();
      break;
    case HeadSequence:
      setHead(*x.h_.hs->h);
      break;
    case HeadMap:
      setHead(*x.h_.hm->h);
      break;
    case HeadSequenceMap:
      setHead(*x.h_.hsm->h);
      break;
    case Reference:
      setHead(*x.h_.ref->v);
      break;
    case Pointer:
      setHead(*x.h_.vp);
      break;
    default:
      t_ = x.t_;
      h_ = x.h_;
      break;
    }
  }
}

char* nvar::pack(size_t& length, bool compress) const{
  size_t size = _packBlockSize;
  char* pbuf = (char*)malloc(size);

  size_t pos = 0;
  pbuf = pack_(pbuf, size, pos);
 
  if(compress){
    size_t csize = size*2;
    char* cbuf = (char*)malloc(csize);

    csize = zlib_compress_(pbuf, cbuf, size, csize);
    free(pbuf);
    length = csize;

    return cbuf;
  }

  length = pos;
  return pbuf;
}

char* nvar::pack_(char* buf, size_t& size, size_t& pos) const{
  if(size - pos < _packBlockSize){
    size += _packBlockSize;
    buf = (char*)realloc(buf, size);
  }
  
  switch(t_){
  case Undefined:
    buf[pos++] = Undefined;
    break;
  case False:
    buf[pos++] = False;
    break;
  case True:
    buf[pos++] = True;
    break;
  case Integer:{
    int64_t i = h_.i;

    if(i & 0xffffffff00000000){
      buf[pos++] = Integer;
      memcpy(buf + pos, &i, 8);
      pos += 8;
    }
    else if(i & 0xffffffffffff0000){
      buf[pos++] = PackInt32;
      int32_t j = i;
      memcpy(buf + pos, &j, 4);
      pos += 4;
    }
    else if(i & 0xffffffffffffff00){
      buf[pos++] = PackInt16;
      int16_t j = i;
      memcpy(buf + pos, &j, 2);
      pos += 2;
    }
    else{
      int8_t j = i;

      switch(j){
      case 0:
        buf[pos++] = Pack0;
        break;
      case 1:
        buf[pos++] = Pack1;
        break;
      case 2:
        buf[pos++] = Pack2;
        break;
      case 3:
        buf[pos++] = Pack3;
        break;
      case 4:
        buf[pos++] = Pack4;
        break;
      case 5:
        buf[pos++] = Pack5;
        break;
      case 6:
        buf[pos++] = Pack6;
        break;
      case 7:
        buf[pos++] = Pack7;
        break;
      case 8:
        buf[pos++] = Pack8;
        break;
      case 9:
        buf[pos++] = Pack9;
        break;
      case 10:
        buf[pos++] = Pack10;
        break;
      default:
        buf[pos++] = PackInt8;
        memcpy(buf + pos, &j, 1);
        ++pos;
        break;
      }
    }
    break;
  }
  case Rational:{
    buf[pos++] = Rational;

    const nrat& r = *h_.r;
    int64_t n = r.numerator();
    int64_t d = r.denominator();
    memcpy(buf + pos, &n, 8);
    pos += 8;
    memcpy(buf + pos, &d, 8);
    pos += 8;
    break;
  }
  case Float:{
    float f = h_.d;
    double d = f;
    if(d == h_.d){
      buf[pos++] = PackFloat32;
      memcpy(buf + pos, &f, 4);
      pos += 4;
    }
    else{
      buf[pos++] = Float;
      memcpy(buf + pos, &h_.d, 8);
      pos += 8;
    }
    break;
  }
  case Real:{
    buf[pos++] = Real;
    const nreal& x = *h_.x;
    nstr s = x.toStr();
    uint16_t len = s.length();

    if(size - pos < len){
      size += len + _packBlockSize;
      buf = (char*)realloc(buf, size);
    }

    memcpy(buf + pos, &len, 2);
    pos += 2;

    memcpy(buf + pos, s.c_str(), len);
    pos += len;
    break;
  }
  case Symbol:{
    const nstr& sbuf = *h_.s;
    uint32_t len = sbuf.length();

    if(len <= 255){
      buf[pos++] = Symbol;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else{
      buf[pos++] = PackLongSymbol;
      memcpy(buf + pos, &len, 4);
      ++pos;
    }
    
    if(size - pos < len){
      size += len + _packBlockSize;
      buf = (char*)realloc(buf, size);
    }

    sbuf.copy(buf + pos, len, 0);
    pos += len;
    break;
  }
  case String:
  case StringPointer:{
    const nstr& sbuf = *h_.s;
    uint32_t len = sbuf.length();

    if(len <= 255){
      buf[pos++] = PackShortString;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else if(len <= 65535){
      buf[pos++] = String;
      uint16_t plen = len;
      memcpy(buf + pos, &plen, 2);
      ++pos;
    }
    else{
      Type t = PackLongString;
      buf[pos++] = t;
      memcpy(buf + pos, &len, 4);
      ++pos;
    }
    
    if(size - pos < len){
      size += len + _packBlockSize;
      buf = (char*)realloc(buf, size);
    }

    sbuf.copy(buf + pos, len, 0);
    pos += len;
    break; 
  }
  case Binary:{
    const nstr& sbuf = *h_.s;
    uint32_t len = sbuf.length();

    Type t = Binary;
    buf[pos++] = t;

    memcpy(buf + pos, &len, 4);
    pos += 4;

    if(size - pos < len){
      size += len + _packBlockSize;
      buf = (char*)realloc(buf, size);
    }

    sbuf.copy(buf + pos, len, 0);
    pos += len;
    break;
  }
  case RawPointer:
    NERROR("attempt to pack pointer");
  case ObjectPointer:
  case LocalObject:
  case SharedObject:
    NERROR("attempt to pack object");
  case Vector:{
    const nvec& v = *h_.v;

    uint32_t len = v.size();
    if(len <= 255){
      buf[pos++] = PackShortVector;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else if(len <= 65535){
      buf[pos++] = Vector;
      uint16_t plen = len;
      memcpy(buf + pos, &plen, 2);
      pos += 2;
    }
    else{
      buf[pos++] = PackLongVector;
      memcpy(buf + pos, &len, 4);
      pos += 4;
    }
    
    for(size_t i = 0; i < len; ++i){
      v[i].pack_(buf, size, pos);
    }

    break;
  }
  case List:{
    const nlist& l = *h_.l;

    uint32_t len = l.size();
    if(len <= 255){
      buf[pos++] = PackShortList;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else if(len <= 65535){
      buf[pos++] = List;
      uint16_t plen = len;
      memcpy(buf + pos, &plen, 2);
      pos += 2;
    }
    else{
      buf[pos++] = PackLongList;
      memcpy(buf + pos, &len, 4);
      pos += 4;
    }
    
    for(auto& itr : l){
      buf = itr.pack_(buf, size, pos);
    }
    break;
  }
  case Function:{
    buf[pos++] = Function;

    const nvec& v = h_.f->v;

    const nstr& sbuf = h_.f->f;
    uint32_t len = sbuf.length();
    uint32_t n = v.size();
    uint32_t m = h_.f->m ? h_.f->m->size() : 0;
    bool isShort;

    if(len <= 255 && n <= 255 && m <= 255){
      isShort = true;
      buf[pos++] = Function;
      uint8_t plen = len;
      uint8_t pn = n;
      memcpy(buf + pos, &plen, 1);
      ++pos;

      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }

      sbuf.copy(buf + pos, len, 0);
      pos += len;
      memcpy(buf + pos, &pn, 1);
    }
    else{
      isShort = false;
      buf[pos++] = PackLongFunction;
      memcpy(buf + pos, &len, 4);
      ++pos;

      if(size - pos < len){
        size += len + _packBlockSize;
        buf = (char*)realloc(buf, size);
      }

      sbuf.copy(buf + pos, len, 0);
      pos += len;
      memcpy(buf + pos, &n, 4);
    }

    for(size_t i = 0; i < n; ++i){
      buf = v[i].pack_(buf, size, pos);
    }

    if(isShort){
      if(m > 0){
        uint8_t mlen = m;
        memcpy(buf + pos, &mlen, 1);
        ++pos;
        for(auto& itr : *h_.f->m){
          buf = itr.first.pack_(buf, size, pos);
          buf = itr.second.pack_(buf, size, pos);
        }
      }
      else{
        uint8_t mlen = 0;
        memcpy(buf + pos, &mlen, 1);
        ++pos;
      } 
    }  
    else if(m > 0){
      uint32_t mlen = m;
      memcpy(buf + pos, &mlen, 4);
      pos += 4;
      for(auto& itr : *h_.f->m){
        buf = itr.first.pack_(buf, size, pos);
        buf = itr.second.pack_(buf, size, pos);
      }
    }
    else{
      uint32_t mlen = 0;
      memcpy(buf + pos, &mlen, 4);
      pos += 4;
    }

    break;
  }
  case HeadSequence:{
    buf[pos++] = HeadSequence;
    nvar h = head();
    buf = h.pack_(buf, size, pos);
    buf = h_.hs->s->pack_(buf, size, pos);
    break;
  }
  case Map:{
    uint32_t len = h_.m->size();
    if(len <= 255){
      buf[pos++] = PackShortMap;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else if(len <= 65535){
      buf[pos++] = Map;
      uint16_t plen = len;
      memcpy(buf + pos, &plen, 2);
      pos += 2;
    }
    else{
      buf[pos++] = PackLongMap;
      memcpy(buf + pos, &len, 4);
      pos += 4;
    }

    for(auto& itr : *h_.m){
      buf = itr.first.pack_(buf, size, pos);
      buf = itr.second.pack_(buf, size, pos);
    }
    break;    
  }
  case Multimap:{
    uint32_t len = h_.mm->size();
    if(len <= 255){
      buf[pos++] = PackShortMultimap;
      uint8_t plen = len;
      memcpy(buf + pos, &plen, 1);
      ++pos;
    }
    else if(len <= 65535){
      buf[pos++] = Multimap;
      uint16_t plen = len;
      memcpy(buf + pos, &plen, 2);
      pos += 2;
    }
    else{
      buf[pos++] = PackLongMultimap;
      memcpy(buf + pos, &len, 4);
      pos += 4;
    }

    for(auto& itr : *h_.mm){
      buf = itr.first.pack_(buf, size, pos);
      buf = itr.second.pack_(buf, size, pos);
    }
    break;    
  }
  case HeadMap:{
    buf[pos++] = HeadMap;
    nvar h = head();
    buf = h.pack_(buf, size, pos);
    buf = h_.hm->m->pack_(buf, size, pos);
    break;
  }
  case SequenceMap:{
    buf[pos++] = SequenceMap;
    buf = h_.sm->s->pack_(buf, size, pos);
    buf = h_.sm->m->pack_(buf, size, pos);
    break;
  }
  case HeadSequenceMap:{
    buf[pos++] = HeadSequenceMap;
    nvar h = head();
    buf = h.pack_(buf, size, pos);
    buf = h_.sm->s->pack_(buf, size, pos);
    buf = h_.sm->m->pack_(buf, size, pos);
    break;
  }
  case Reference:
    buf[pos++] = Reference;
    buf = h_.ref->v->pack_(buf, size, pos);
    break;
  case Pointer:
    buf[pos++] = Pointer;
    buf = h_.vp->pack_(buf, size, pos);
    break;
  default:
    assert(false);
  }
  
  return buf;
}

void nvar::unpack(char* buf, size_t size, bool compressed){
  assert(t_ == Undefined);

  size_t pos = 0;

  if(compressed){
    int psize = size*2;
    char* pbuf = (char*)malloc(psize);

    pbuf = zlib_decompress_(buf, size, pbuf, &psize, true);
    unpack_(pbuf, pos);
    free(pbuf);
  }
  else{
    unpack_(buf, pos);
  }
}

void nvar::unpack_(char* buf, size_t& pos){
  Type t = buf[pos++];

  switch(t){
  case Undefined:
    break;
  case False:
    t_ = False;
    break;
  case True:
    t_ = True;
    break;
  case Pack0:
    t_ = Integer;
    h_.i = 0;
    break;
  case Pack1:
    t_ = Integer;
    h_.i = 1;
    break;
  case Pack2:
    t_ = Integer;
    h_.i = 2;
    break;
  case Pack3:
    t_ = Integer;
    h_.i = 3;
    break;
  case Pack4:
    t_ = Integer;
    h_.i = 4;
    break;
  case Pack5:
    t_ = Integer;
    h_.i = 5;
    break;
  case Pack6:
    t_ = Integer;
    h_.i = 6;
    break;
  case Pack7:
    t_ = Integer;
    h_.i = 7;
    break;
  case Pack8:
    t_ = Integer;
    h_.i = 8;
    break;
  case Pack9:
    t_ = Integer;
    h_.i = 9;
    break;
  case Pack10:
    t_ = Integer;
    h_.i = 10;
    break;
  case PackInt8:{
    t_ = Integer;
    h_.i = *(uint8_t*)(buf + pos);
    ++pos;
    break;
  }
  case PackInt16:{
    uint16_t i;
    memcpy(&i, buf + pos, 2);
    pos += 2;
    t_ = Integer;
    h_.i = i;
    break;
  }
  case PackInt32:{
    uint32_t i;
    memcpy(&i, buf + pos, 4);
    pos += 4;
    t_ = Integer;
    h_.i = i;
    break;
  }
  case Integer:{
    uint64_t i;
    memcpy(&i, buf + pos, 8);
    pos += 8;
    t_ = Integer;
    h_.i = i;
    break;
  }
  case Rational:{
    int64_t n;
    memcpy(&n, buf + pos, 8);
    pos += 8;
    
    int64_t d;
    memcpy(&d, buf + pos, 8);
    pos += 8;

    t_ = Rational;
    h_.r = new nrat(n, d);
    break;
  }
  case PackFloat32:{
    float f;
    memcpy(&f, buf + pos, 4);
    pos += 4;
    t_ = Float;
    h_.d = f;
    break;
  }
  case Float:{
    double d;
    memcpy(&d, buf + pos, 8);
    pos += 8;
    t_ = Float;
    h_.d = d;
    break;
  }
  case Real:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;
    nstr s;
    s.insert(0, buf + pos, len);
    pos += len;
    t_ = Real;
    h_.x = new nreal(s.c_str());
    break;
  }
  case Symbol:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;
    t_ = Symbol;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case PackLongSymbol:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;
    t_ = Symbol;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case PackShortString:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;
    t_ = String;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case String:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;
    t_ = String;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case PackLongString:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;
    t_ = String;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case Binary:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;
    t_ = Binary;
    h_.s = new nstr;
    h_.s->insert(0, buf + pos, len);
    pos += len;
    break;
  }
  case PackShortVector:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;
    t_ = Vector;
    h_.v = new nvec(len);
    nvec& v = *h_.v;
    for(size_t i = 0; i < len; ++i){
      v[i].unpack_(buf, pos);
    }
    break;
  }
  case Vector:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;
    t_ = Vector;
    h_.v = new nvec(len);
    nvec& v = *h_.v;
    for(size_t i = 0; i < len; ++i){
      v[i].unpack_(buf, pos);
    }
    break;
  }
  case PackLongVector:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;
    t_ = Vector;
    h_.v = new nvec(len);
    nvec& v = *h_.v;
    for(size_t i = 0; i < len; ++i){
      v[i].unpack_(buf, pos);
    }
    break;
  }
  case PackShortList:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;
    t_ = List;
    h_.l = new nlist(len);
    nlist& l = *h_.l;
    for(auto& itr : l){
      itr.unpack_(buf, pos);
    }
    break;
  }
  case List:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;
    t_ = List;
    h_.l = new nlist(len);
    nlist& l = *h_.l;
    for(auto& itr : l){
      itr.unpack_(buf, pos);
    }
    break;
  }
  case PackLongList:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;
    t_ = List;
    h_.l = new nlist(len);
    nlist& l = *h_.l;
    for(auto& itr : l){
      itr.unpack_(buf, pos);
    }
    break;
  }
  case Function:{
    uint8_t slen = *(uint8_t*)(buf + pos);
    ++pos;
    
    nstr f;
    f.insert(0, buf + pos, slen);
    pos += slen;

    uint8_t n = *(uint8_t*)(buf + pos);
    ++pos;

    t_ = Function;
    h_.f = new CFunction(move(f), n);
    nvec& v = h_.f->v;

    for(size_t i = 0; i < n; ++i){
      v[i].unpack_(buf, pos);
    }

    uint8_t mlen;
    memcpy(&mlen, buf + pos, 1);
    ++pos;

    if(mlen > 0){
      nmap* m = new nmap;
      h_.f->m = m;

      for(size_t i = 0; i < mlen; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        m->emplace(move(k), move(v));
      }      
    }

    break;
  }
  case PackLongFunction:{
    uint32_t slen;
    memcpy(&slen, buf + pos, 4);
    pos += 4;
    
    nstr f;
    f.insert(0, buf + pos, slen);
    pos += slen;

    uint32_t n;
    memcpy(&n, buf + pos, 4);
    pos += 4;

    t_ = Function;
    h_.f = new CFunction(move(f), n);
    nvec& v = h_.f->v;

    for(size_t i = 0; i < n; ++i){
      v[i].unpack_(buf, pos);
    }

    uint32_t mlen;
    memcpy(&mlen, buf + pos, 4);
    pos += 4;

    if(mlen > 0){
      nmap* m = new nmap;
      h_.f->m = m;

      for(size_t i = 0; i < mlen; ++i){
        nvar k;
        k.unpack_(buf, pos);
        
        nvar v;
        v.unpack_(buf, pos);
        
        m->emplace(move(k), move(v));
      }      
    }

    break;
  }
  case HeadSequence:{
    nvar* h = new nvar;
    h->unpack_(buf, pos);

    nvar* s = new nvar;
    s->unpack_(buf, pos);

    t_ = HeadSequence;
    h_.hs = new CHeadSequence(h, s);
    break;
  }
  case PackShortMap:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;

    t_ = Map;
    h_.m = new nmap;
    nmap& m = *h_.m;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      m.emplace(move(k), move(v));
    }
    break;
  }
  case Map:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;

    t_ = Map;
    h_.m = new nmap;
    nmap& m = *h_.m;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      m.emplace(move(k), move(v));
    }
    break;
  }
  case PackLongMap:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;

    t_ = Map;
    h_.m = new nmap;
    nmap& m = *h_.m;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      m.emplace(move(k), move(v));
    }
    break;
  }
  case PackShortMultimap:{
    uint8_t len = *(uint8_t*)(buf + pos);
    ++pos;

    t_ = Multimap;
    h_.mm = new nmmap;
    nmmap& mm = *h_.mm;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      mm.emplace(move(k), move(v));
    }
    break;
  }
  case Multimap:{
    uint16_t len;
    memcpy(&len, buf + pos, 2);
    pos += 2;

    t_ = Multimap;
    h_.mm = new nmmap;
    nmmap& mm = *h_.mm;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      mm.emplace(move(k), move(v));
    }
    break;
  }
  case PackLongMultimap:{
    uint32_t len;
    memcpy(&len, buf + pos, 4);
    pos += 4;

    t_ = Multimap;
    h_.mm = new nmmap;
    nmmap& mm = *h_.mm;

    for(size_t i = 0; i < len; ++i){
      nvar k;
      k.unpack_(buf, pos);

      nvar v;
      v.unpack_(buf, pos);

      mm.emplace(move(k), move(v));
    }
    break;
  }
  case HeadMap:{
    nvar* h = new nvar;
    h->unpack_(buf, pos);

    nvar* m = new nvar;
    m->unpack_(buf, pos);

    t_ = HeadMap;
    h_.hm = new CHeadMap(h, m);
    break;
  }
  case SequenceMap:{
    nvar* s = new nvar;
    s->unpack_(buf, pos);

    nvar* m = new nvar;
    m->unpack_(buf, pos);

    t_ = SequenceMap;
    h_.sm = new CSequenceMap(s, m);
    break;
  }
  case HeadSequenceMap:{
    nvar* h = new nvar;
    h->unpack_(buf, pos);

    nvar* s = new nvar;
    s->unpack_(buf, pos);

    nvar* m = new nvar;
    m->unpack_(buf, pos);

    t_ = HeadSequenceMap;
    h_.hsm = new CHeadSequenceMap(h, s, m);
    break;
  }
  case Reference:{
    t_ = Reference;
    nvar* v = new nvar;
    v->unpack_(buf, pos);
    h_.ref = new CReference(v);
    break;
  }
  case Pointer:{
    t_ = Reference;
    nvar* v = new nvar;
    v->unpack_(buf, pos);
    h_.vp = v;
    break;
  }
  default:
    NERROR("unpack error");
  }
}

void nvar::save(const nstr& path) const{
  nstr tempPath = NSys::tempFilePath();

  FILE* file = fopen(tempPath.c_str(), "wb");

  if(!file){
    NERROR("failed to create file: " + tempPath);
  }

  size_t len;
  char* buf = pack(len);
  size_t n = fwrite(buf, 1, len, file);

  fclose(file);
  free(buf);

  if(n != len){
    remove(tempPath.c_str());
    NERROR("failed to write file: " + tempPath);
  }

  if(!NSys::rename(tempPath, path)){
    remove(tempPath.c_str());
    NERROR("failed to move file into place: " + path);
  }
}

void nvar::open(const nstr& path){
  FILE* file = fopen(path.c_str(), "rb");

  if(!file){
    NERROR("failed to open file: " + path);
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);

  if(size < 0){
    NERROR("[1] failed to read file: " + path);
  }

  rewind(file);

  char* buf = (char*)malloc(size);

  size_t n = fread(buf, 1, size, file);
  fclose(file);

  if(n < size){
    NERROR("[2] failed to read file: " + path);
  }

  unpack(buf, size);

  free(buf);
}

void nvar::sqrt(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Sqrt") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::sqrt(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Sqrt") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::sqrt(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::sqrt(h_.d);
    break;
  case Real:
    *h_.x = nreal::sqrt(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Sqrt") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].sqrt(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->sqrt(o);
    break;
  case HeadMap:
    h_.hm->h->sqrt(o);
    break;
  case SequenceMap:
    h_.sm->s->sqrt(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->sqrt(o);
    break;
 case Reference:
   h_.ref->v->sqrt(o);
   break;
 case Pointer:
   h_.vp->sqrt(o);
   break;
  }
}

void nvar::exp(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Exp") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::exp(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Exp") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::exp(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::exp(h_.d);
    break;
  case Real:
    *h_.x = nreal::exp(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Exp") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].exp(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->exp(o);
    break;
  case HeadMap:
    h_.hm->h->exp(o);
    break;
  case SequenceMap:
    h_.sm->s->exp(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->exp(o);
    break;
 case Reference:
   h_.ref->v->exp(o);
   break;
 case Pointer:
   h_.vp->exp(o);
   break;
  }
}

nvar nvar::pow(const nvar& x, NObject* o) const{
  switch(t_){
  case None:
  case Undefined:
    NERROR("left operand is undefined");
  case False:
  case True:
    NERROR("left operand is invalid");
  case Integer:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return std::pow(h_.i, x.h_.i);
    case Rational:
      if(o){
        return o->process(nfunc("Pow") << h_.i << *x.h_.r, NObject::Delegated);
      }

      return std::pow(h_.i, x.h_.r->toDouble());
    case Float:
      return std::pow(h_.i, x.h_.d);
    case Real:
      return nreal::pow(h_.i, *x.h_.x);
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Pow");
      h.f->v.push_back(h_.i);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case HeadSequence:
      return pow(*x.h_.hs->h, o);
    case HeadMap:
      return pow(*x.h_.hm->h, o);
    case HeadSequenceMap:
      return pow(*x.h_.hsm->h, o);
    case Reference:
      return pow(*x.h_.ref->v, o);
    case Pointer:
      return pow(*x.h_.vp, o);
    default:
      NERROR("invalid operands");
    }
  case Rational:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      if(o){
        return o->process(nfunc("Pow") << *h_.r << x.h_.i, NObject::Delegated);
      }

      return std::pow(h_.r->toDouble(), x.h_.i);
    case Rational:
      if(o){
        return o->process(nfunc("Pow") << *h_.r << *x.h_.r, NObject::Delegated);
      }

      return std::pow(h_.r->toDouble(), x.h_.r->toDouble());
    case Float:
      return std::pow(h_.r->toDouble(), x.h_.d);
    case Real:
      return nreal::pow(*h_.r, *x.h_.x);
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Pow");
      h.f->v.push_back(*h_.r);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case HeadSequence:
      return pow(*x.h_.hs->h, o);
    case HeadMap:
      return pow(*x.h_.hm->h, o);
    case HeadSequenceMap:
      return pow(*x.h_.hsm->h, o);
    case Reference:
      return pow(*x.h_.ref->v, o);
    case Pointer:
      return pow(*x.h_.vp, o);
    default:
      NERROR("invalid operands");
    } 
  case Float:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return std::pow(h_.d, x.h_.i);
    case Rational:
      return std::pow(h_.d, x.h_.r->toDouble());
    case Float:
      return std::pow(h_.d, x.h_.d);
    case Real:
      return std::pow(h_.d, x.h_.x->toDouble());
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Pow");
      h.f->v.push_back(h_.d);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case HeadSequence:
      return pow(*x.h_.hs->h, o);
    case HeadMap:
      return pow(*x.h_.hm->h, o);
    case HeadSequenceMap:
      return pow(*x.h_.hsm->h, o);
    case Reference:
      return pow(*x.h_.ref->v);
    case Pointer:
      return pow(*x.h_.vp, o);
    default:
      NERROR("invalid operands");
    }
  case Real:
    switch(x.t_){
    case None:
    case Undefined:
      NERROR("right operand is undefined");
    case Integer:
      return nreal::pow(*h_.x, x.h_.i); 
    case Rational:
      return nreal::pow(*h_.x, *x.h_.r); 
    case Float:
      return std::pow(h_.x->toDouble(), x.h_.d);
    case Real:
      return nreal::pow(*h_.x, *x.h_.x); 
    case Function:
    case Symbol:{
      Head h;
      h.f = new CFunction("Pow");
      h.f->v.push_back(*h_.x);
      h.f->v.push_back(new nvar(x, Copy));
      return new nvar(Function, h);
    }
    case HeadSequence:
      return pow(*x.h_.hs->h, o);
    case HeadMap:
      return pow(*x.h_.hm->h, o);
    case HeadSequenceMap:
      return pow(*x.h_.hsm->h, o);
    case Reference:
      return pow(*x.h_.ref->v);
    case Pointer:
      return pow(*x.h_.vp, o);
    default:
      NERROR("invalid operands");
    }  
  case Symbol:
  case Function:{
    Head h;
    h.f = new CFunction("Pow");
    h.f->v.emplace_back(new nvar(*this, Copy));
    h.f->v.emplace_back(nvar(x, Copy));

    return new nvar(Function, h);
  }
  case HeadSequence:
    return h_.hs->h->pow(x, o);
  case HeadMap:
    return h_.hm->h->pow(x, o);
  case HeadSequenceMap:
    return h_.hsm->h->pow(x, o);
  case Reference:
    return h_.ref->v->pow(x);
  case Pointer:
    return h_.vp->pow(x);
  default:
    NERROR("left operand is invalid");
  }  
}

void nvar::abs(){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    h_.i = std::abs(h_.i);
    break;
  case Rational:
    if(*h_.r < 0){
      *h_.r = - *h_.r;
    }
    break;
  case Float:
    h_.d = std::abs(h_.d);
    break;
  case Real:
    *h_.x = nreal::abs(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Abs") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].abs();
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->abs();
    break;
  case HeadMap:
    h_.hm->h->abs();
    break;
  case SequenceMap:
    h_.sm->s->abs();
    break;
  case HeadSequenceMap:
    h_.hsm->s->abs();
    break;
 case Reference:
   h_.ref->v->abs();
   break;
 case Pointer:
   h_.vp->abs();
   break;
  }
}

void nvar::log10(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Log10") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::log10(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Log10") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::log10(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::log10(h_.d);
    break;
  case Real:
    *h_.x = nreal::log10(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Log10") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].log10(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->log10(o);
    break;
  case HeadMap:
    h_.hm->h->log10(o);
    break;
  case SequenceMap:
    h_.sm->s->log10(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->log10(o);
    break;
 case Reference:
   h_.ref->v->log10(o);
   break;
 case Pointer:
   h_.vp->log10(o);
   break;
  }
}

void nvar::log(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Log") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::log(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Log") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::log(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::log(h_.d);
    break;
  case Real:
    *h_.x = nreal::log(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Log") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].log(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->log(o);
    break;
  case HeadMap:
    h_.hm->h->log(o);
    break;
  case SequenceMap:
    h_.sm->s->log(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->log(o);
    break;
 case Reference:
   h_.ref->v->log(o);
   break;
 case Pointer:
   h_.vp->log(o);
   break;
  }
}

void nvar::cos(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Cos") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::cos(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Cos") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::cos(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::cos(h_.d);
    break;
  case Real:
    *h_.x = nreal::cos(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Cos") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].cos(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->cos(o);
    break;
  case HeadMap:
    h_.hm->h->cos(o);
    break;
  case SequenceMap:
    h_.sm->s->cos(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->cos(o);
    break;
 case Reference:
   h_.ref->v->cos(o);
   break;
 case Pointer:
   h_.vp->cos(o);
   break;
  }
}

void nvar::acos(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Acos") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::acos(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Acos") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::acos(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::acos(h_.d);
    break;
  case Real:
    *h_.x = nreal::acos(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Acos") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].acos(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->acos(o);
    break;
  case HeadMap:
    h_.hm->h->acos(o);
    break;
  case SequenceMap:
    h_.sm->s->acos(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->acos(o);
    break;
 case Reference:
   h_.ref->v->acos(o);
   break;
 case Pointer:
   h_.vp->acos(o);
   break;
  }
}

void nvar::cosh(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Cosh") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::cosh(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Cosh") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::cosh(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::cosh(h_.d);
    break;
  case Real:
    *h_.x = nreal::cosh(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Cosh") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].cosh(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->cosh(o);
    break;
  case HeadMap:
    h_.hm->h->cosh(o);
    break;
  case SequenceMap:
    h_.sm->s->cosh(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->cosh(o);
    break;
 case Reference:
   h_.ref->v->cosh(o);
   break;
 case Pointer:
   h_.vp->cosh(o);
   break;
  }
}

void nvar::sin(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Sin") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::sin(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Sin") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::sin(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::sin(h_.d);
    break;
  case Real:
    *h_.x = nreal::sin(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Sin") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].sin(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->sin(o);
    break;
  case HeadMap:
    h_.hm->h->sin(o);
    break;
  case SequenceMap:
    h_.sm->s->sin(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->sin(o);
    break;
 case Reference:
   h_.ref->v->sin(o);
   break;
 case Pointer:
   h_.vp->sin(o);
   break;
  }
}

void nvar::asin(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Asin") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::asin(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Asin") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::asin(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::asin(h_.d);
    break;
  case Real:
    *h_.x = nreal::asin(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Asin") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].asin(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->asin(o);
    break;
  case HeadMap:
    h_.hm->h->asin(o);
    break;
  case SequenceMap:
    h_.sm->s->asin(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->asin(o);
    break;
 case Reference:
   h_.ref->v->asin(o);
   break;
 case Pointer:
   h_.vp->asin(o);
   break;
  }
}

void nvar::sinh(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Sinh") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::sinh(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Sinh") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::sinh(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::sinh(h_.d);
    break;
  case Real:
    *h_.x = nreal::sinh(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Sinh") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].sinh(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->sinh(o);
    break;
  case HeadMap:
    h_.hm->h->sinh(o);
    break;
  case SequenceMap:
    h_.sm->s->sinh(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->sinh(o);
    break;
 case Reference:
   h_.ref->v->sinh(o);
   break;
 case Pointer:
   h_.vp->sinh(o);
   break;
  }
}

void nvar::tan(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Tan") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::tan(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Tan") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::tan(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::tan(h_.d);
    break;
  case Real:
    *h_.x = nreal::tan(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Tan") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].tan(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->tan(o);
    break;
  case HeadMap:
    h_.hm->h->tan(o);
    break;
  case SequenceMap:
    h_.sm->s->tan(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->tan(o);
    break;
 case Reference:
   h_.ref->v->tan(o);
   break;
 case Pointer:
   h_.vp->tan(o);
   break;
  }
}

void nvar::atan(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Atan") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::atan(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Atan") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::atan(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::atan(h_.d);
    break;
  case Real:
    *h_.x = nreal::atan(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Atan") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].atan(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->atan(o);
    break;
  case HeadMap:
    h_.hm->h->atan(o);
    break;
  case SequenceMap:
    h_.sm->s->atan(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->atan(o);
    break;
 case Reference:
   h_.ref->v->atan(o);
   break;
 case Pointer:
   h_.vp->atan(o);
   break;
  }
}

void nvar::tanh(NObject* o){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    if(o){
      *this = o->process(nfunc("Tanh") << *this, NObject::Delegated);
      break;
    }

    h_.d = h_.i;
    h_.d = std::tanh(h_.d);
    t_ = Float;
    break;
  case Rational:{
    if(o){
      *this = o->process(nfunc("Tanh") << *this, NObject::Delegated);
      break;
    }

    double d = h_.r->toDouble();
    delete h_.r;
    h_.d = std::tanh(d);
    t_ = Float;
    break;
  }
  case Float:
    h_.d = std::tanh(h_.d);
    break;
  case Real:
    *h_.x = nreal::tanh(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Tanh") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].tanh(o);
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->tanh(o);
    break;
  case HeadMap:
    h_.hm->h->tanh(o);
    break;
  case SequenceMap:
    h_.sm->s->tanh(o);
    break;
  case HeadSequenceMap:
    h_.hsm->s->tanh(o);
    break;
 case Reference:
   h_.ref->v->tanh(o);
   break;
 case Pointer:
   h_.vp->tanh(o);
   break;
  }
}

void nvar::floor(){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    break;
  case Rational:
    h_.i = h_.r->numerator() / h_.r->denominator();
    break;
  case Float:
    h_.d = std::floor(h_.d);
    break;
  case Real:
    *h_.x = nreal::floor(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Floor") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].floor();
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->floor();
    break;
  case HeadMap:
    h_.hm->h->floor();
    break;
  case SequenceMap:
    h_.sm->s->floor();
    break;
  case HeadSequenceMap:
    h_.hsm->s->floor();
    break;
 case Reference:
   h_.ref->v->floor();
   break;
 case Pointer:
   h_.vp->floor();
   break;
  }
}

void nvar::ceil(){
  switch(t_){
  case None:
  case Undefined:
    NERROR("operand is undefined");
  case False:
  case True:
    NERROR("operand is invalid");
  case Integer:
    break;
  case Rational:{
    int64_t n = h_.r->numerator();
    int64_t d = h_.r->denominator();

    h_.i = n / d + n % d > 0 ? 1 : 0;
    t_ = Integer;
    break;
  }
  case Float:
    h_.d = std::ceil(h_.d);
    break;
  case Real:
    *h_.x = nreal::ceil(*h_.x);
    break;
  case Symbol:
  case Function:
    *this = nfunc("Ceil") << move(*this);
    break;
  case Vector:{
    nvec& v = *h_.v;
    size_t size = v.size();
    for(size_t i = 0; i < size; ++i){
      v[i].ceil();
    }
    break;
  }
  case HeadSequence:
    h_.hs->s->ceil();
    break;
  case HeadMap:
    h_.hm->h->ceil();
    break;
  case SequenceMap:
    h_.sm->s->ceil();
    break;
  case HeadSequenceMap:
    h_.hsm->s->ceil();
    break;
 case Reference:
   h_.ref->v->ceil();
   break;
 case Pointer:
   h_.vp->ceil();
   break;
  }
}
