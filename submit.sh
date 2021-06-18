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
#SBATCH --time=1:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=6000
#SBATCH --mail-type=END #uncomment to enable mail
#SBATCH --array=1-100 #defines SLURM_ARRAY_TASK_ID
# If you change output/error here, please change
# the cp command at the end of this macro
#SBATCH --output=logs/output_%A-run%a.txt
#SBATCH --error=logs/errors_%A-run%a.txt

## 5 AMPT events: ~20 min

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
#4Gigs of ram were not enough always for 4-cpu 100 evts runs.

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
    echo "Please give the full path of the macro that will be used inside the container."
    return 0
fi

if [[ $5 != /* ]]
then
    echo "Please give the full path of the macro that will be used inside the container."
    return 0
fi

#nevents=$( echo ${inputfiles[$i]#*_n} | grep -o -E '[0-9]+' )

comment=$1
nevents=$2
digitizerHz=$3
digitizerComment=$4
insideMacro=$5

#nCPUS=1
collSystem=PbPb
o2Version=21-06-10
dig=${digitizerHz}Hz-${digitizerComment}

n=$SLURM_ARRAY_TASK_ID
outputdir=/scratch/project_2003583/simO2_outputs/${collSystem}_o2ver-${o2Version}_${comment}/sim/run_job$n
outputdirDigit=/scratch/project_2003583/simO2_outputs/${collSystem}_o2ver-${o2Version}_${comment}/dig_${dig}/run_job$n

echo "Starting Slurm array job ${SLURM_ARRAY_JOB_ID}, task ${SLURM_ARRAY_TASK_ID}"

mkdir -p $outputdir
mkdir -p $outputdirDigit
mkdir -p ${outputdir}/logs
/projappl/project_2003583/simO2/runningO2/run $outputdir $outputdirDigit $nevents $digitizerHz $insideMacro
sleep 1
cp logs/output_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
cp logs/errors_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
sleep 1
