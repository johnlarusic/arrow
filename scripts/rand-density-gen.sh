#!/bin/bash

for maxcost in 300 600 900 1200 1500 1800 2100 2400 2700 3000
do
    python rand-density.py -n den200.$maxcost -s 200 -r $maxcost -l 0 -u $maxcost > den200.$maxcost.tsp
done