#! /bin/bash

if [ $# -ne 2 ]; then
echo "Error in usage: ./run_FHMM1.sh in_fin.txt out_fin.txt"
exit 1
fi

Infile=$1
Outfile=$2

./Binary/FingeringHMM1_Run ./Code/param_FHMM1.txt ${Infile} ${Outfile} 0.964 -5

