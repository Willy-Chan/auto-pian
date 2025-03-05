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

	if(argc!=3){cout<<"Error in usage: $./this true_fingering.txt est_fingering.txt"<<endl; return -1;}
	string trueFile=string(argv[1]);
	string estFile=string(argv[2]);

	vector<PianoFingering> data;

	PianoFingering fin;
	fin.ReadFile(trueFile);
	data.push_back(fin);

	fin.ReadFile(estFile);
	data.push_back(fin);

	vector<int> matchPos=GetMatchPos(data);

	double nNotes=fin.evts.size();
	double nMatchedNotes=matchPos.size();

cout<<"MatchRate: "<<nMatchedNotes<<"/"<<nNotes<<"\t"<<nMatchedNotes/nNotes<<"\t"<<nMatchedNotes/nNotes*100<<" %"<<endl;


	return 0;
}//end main


