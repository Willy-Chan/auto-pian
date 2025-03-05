#! /bin/bash

mkdir ./Binary

g++ -O2 ./Code/FingeringHMM1_Train_v190308.cpp -o ./Binary/FingeringHMM1_Train
g++ -O2 ./Code/FingeringHMM1_Run_v181122.cpp -o ./Binary/FingeringHMM1_Run
g++ -O2 ./Code/FingeringHMM2_Train_v190308.cpp -o ./Binary/FingeringHMM2_Train
g++ -O2 ./Code/FingeringHMM2_Run_v181125.cpp -o ./Binary/FingeringHMM2_Run
g++ -O2 ./Code/FingeringHMM3_Train_v190308.cpp -o ./Binary/FingeringHMM3_Train
g++ -O2 ./Code/FingeringHMM3_Run_v181125.cpp -o ./Binary/FingeringHMM3_Run
g++ -O2 ./Code/CHMM_Train_v190320.cpp -o ./Binary/CHMM_Train
g++ -O2 ./Code/CHMM_Run_v190321.cpp -o ./Binary/CHMM_Run

g++ -O2 ./Code/Evaluate_SimpleMatchRate.cpp -o ./Binary/Evaluate_SimpleMatchRate
g++ -O2 ./Code/Evaluate_MultipleGroundTruth.cpp -o ./Binary/Evaluate_MultipleGroundTruth

