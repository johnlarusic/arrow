#!/bin/bash

for size in 50 100 250 500
do
    for n in 1 2 3 4 5
    do
        python nozzle-vanes.py -n vane$size.$n -s $size -r $size$n > vane$size.$n.atsp
    done
done