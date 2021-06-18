#!/bin/bash

for inn in {1..100}; do root -q "/projappl/project_2003583/simO2/runningO2/post/FillFV0Hits.C(\"/scratch/project_2003583/simO2_outputs/run_5p5TeV_midcent_job${inn}/fv0-map${inn}.root\", \"/scratch/project_2003583/simO2_outputs/run_5p5TeV_midcent_job${inn}/o2sim_HitsFV0.root\")"; done
