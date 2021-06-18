#!/bin/bash

i=0
while :
do
    time=$(date)
    echo Round $i at $time
    pmap $1 | tail -n1 
    pmap $2 | tail -n1 
    pmap $3 | tail -n1 
    pmap $4 | tail -n1 
    ((i++))
    sleep 10
done
