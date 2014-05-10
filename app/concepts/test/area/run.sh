#/bin/sh

./test -population 3000 -fill 1 -resetInterval 50000 -mergeRate 0.8 -verbose 1 -complexity 30 &> run.out
