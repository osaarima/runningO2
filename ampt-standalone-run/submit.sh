#!/bin/bash
## sbatch will check arguments from the comments in the
## beginning of this file.
#SBATCH --job-name=simo2_SAmpt_osanmasa
#SBATCH --account=project_2003583
# partition explained here: https://docs.csc.fi/computing/running/batch-job-partitions/
# test = 15min, 80tasks,   2node,  382GiB max memory, 3600GiB max storage
# small= 3days, 40tasks,   1node,  382GiB max memory, 3600GiB max storage
# large= 3days, 1040tasks, 26node, 382GiB max memory, 3600GiB max storage
#SBATCH --partition=small
#SBATCH --time=48:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=8000
#SBATCH --gres=nvme:10
#SBATCH --mail-type=END #uncomment to enable mail
#SBATCH --array=1-100 #defines SLURM_ARRAY_TASK_ID
# If you change output/error here, please change
# the cp command at the end of this macro
#SBATCH --output=logs/output_%A-run%a.txt
#SBATCH --error=logs/errors_%A-run%a.txt

#This job uses the LOCAL_SCRATCH space for the ampt generated temporary
#files. 100 events took a total of 367M of space in the temp dir.
#The space is requested by --gres=nvme:2, and the number is in GB.

if [ "$1" == "-h" ]
then
    echo "Usage: `basename $0` comment nevents[=1] energy[=5500 (give in GeV)] digitizerHz[=5 (Hz)] digitizerComment[=default] bmin[=6.99 (give in fm)] bmax[=8.56 (give in fm)] seedbase[=1000]"
    exit 0
fi

if [ -z "$1" ]
then
    echo "Please give a comment to make this run unique (check `basename $0` -h for help)"
    exit 0
fi

comment=$1

if [ -z "$2" ]
then
    nevents=1
else
    nevents=$2
fi

if [ -z "$3" ]
then
    energy=5500
else
    energy=$3
fi

if [ -z "$4" ]
then
    digitizerHz=5
else
    digitizerHz=$4
fi

if [ -z "$5" ]
then
    digitizerComment=default
else
    digitizerComment=$5
fi

if [ -z "$6" ]
then
    bmin=6.99
else
    bmin=$6
fi

if [ -z "$7" ]
then
    bmax=8.56
else
    bmax=$7
fi

if [ -z "$8" ]
then
    seedbase=1000
else
    seedbase=$8
fi

source setup.sh

echo "Starting Slurm array job ${SLURM_ARRAY_JOB_ID}, task ${SLURM_ARRAY_TASK_ID}"

collSystem=PbPb
o2Version=21-10-12
dig=${digitizerHz}Hz-${digitizerComment}

n=$SLURM_ARRAY_TASK_ID

outputBasedir=/scratch/project_2003583/simO2_outputs/${collSystem}_StandaloneAMPT_o2ver-${o2Version}_${comment}_${energy}GeV
outputdir=${outputBasedir}/sim/run_job$n
outputdirDigit=${outputBasedir}/dig_${dig}/run_job$n

mkdir -p ${outputdir}/logs
mkdir -p $outputdirDigit
mkdir -p ${outputBasedir}/runScripts/ampt-standalone-run
mkdir -p ${outputBasedir}/runScripts/post

#Only need to do once.
if [ $n -eq 1 ]
then
    cp /projappl/project_2003583/simO2/runningO2/README_RUNNING /projappl/project_2003583/simO2/runningO2/inside_container_runDigitizer.sh /projappl/project_2003583/simO2/runningO2/inside_container_post-analysis.sh ${outputBasedir}/runScripts/
    cp /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/run /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/submit.sh /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/input.ampt /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/inside_container_standalone_ampt.sh /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/gengen.C /projappl/project_2003583/simO2/runningO2/ampt-standalone-run/AmptToNTuple.C ${outputBasedir}/runScripts/ampt-standalone-run/
    cp /projappl/project_2003583/simO2/runningO2/post/Clean_o2sim.C /projappl/project_2003583/simO2/runningO2/post/FillFV0Hits.C ${outputBasedir}/runScripts/post/
fi

eventfirst=$(((${n}-1)*nevents))
/projappl/project_2003583/simO2/runningO2/ampt-standalone-run/run $eventfirst $nevents $energy $digitizerHz $bmin $bmax $outputdir $outputdirDigit $seedbase

sleep 1
mv logs/output_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
mv logs/errors_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
