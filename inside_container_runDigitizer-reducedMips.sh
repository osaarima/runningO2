#!/bin/bash

#This macro requires that the $dir, $dirDigit, and $digHz are set.

set -x

cd $dir
ln -rfs * $dirDigit/
cd $dirDigit

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim-digitizer-workflow -b --onlyDet=FV0,FT0 --interactionRate $digHz --configKeyValues 'FV0DigParam.adcChannelsPerMip=8;FV0DigParam.adcChannelsPerMilivolt=1.142857143;FV0DigParam.normRingA1ToA4=3.95305165e-13;FV0DigParam.normRing5=4.01517935e-13'

#Important: come back to the original directory
cd $dir
