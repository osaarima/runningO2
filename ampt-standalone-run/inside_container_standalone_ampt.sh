#!/bin/bash

set -x

cd $amptTempDir

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c ${amptTempDir}/exec

echo "Run root macro.."

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -q -l "${amptrundir}/AmptToNTuple.C(\"${amptTempDir}/ana\", \"${amptTempDir}/ana/ampt.root\", ${frstEvt})"

cd $dir

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${Nevt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/projappl/project_2003583/simO2/runningO2/ampt-standalone-run/gengen.C;GeneratorExternal.funcName=gengen("${amptTempDir}/ana/ampt.root", "ampt")'

#source /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh

source /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh

cd $currentDir

echo "Done!"
