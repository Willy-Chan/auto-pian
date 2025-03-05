#include<iostream>
#include<string>
#include<sstream>
#include<cmath>
#include<vector>
#include<algorithm>
#include<fstream>
#include<cassert>
#include"PianoFingering_v170101_2.hpp"
#include"BasicCalculation_v170122.hpp"
#include"FingeringEvaluation_v190306.hpp"
using namespace std;

int main(int argc, char** argv) {

	vector<int> v(100);
	vector<double> d(100);
	vector<string> s(100);
	stringstream ss;

	if(argc<4){cout<<"Error in usage: $./this nGT GT_1_fin.txt GT_2_fin.txt ... GT_nGT_fin.txt est_fingering.txt"<<endl; return -1;}
	int nGT=atoi(argv[1]);
	if(argc!=nGT+3){cout<<"Error in usage: $./this nGT GT_1_fin.txt GT_2_fin.txt ... GT_nGT_fin.txt est_fingering.txt"<<endl; return -1;}
	vector<string> GTFiles;
	for(int i=2;i<nGT+2;i+=1){GTFiles.push_back( string(argv[i]) );};
	string estFile=string(argv[nGT+2]);

	vector<PianoFingering> finsGT;
	PianoFingering finEst;

	finsGT.resize(nGT);
	for(int i=0;i<nGT;i+=1){finsGT[i].ReadFile(GTFiles[i]);};
	finEst.ReadFile(estFile);

	int nNotes=finEst.evts.size();

	int nMetrics=4;
	vector<double> metrics(nMetrics);
	//0: GeneralMatchRate
	//1: HighestMatchRate
	//2: SoftMatchRate
	//3: RecombinationMatchRate

	metrics[0]=AveragePairwiseMatchRate(finsGT,finEst);
	metrics[1]=(nNotes-MultiGTError(finsGT,finEst,1,10000,10000))/double(nNotes);
	metrics[2]=(nNotes-MultiGTError(finsGT,finEst,1,0,0))/double(nNotes);
	metrics[3]=(nNotes-MultiGTError(finsGT,finEst,1,1,10000))/double(nNotes);

cout<<"General,Highest,Soft,Recomb: ";
	for(int k=0;k<nMetrics;k+=1){
cout<<metrics[k]<<"\t";
	}//endfor k
cout<<endl;

	return 0;
}//end main


