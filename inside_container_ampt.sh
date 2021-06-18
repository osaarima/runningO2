#!/bin/bash

set -x

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${evt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/alicesw/O2/run/SimExamples/AliRoot_AMPT/aliroot_ampt.macro;GeneratorExternal.funcName=ampt(5500., 6.99, 8.56, 4 )'

source /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh

source /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh
