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
#SBATCH --time=4:00:00
#SBATCH --ntasks=1
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=18000
#SBATCH --mail-type=END #uncomment to enable mail
#SBATCH --array=1-2 #defines SLURM_ARRAY_TASK_ID
# If you change output/error here, please change
# the cp command at the end of this macro
#SBATCH --output=logs/output_%A-run%a.txt
#SBATCH --error=logs/errors_%A-run%a.txt

if [ "$1" == "-h" ]
then
    echo "Usage: `basename $0` comment nevents[=1] energy[=5500 (give in GeV)] bmin[=6.99 (give in fm)] bmax[=8.56 (give in fm)] seedbase[=1000]"
    exit 0
fi

if [ -z "$1" ]
then
    echo "Please give a comment to make this run unique (check `basename $0` -h for help)"
    exit 0
fi

source setup.sh

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
    bmin=6.99
else
    bmin=$4
fi

if [ -z "$5" ]
then
    bmax=8.56
else
    bmax=$5
fi

if [ -z "$6" ]
then
    seedbase=1000
else
    seedbase=$6
fi

echo "Starting Slurm array job ${SLURM_ARRAY_JOB_ID}, task ${SLURM_ARRAY_TASK_ID}"

collSystem=PbPb
o2Version=21-09-20
dig=${digitizerHz}Hz-${digitizerComment}

n=$SLURM_ARRAY_TASK_ID

outputdir=/scratch/project_2003583/simO2_outputs/${collSystem}_StandaloneAMPT_o2ver-${o2Version}_${comment}_${energy}GeV/sim/run_job$n
outputdirDigit=/scratch/project_2003583/simO2_outputs/${collSystem}_StandaloneAMPT_o2ver-${o2Version}_${comment}_${energy}GeV/dig_${dig}/run_job$n

mkdir $outputdir
#mkdir -p $outputdirDigit
mkdir ${outputdir}/logs

eventfirst=$(((${n}-1)*nevents))
run $eventfirst $nevents $energy $bmin $bmax $outputdir $outputdirDigit $seedbase

sleep 1
mv logs/output_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
mv logs/errors_${SLURM_ARRAY_JOB_ID}-run${SLURM_ARRAY_TASK_ID}.txt $outputdir/logs/
