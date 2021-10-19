#!/bin/bash

#This might take some time so entering screen might be a good idea.

# We assume that the interactive mode is activated (for example with command)
#sinteractive --account project_2003583 --time 48:00:00 --mem 8000 --tmp 0
# and that singularity container has been entered:
#/projappl/project_2003583/run_singularity_docker.sh
# and that O2 environment has been entered with this command:
#alienv enter O2/latest-dev-o2,AliRoot/latest-master-o2

#100*100events took about 9 hours.

rootFolder=$1
digitizerFolder=$2
interactionRate=$3
howManyRuns=$4

for ((i=1; i<=${howManyRuns}; i++));
do
    echo ======== Digitize run $i ========
    cd ${rootFolder}/sim/run_job${i}
    ln -rfs * ${rootFolder}/${digitizerFolder}/run_job${i}
    cd ${rootFolder}/${digitizerFolder}/run_job${i}

    o2-sim-digitizer-workflow -b --onlyDet=FV0,FT0 --interactionRate $interactionRate
done
