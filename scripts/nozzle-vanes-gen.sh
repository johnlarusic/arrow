#!/bin/bash

for size in 50 100 250 500
do
    for n in 0 1 2 3 4 5 6 7 8 9
    do
        python nozzle-vanes.py -n vane$size.$n -s $size -r $size$n > vane$size.$n.atsp
    done
done