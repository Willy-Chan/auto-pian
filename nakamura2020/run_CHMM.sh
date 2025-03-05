#! /bin/bash

if [ $# -ne 2 ]; then
echo "Error in usage: ./run_CHMM.sh in_fin.txt out_fin.txt"
exit 1
fi

Infile=$1
Outfile=$2

./Binary/CHMM_Run ./Code/ChordFinergingTemplates.txt ./Code/param_CHMM1.txt ${Infile} ${Outfile} 0.94 4.70 7.53 5.29 0.10

