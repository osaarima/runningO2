#!/bin/bash

set -x

alienv setenv O2/latest-dev-o2,AliRoot/latest-master-o2 -c o2-sim -j 1 --seed ${seed} -n ${evt} -m FV0 FT0 ITS PIPE MFT -g pythia8hi --configKeyValues 'GeneratorPythia8.config=/projappl/project_2003583/simO2/runningO2/pythia8_hi.cfg'

source /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh

source /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh
