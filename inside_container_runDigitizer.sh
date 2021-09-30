#!/bin/bash

#This macro requires that the $dir, $dirDigit, and $digHz are set.

set -x

cd $dir
ln -rfs * $dirDigit/
cd $dirDigit

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim-digitizer-workflow -b --onlyDet=FV0,FT0 --interactionRate $digHz

#Important: come back to the original directory
cd $dir
