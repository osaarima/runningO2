#!/bin/bash

set -x

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q "/projappl/project_2003583/simO2/post_analysis/RemoveCkov.C(\"o2sim_Kine.root\")"

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c root -b -x -l -q /projappl/project_2003583/simO2/post_analysis/FillFV0Hits.C

