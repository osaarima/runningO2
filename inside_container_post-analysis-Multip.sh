#!/bin/bash

set -x

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q '/projappl/project_2003583/simO2/runningO2/post/Multi.C(".")'

