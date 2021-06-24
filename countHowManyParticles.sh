#!/bin/bash

for ((i=0; i<30; i++))
do
    lines=$(grep -Rnwi ./run_*/logs/output*.txt -e "MCtracks : ${i}," | wc -l)
    echo Run ${i}: ${lines}
    for ((j=0; j<${lines}; j++))
    do
        echo -n =
    done
    echo
done
