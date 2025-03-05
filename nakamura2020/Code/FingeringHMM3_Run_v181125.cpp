#include<iostream>
#include<string>
#include<sstream>
#include<cmath>
#include<vector>
#include<algorithm>
#include<fstream>
#include<cassert>
#include"FingeringHMM_v180925.hpp"
#include"ProbabilityVisualizer_v180920.hpp"

using namespace std;

int main(int argc, char** argv) {

	vector<int> v(100);
	vector<double> d(100);
	vector<string> s(100);
	stringstream ss;

	if(argc!=10){cout<<"Error in usage! : $./this param.txt in_fingering.txt out_fingering.txt w1(0.667) w2(0.5) w3(0.2) lam1(0) lam2(0.9) stCost(-5)"<<endl; return -1;}
	string paramFile=string(argv[1]);
	string inFile=string(argv[2]);
	string outFile=string(argv[3]);
	double w1=atof(argv[4]);
	double w2=atof(argv[5]);
	double w3=atof(argv[6]);
	double lam1=atof(argv[7]);
	double lam2=atof(argv[8]);
	double stCost=atof(argv[9]);

	FingeringHMM_3rd model(w1,w2,w3,lam1,lam2,stCost);
	model.ReadParamFile(paramFile);//"param_FHMM3.txt"

	PianoFingering fingering;
	fingering.ReadFile(inFile);
	model.testData.clear();
	model.testData.push_back(fingering);
	model.ViterbiTwoHands();
	model.testData[0].WriteFile(outFile);

	return 0;
}//end main


