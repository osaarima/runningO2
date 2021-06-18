#!/bin/bash

echo "#alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${evt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/alicesw/O2/run/SimExamples/AliRoot_AMPT/aliroot_ampt.macro;GeneratorExternal.funcName=ampt(5500., 6.99, 8.56, 4 )'"

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${evt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/alicesw/O2/run/SimExamples/AliRoot_AMPT/aliroot_ampt.macro;GeneratorExternal.funcName=ampt(5500., 6.99, 8.56, 4 )'


echo "#alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim-digitizer-workflow -b --onlyDet=FV0,FT0 --interactionRate 5"

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim-digitizer-workflow -b --onlyDet=FV0,FT0 --interactionRate 5
