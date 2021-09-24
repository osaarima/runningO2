#!/bin/bash

set -x

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c $TMPDIR/ampt_job_${SLURM_ARRAY_TASK_ID}/exec

echo "Run root macro.."

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -q -l "${amptrundir}/AmptToNTuple.C(${amptTempDir}/ana, ${amptTempDir}/ana/ampt.root, ${frstEvt})"

#rm ${amptTempDir}/ana/ampt.dat

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${Nevt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/projappl/project_2003583/simO2/runningO2/ampt-standalone-run/gengen.C;GeneratorExternal.funcName=gengen("../testdir/ampt-output00.root", "ampt")'

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${evt} -m FV0 FT0 ITS PIPE MFT -g external --configKeyValues 'GeneratorExternal.fileName=/alicesw/O2/run/SimExamples/AliRoot_AMPT/aliroot_ampt.macro;GeneratorExternal.funcName=ampt(5500., 6.99, 8.56, 4 )'

echo "Done!"
