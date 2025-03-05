#include<iostream>
#include<string>
#include<sstream>
#include<cmath>
#include<vector>
#include<algorithm>
#include<fstream>
#include<cassert>
#include"CHMM_v181128.hpp"
#include"ProbabilityVisualizer_v180920.hpp"

using namespace std;

int main(int argc, char** argv) {

	vector<int> v(100);
	vector<double> d(100);
	vector<string> s(100);
	stringstream ss;

	if(argc!=10){cout<<"Error in usage! : $./this ChordFinergingTemplates.txt param.txt in_fingering.txt out_fingering.txt beta1 beta2 gamma1 gamma2 zeta"<<endl; return -1;}
	string templateFile=string(argv[1]);
	string paramFile=string(argv[2]);
	string inFile=string(argv[3]);
	string outFile=string(argv[4]);
	double wVT=atof(argv[5]);
	double wVO=atof(argv[6]);
	double wHT=atof(argv[7]);
	double wHO=atof(argv[8]);
	double decay=atof(argv[9]);

	CHMM model(templateFile);
	model.ReadParamFile(paramFile);
	model.wVerTr=wVT;
	model.wVerOut=wVO;
	model.wHorTr=wHT;
	model.wHorOut=wHO;
	model.decay=decay;

	PianoFingering fingering,fingering_tmp;
	fingering.ReadFile(inFile);
	model.testData.clear();
	model.testDataClR.clear();
	model.testDataClL.clear();
	model.testData.push_back(fingering);
	fingering_tmp=fingering;
	fingering_tmp.SelectHandByChannel(0);
	ClusteredFingering cfingering(fingering_tmp);
	model.testDataClR.push_back(cfingering);
	fingering_tmp=fingering;
	fingering_tmp.SelectHandByChannel(1);
	cfingering.FromPianoFingering(fingering_tmp);
	model.testDataClL.push_back(cfingering);

	model.ViterbiTwoHands();
	model.testData[0].WriteFile(outFile);


	return 0;
}//end main


