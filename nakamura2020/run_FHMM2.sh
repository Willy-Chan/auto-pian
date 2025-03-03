#! /bin/bash

if [ $# -ne 2 ]; then
echo "Error in usage: ./run_FHMM2.sh in_fin.txt out_fin.txt"
exit 1
fi

Infile=$1
Outfile=$2

./Binary/FingeringHMM2_Run ./Code/param_FHMM2.txt ${Infile} ${Outfile} 0.556 0.407 0.474 -5

