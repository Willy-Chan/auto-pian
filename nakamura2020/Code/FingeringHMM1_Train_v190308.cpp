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

	if(argc!=4){cout<<"Error in usage! : $./this list_train.txt DataFolder paramHeader(param_FHMM1)"<<endl; return -1;}
	string listTrain=string(argv[1]);//"../../Data/list_train.txt"
	string dataFolder=string(argv[2]);//"../../Data/fingering_pub/"
	string paramHeader=string(argv[3]);//"param_FHMM1"

	if(dataFolder[dataFolder.size()-1]!='/'){dataFolder+="/";}

	FingeringHMM model;
	model.RandomInit();
	model.ReadTrainOrTestData("train",listTrain,dataFolder);

	model.SupervisedLearning(false,false);
	model.WriteParamFile(paramHeader+".txt");

// 	model.SupervisedLearning(true,false);
// 	model.WriteParamFile(paramHeader+"_TR.txt");
// 
// 	model.SupervisedLearning(false,true);
// 	model.WriteParamFile(paramHeader+"_RF.txt");
// 
// 	model.SupervisedLearning(true,true);
// 	model.WriteParamFile(paramHeader+"_TRRF.txt");


	return 0;
}//end main


