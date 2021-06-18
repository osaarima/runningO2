#!/bin/bash

PROG=`basename $0`
if [ $# -ne 2 ]
then
    echo "Usage: $PROG <simulation folder (not sim)> <filename for output>"
    return 0;
fi

simFolder=$1
fileName=$2

root -b -x -l -q "/projappl/project_2003583/simO2/post_analysis/FillPhiEtaHistograms.C(\"${simFolder}/${fileName}\", \"\",0,0,\"${simFolder}\")"; 
