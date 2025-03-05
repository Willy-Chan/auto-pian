** Contents **

A set of programs for piano fingering estimation, used in the following paper

Eita Nakamura, Yasuyuki Saito, Kazuyoshi Yoshii
Statistical Learning and Estimation of Piano Fingering 
Submitted to Information Sciences [arXiv:1904.10237]

Specifically the following programs are included:

Training and estimation algorithms for 1st-order HMM, 2nd-order HMM, 3rd-order HMM, and chord HMM
Evaluation script

** Compile **

The source code is written in C++.

./compile.sh

Or execute equivalent commants written in compile.sh.

The following binary programs should be produced in the 'Binary' folder.

FingeringHMM1_Train
FingeringHMM1_Run
FingeringHMM2_Train
FingeringHMM2_Run
FingeringHMM3_Train
FingeringHMM3_Run
CHMM_Train
CHMM_Run
Evaluate_SimpleMatchRate
Evaluate_MultipleGroundTruth

** Usage **

To run the fingering estimation programs with the default setting (the parameterization trained with the miscellaneous set),

./run_FHMM1.sh in_fin.txt out_fin.txt
./run_FHMM2.sh in_fin.txt out_fin.txt
./run_FHMM3.sh in_fin.txt out_fin.txt
./run_CHMM.sh in_fin.txt out_fin.txt

From top to bottom, the 1st-order HMM, 2nd-order HMM, 3rd-order HMM, and chord HMM. Input and output files should be given in the fingering file data format used in PIG Dataset. The fingering numbers in in_fin.txt will be replaced by the estimated fingering numbers in out_fin.txt.

To run the estimation programs with a custermized parametrization, execute directly the binary files in the Binary folder:
./Binary/FingeringHMM1_Run
./Binary/FingeringHMM2_Run
./Binary/FingeringHMM3_Run
./Binary/CHMM_Run

The required arguments will be shown when you execute the programs without any argument. Please also see individual files ./run_FHMM1.sh etc.

To train model parameters, the followng programs can be used:
./Binary/FingeringHMM1_Train list_train.txt DataFolder param
./Binary/FingeringHMM2_Train list_train.txt DataFolder param
./Binary/FingeringHMM3_Train list_train.txt DataFolder param
./Binary/CHMM_Train ./Code/ChordFinergingTemplates.txt list_train.txt DataFolder param
Training data must be a set of fingering files (for example, 1_fingering.txt, 2_fingering.txt, ....; file names should have the suffix "_fingering.txt"), which are assumed to be placed in a folder. DataFolder is a path to the folder. The list of fingering files should be written in list_train.txt, where the suffix should be removed (for example, 1, 2, ..., one in each line). Finally, param (or something else) is the name of the  parameter file (the output will be param.txt).

- Evaluation tool

To compute the simple match rate:
./Binary/Evaluate_SimpleMatchRate GT_fin.txt EST_fin.txt

To compute the match rates defined for multiple ground truths:
./Binary/Evaluate_MultipleGroundTruth nGT GT_1_fin.txt GT_2_fin.txt ... GT_nGT_fin.txt EST_fin.txt
where nGT is the number of ground truths.

** Contact **

Eita Nakamura
eita.nakamura@gmail.com
http://eita-nakamura.github.io/

