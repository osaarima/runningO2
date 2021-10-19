#!/bin/bash

set -x

#alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q "/projappl/project_2003583/simO2/runningO2/post/RemoveCkov.C(\"o2sim_Kine.root\")"

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q /projappl/project_2003583/simO2/runningO2/post/FillFV0Hits.C

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q "/projappl/project_2003583/simO2/runningO2/post/Clean_o2sim.C(\"./\", \"o2sim_Kine_PPOnly.root\", \"\")"

#rm ${dir}/o2sim_Kine.root

