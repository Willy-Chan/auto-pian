#ifndef ProbabilityVisualizer_HPP
#define ProbabilityVisualizer_HPP

#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<cfloat>
#include<cmath>
#include<cassert>
#include<algorithm>
#include"BasicCalculation_v170122.hpp"

#define EPS (0.1)

using namespace std;

	void VisualizeProb(Prob<int> prob,string filename){

		double DeltaX=1000;//170
		double DeltaY=500;//200
		double DeltaXDistr=DeltaX/1.2;
		double DeltaYDistr=DeltaY/1.5;
		double BoxSizeX=DeltaXDistr*1.1;
		double BoxSizeY=DeltaYDistr*1.;

		double x=0.5*BoxSizeX;
		double y=0.5*BoxSizeY;

		double width=2*DeltaX;
		double height=2*DeltaY;

		double scale=DeltaYDistr*0.85;
		double delta=DeltaXDistr/double(prob.P.size());

		double offsetDistrX=-0.5*DeltaXDistr;
		double offsetDistrY=0.35*DeltaYDistr;

		double fontSize1=15.0;
		double fontSize2=20.0;
//		double fontSize3=1.*delta;
		double fontSize3=10;
		double lineWidth=2.;

		ofstream ofs(filename.c_str());
ofs<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"<<"\n";
ofs<<"<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" width=\""<<width<<"\" height=\""<<height<<"\">"<<"\n";
ofs<<"\n";

ofs<<"<rect x=\""<<x-0.5*BoxSizeX<<"\" y=\""<<y-0.5*BoxSizeY<<"\" width=\""<<BoxSizeX<<"\" height=\""<<BoxSizeY<<"\" opacity=\""<<1.<<"\" fill=\""<<"rgb(255,255,255)"<<"\" stroke=\"rgb(30,120,255)\" stroke-width=\""<<lineWidth<<"\"/>"<<"\n";

		///Draw distributions
		for(int j=0;j<prob.P.size();j+=1){
ofs<<"<rect x=\""<<x+j*delta+offsetDistrX<<"\" y=\""<<y-prob.P[j]*scale+offsetDistrY<<"\" width=\""<<0.8*delta<<"\" height=\""<<prob.P[j]*scale<<"\" opacity=\""<<1.<<"\" fill=\""<<"rgb(255,30,120)"<<"\"/>"<<"\n";
ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<x+(j+0.5)*delta+offsetDistrX<<","<<y+offsetDistrY+fontSize3<<")\" font-size=\""<<fontSize3<<"\" fill=\"black\">"<<j<<"</text>"<<"\n";
		}//

ofs<<"\n";
ofs<<"</svg>"<<"\n";
		ofs.close();

	}//end VisualizeProb

	void VisualizeTrProb(vector<Prob<int> > trProb,string filename){

		double width,height;
		double originX,originY;
		double deltaX,deltaY;
		double axisLabelFontSize,symbFontSize;
		double linewidth;
		double maxLen=trProb.size();
		double maxLenX=trProb[0].P.size();
		double nSymb=trProb.size();

		originX=50;
		originY=50;
		deltaX=10;
		deltaY=10;
		linewidth=0.5;
		axisLabelFontSize=8;
		symbFontSize=5;

		width=(maxLenX+1+nSymb+10)*deltaX;
		height=(maxLen+10)*deltaX;

		ofstream ofs(filename.c_str());
ofs<<"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"<<"\n";
ofs<<"<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" width=\""<<width<<"\" height=\""<<height<<"\">"<<"\n";
ofs<<"\n";

	///Draw tr prob box
ofs<<"<rect x=\""<<originX<<"\" y=\""<<originY<<"\" width=\""<<maxLenX*deltaX<<"\" height=\""<<maxLen*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(0,0,0)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
	for(int i=1;i<maxLen;i+=1){
ofs<<"<line x1=\""<<originX<<"\" x2=\""<<originX+maxLenX*deltaX<<"\" y1=\""<<originY+i*deltaY<<"\" y2=\""<<originY+i*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(120,120,120)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
	}//endfor i
	for(int i=1;i<maxLenX;i+=1){
ofs<<"<line x1=\""<<originX+i*deltaX<<"\" x2=\""<<originX+i*deltaX<<"\" y1=\""<<originY<<"\" y2=\""<<originY+maxLen*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(120,120,120)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
	}//endfor i
	///Draw tr prob labels
	for(int i=1;i<maxLen;i+=1){
ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<originX-1.*axisLabelFontSize<<","<<originY+(maxLen-i)*deltaY-0.5*(deltaY-0.6*axisLabelFontSize)<<")\" font-size=\""<<axisLabelFontSize<<"\" fill=\"black\">"<<i<<"</text>"<<"\n";
	}//endfor i
ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<originX-1.*axisLabelFontSize<<","<<originY+(maxLen)*deltaY-0.5*(deltaY-0.6*axisLabelFontSize)<<")\" font-size=\""<<axisLabelFontSize<<"\" fill=\"black\">"<<"0"<<"</text>"<<"\n";
	for(int i=1;i<=maxLenX;i+=1){
ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<originX+(i-0.5)*deltaX<<","<<originY+(maxLen)*deltaY+1.*axisLabelFontSize<<")\" font-size=\""<<axisLabelFontSize<<"\" fill=\"black\">"<<(i-1)<<"</text>"<<"\n";
	}//endfor i
//ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<originX+(maxLen-0.5)*deltaX<<","<<originY+(maxLen)*deltaY+1.*axisLabelFontSize<<")\" font-size=\""<<axisLabelFontSize<<"\" fill=\"black\">"<<"End"<<"</text>"<<"\n";

	///Draw out prob box
//ofs<<"<rect x=\""<<originX+(maxLen+1)*deltaX<<"\" y=\""<<originY<<"\" width=\""<<nSymb*deltaX<<"\" height=\""<<(maxLen)*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(0,0,0)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
//	for(int i=1;i<maxLen;i+=1){
//ofs<<"<line x1=\""<<originX+(maxLen+1)*deltaX<<"\" x2=\""<<originX+(maxLen+1+nSymb)*deltaX<<"\" y1=\""<<originY+i*deltaY<<"\" y2=\""<<originY+i*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(120,120,120)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
//	}//endfor i
//	for(int i=1;i<nSymb;i+=1){
//ofs<<"<line x1=\""<<originX+(maxLen+1+i)*deltaX<<"\" x2=\""<<originX+(maxLen+1+i)*deltaX<<"\" y1=\""<<originY<<"\" y2=\""<<originY+(maxLen)*deltaY<<"\" fill=\"transparent\" stroke=\"rgb(120,120,120)\" stroke-width=\""<<linewidth<<"\"/>"<<"\n";
//	}//endfor i
	///Draw out prob labels
//	for(int i=0;i<nSymb;i+=1){
//ofs<<"<text text-anchor=\"middle\" transform=\"translate("<<originX+(maxLen+i+2-0.5)*deltaX<<","<<originY+(maxLen)*deltaY+1.5*symbFontSize+((i%2==0)? 0:1.5*symbFontSize)<<")\" font-size=\""<<symbFontSize<<"\" fill=\"black\">"<<symbols[i]<<"</text>"<<"\n";
//	}//endfor i

	///Draw tr prob
	for(int i=0;i<trProb.size();i+=1){
		for(int j=0;j<trProb[i].P.size();j+=1){
ofs<<"<rect x=\""<<originX+(j)*deltaX<<"\" y=\""<<originY+(maxLen-1-i)*deltaY<<"\" width=\""<<deltaX<<"\" height=\""<<deltaY<<"\" opacity=\""<<trProb[i].P[j]<<"\" fill=\""<<"rgb(255,30,120)"<<"\"/>"<<"\n";
		}//endfor j
	}//endfor i

	///Draw out prob
//	for(int i=0;i<maxLen;i+=1){
//		for(int x=0;x<nSymb;x+=1){
//ofs<<"<rect x=\""<<originX+(maxLen+1+x)*deltaX<<"\" y=\""<<originY+(maxLen-1-i)*deltaY<<"\" width=\""<<deltaX<<"\" height=\""<<deltaY<<"\" opacity=\""<<outProb[i].P[x]<<"\" fill=\""<<"rgb(30,120,255)"<<"\"/>"<<"\n";
//		}//endfor x
//	}//endfor i

ofs<<"\n";
ofs<<"</svg>"<<"\n";
		ofs.close();

	}//end VisualizeTrProb

void VisualizeStationaryProb(vector<Prob<int> > trProb,string filename){
	Prob<int> statProb;
	statProb.Resize(trProb.size());
	for(int i=0;i<trProb.size();i+=1){statProb.P[i]=1;}
	statProb.Normalize();

	Prob<int> _statProb;
	for(int iter=0;iter<300;iter+=1){
		_statProb=statProb;
		for(int i=0;i<trProb.size();i+=1){
			statProb.P[i]=0;
			for(int ip=0;ip<trProb.size();ip+=1){
				statProb.P[i]+=_statProb.P[ip]*trProb[ip].P[i];
			}//endfor ip
		}//endfor i
		statProb.Normalize();
	}//endfor iter
	VisualizeProb(statProb,filename);

}//end VisualizeStationaryProb


#endif // ProbabilityVisualizer_HPP
