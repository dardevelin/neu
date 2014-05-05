#ifndef FOO_H
#define FOO_H

#include <neu/nvar.h>
#include <neu/NObject.h>

namespace neu{

class Foo : public NObject{
public:
  Foo();

  Foo(NBroker* broker);

  ndist bar();

  ndist baz(int x);

  NFunc handle(const nvar& n, uint32_t flags);

private:
  class Foo_* x_;
};

} // end namespace neu

#endif // FOO_H
