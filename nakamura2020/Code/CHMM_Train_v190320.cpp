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

	if(argc!=5){cout<<"Error in usage! : $./this ChordFinergingTemplates.txt list_train.txt DataFolder paramHeader(param_CHMM1)"<<endl; return -1;}
	string templateFile=string(argv[1]);//"ChordFinergingTemplates.txt"
	string listTrain=string(argv[2]);//"../../Data/list_train.txt"
	string dataFolder=string(argv[3]);//"../../Data/fingering_pub/"
	string paramHeader=string(argv[4]);//"param_CHMM1"

	CHMM model(templateFile);
	model.RandomInit();
	model.ReadTrainOrTestData("train",listTrain,dataFolder);
	model.SupervisedLearning();
	model.WriteParamFile(paramHeader+".txt");

//	model.SupervisedLearningTR();
//	model.WriteParamFile("param_CHMM1TR.txt");

	return 0;
}//end main


