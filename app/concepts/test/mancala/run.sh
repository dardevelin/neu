#/bin/sh

#./test -population 2000 -fill 0 -resetInterval 50000 -mergeRate 0.8 -verbose 1 -complexity 20 &> test.out

./test -population 3000 -fill 1 -resetInterval 200000 -mergeRate 0.8 -verbose 1 -complexity 40 -unusedBias 1.0 -restartRate 1.0
