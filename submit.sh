#!/bin/bash
## sbatch usage: sbatch submit.sh <comment> <nEvts> <digitizer interaction rate Hz> <digitizer comment> <insideMacro.sh>
## sbatch will check arguments from the comments in the
## beginning of this file.
#SBATCH --job-name=simo2_osanmasa
#SBATCH --account=project_2003583
# partition explained here: https://docs.csc.fi/computing/running/batch-job-partitions/
# test = 15min, 80tasks,   2node,  382GiB max memory, 3600GiB max storage
# small= 3days, 40tasks,   1node,  382GiB max memory, 3600GiB max storage
# large= 3days, 1040tasks, 26node, 382GiB max memory, 3600GiB max storage
#SBATCH --partition=small
#SBATCH --time=0:30:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=5000
##SBATCH --mail-type=END #uncomment to enable mail
#SBATCH --array=1-50 #defines SLURM_ARRAY_TASK_ID
# If you change output/error here, please change
# the mv command at the end of this macro
#SBATCH --output=logs/output_%A-run%a.txt
#SBATCH --error=logs/errors_%A-run%a.txt

## 5 AMPT events: ~30 min

#time 8, ntasks 1, nodes 1, array 100, cpu 4, mem/cpu 4G
#Job ID: 5295754
#Array Job ID: 5295754_100
#Cluster: puhti
#User/Group: osanmasa/osanmasa
#State: COMPLETED (exit code 0)
#Nodes: 1
#Cores per node: 4
#CPU Utilized: 04:23:10
#CPU Efficiency: 17.42% of 1-01:10:40 core-walltime
#Job Wall-clock time: 06:17:40
#Memory Utilized: 14.17 GB
#Memory Efficiency: 90.68% of 15.62 GB
#Job consumed 35.01 CSC billing units based on following used resources
#Billed project: project_2003583
#CPU BU: 25.18
#Mem BU: 9.84
#
#Notes: 
#4Gigs/CPU of ram were not enough always for 4-cpu 100 evts runs.

#source setup.sh

if [ "$1" == "help" ]
then
    echo "Usage: `basename $0` <comment> <nEvts> <digitizer interaction rate Hz> <digitizer comment> <insideMacro.sh>"
    return 0
fi

if [ -z "$1" ]
then
    echo "Please give a comment to make this run unique (check '`basename $0` help' for help)"
    return 0
fi

#inputfiles=(input/cent20*/*)
if [ -z "$2" ]
then
    echo "Please give number of events"
    return 0
fi

if [ -z "$3" ]
then
    echo "Please give the interaction rate of the digitizer"
    return 0
fi

if [ -z "$4" ]
then
    echo "Please give a comment to describe what kind of digitizer you are using."
    return 0
fi

if [ -z "$5" ]
then
    echo "Please give seed base number."
    return 0
fi

if [ -z "$6" ]
then
    echo "Please give the full path of the macro that will be used inside the container."
    return 0
fi

if [[ $6 != /* ]]
then
    echo "Please give the full path of the macro that will be used inside the container."
    return 0
fi

#nevents=$( echo ${inputfiles[$i]#*_n} | grep -o -E '[0-9]+' )

comment=$1
nevents=$2
digitizerHz=$3
digitizerComment=$4
seedBase=$5
insideMacro=$6
energy=5500

#nCPUS=1
collSystem=PbPb
o2Version=21-10-12
dig=${digitizerHz}Hz-${digitizerComment}

#Comment this out if not using Standalone AMPT (this is used for the purposes of doing post analysis or digitizer only.)
standaloneFlag=StandaloneAMPT_

n=$SLURM_ARRAY_TASK_ID
outputBasedir=/scratch/project_2003583/simO2_outputs/${collSystem}_${standaloneFlag}o2ver-${o2Version}_${comment}_${energy}GeV
outputdir=${outputBasedir}/sim/run_job$n
outputdirDigit=${outputBasedir}/dig_${dig}/run_job$n

echo "Starting Slurm array job ${SLURM_ARRAY_JOB_ID}, task ${SLURM_ARRAY_TASK_ID}"

mkdir -p $outputdir
mkdir -p $outputdirDigit
mkdir -p ${outputdir}/logs
mkdir -p ${outputBasedir}/runScripts/post

#Only need to do once.
if [ $n -eq 1 ]
then
	cp /projappl/project_2003583/simO2/runningO2/run /projappl/project_2003583/simO2/runningO2/submit.sh /projappl/project_2003583/simO2/runningO2/README_RUNNING /projappl/project_2003583/simO2/runningO2/post/Clean_o2sim.C /projappl/project_2003583/simO2/runningO2/post/FillFV0Hits.C /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh $insideMacro /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh ${outputBasedir}/runScripts/
	cp /projappl/project_2003583/simO2/runningO2/post/Clean_o2sim.C /projappl/project_2003583/simO2/runningO2/post/FillFV0Hits.C ${outputBasedir}/runScripts/post/
fi

/projappl/project_2003583/simO2/runningO2/run $outputdir $outputdirDigit $nevents $digitizerHz $seedBase $insideMacro
sleep 1
mv logs/output_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
mv logs/errors_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
sleep 1
