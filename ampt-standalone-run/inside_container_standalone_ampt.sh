#!/bin/bash

set -x

cd $amptTempDir

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c ${amptTempDir}/exec

time=$(date)
echo "AMPT events completed: $time"

echo "Run root macro.."

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -q -l "${amptrundir}/AmptToNTuple.C(\"${amptTempDir}/ana\", \"${amptTempDir}/ana/ampt.root\", ${frstEvt})"

time=$(date)
echo "AMPT events transformed into root file: $time"

cd $dir

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${Nevt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/projappl/project_2003583/simO2/runningO2/ampt-standalone-run/gengen.C;GeneratorExternal.funcName=gengen("${amptTempDir}/ana/ampt.root", "ampt")'


#Digitizer has problems when running straight after standalone ampt. Please run separately using runningO2/submit.sh
#source /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh

source /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh

cd $currentDir

echo "Done!"
