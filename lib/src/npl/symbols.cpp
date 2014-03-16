#include <neu/nvar.h>

using namespace std;
using namespace neu;

void __forceNPLSymbols(){
  nvar x1;
  int64_t y1 = 2;
  nvar z1 = x1 + y1;
  
  nvar x2;
  double y2 = 2;
  nvar z2 = x2 + y2;
  
  int8_t* v3 = 0;
  nvar x3(v3, 1);
  
  int16_t* v4 = 0;
  nvar x4(v4, 1);
  
  int32_t* v5 = 0;
  nvar x5(v5, 1);
  
  int64_t* v6 = 0;
  nvar x6(v6, 1);
  
  float* v7 = 0;
  nvar x7(v7, 1);
  
  double* v8 = 0;
  nvar x8(v8, 1);
  
  int64_t x9 = 0;
  nvar v9;
  v9 = x9;
  
  double x10 = 0;
  nvar v10;
  v10 = x10;
  
  nvar x11;
  int64_t y11 = 2;
  nvar z11 = x11 - y11;
  
  nvar x12;
  double y12 = 2;
  nvar z12 = x12 - y12;
  
  nvar x13;
  int64_t y13 = 2;
  nvar z13 = x13 * y13;
  
  nvar x14;
  double y14 = 2;
  nvar z14 = x14 * y14;
  
  nvar x15;
  int64_t y15 = 2;
  nvar z15 = x15 / y15;
  
  nvar x16;
  double y16 = 2;
  nvar z16 = x16 / y16;
  
  nvar x17;
  int64_t y17 = 2;
  nvar z17 = x17 % y17;
  
  nvar x18;
  double y18 = 2;
  nvar z18 = x18 % y18;

  nvar x19;
  int64_t y19 = 2;
  x19 += y19;
  
  nvar x20;
  double y20 = 2;
  x20 += y20;
  
  int64_t x21 = x20.toLong();
  double x22 = x20.toDouble();
  
  nvar x23;
  int64_t y23 = 2;
  x23 -= y23;
  
  nvar x24;
  double y24 = 2;
  x24 -= y24;
  
  nvar x25;
  int64_t y25 = 2;
  x25 *= y25;
  
  nvar x26;
  double y26 = 2;
  x26 *= y26;
  
  nvar x27;
  int64_t y27 = 2;
  x27 /= y27;
  
  nvar x28;
  double y28 = 2;
  x28 /= y28;
  
  nvar x29;
  int64_t y29 = 2;
  x29 %= y29;
  
  nvar x30;
  double y30 = 2;
  x30 %= y30;
  
  nvar x31 = 2;
  nvar x32 = !x31;
  
  nvar x33 = x31 && x32;
  nvar x34 = x31 || x32;
  
  nvar x35 = 2;
  int64_t x36 = 9;
  double x37 = 3;
  bool b;
  
  b = x35 == x35;
  b = x35 == x36;
  b = x35 == x37;
  
  b = x35 != x35;
  b = x35 != x36;
  b = x35 != x37;
  
  b = x35 < x35;
  b = x35 < x36;
  b = x35 < x37;
  
  b = x35 <= x35;
  b = x35 <= x36;
  b = x35 <= x37;
  
  b = x35 > x35;
  b = x35 > x36;
  b = x35 > x37;
  
  b = x35 >= x35;
  b = x35 >= x36;
  b = x35 >= x37;
  
  nvar x38;
  int x39 = 1;
  nvar x40 = x38[x39];
  
  nvar x41;
  nvar x42;
  nvar x43 = x41[x42];
  
  size_t x44 = x41.size();
  
  nvar x45 = -x43;
  
  nvar x46;
  nvar x47;
  
  nvar x48 = nvar::pow(x46, x47);
  nvar x49 = nvar::sqrt(x46);
  nvar x50 = nvar::exp(x46);
  nvar x51 = nvar::log(x47);
  nvar x52 = nvar::floor(x48);
  
  nvar x53;
  x53("a") = 9;
  x53(x52) = 10;
  
  nvar x54("b");
  
  nvar x55 = 9;
  nvar x56;
  x56.pushBack(x55);
  
  nvar x;
  nvar y;
  
  x.touchMultimap();
  x.touchList();
  x.toBool();
  
  nvar ks1 = x.keys();
  
  x.pushFront(y);
  nvar b9 = x.popBack();
  nvar f = x.popFront();
  
  bool hk = x.hasKey(y);
  x.insert(0, y);
  x.clear();
  bool ep = x.empty();
  nvar b2 = x.back();
  nvar g = x.get(y);
  nvar g2 = x.get(y, g);
  x.erase(b);
  nvar c1 = nvar::cos(x);
  c1 = nvar::acos(x);
  c1 = nvar::cosh(x);
  c1 = nvar::sin(x);
  c1 = nvar::asin(x);
  c1 = nvar::sinh(x);
  c1 = nvar::tan(x);
  c1 = nvar::atan(x);
  c1 = nvar::tanh(x);
  
  x.merge(y);
  x.outerMerge(y);
}
