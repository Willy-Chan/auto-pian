#ifndef CHMM_HPP
#define CHMM_HPP

#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<cmath>
#include<cassert>
#include<algorithm>
#include"BasicCalculation_v170122.hpp"
#include"KeyPos_v161230.hpp"
#include"PianoFingering_v170101_2.hpp"

using namespace std;

class ClusteredFingeringNoteEvt{
public:
	int pitch;
	string sitch;
	double ontime;
	int channel;
	int finger;
	string fingerNum;
	int tied;//0:untied, 1:tied
	int orgPos;//position in the fingering data
	int ext1;//position in the original both-hand fingering data
};//endclass ClusteredFingeringNoteEvt

class LessClusteredFingeringNoteEvt{
public:
	bool operator()(const ClusteredFingeringNoteEvt& a, const ClusteredFingeringNoteEvt& b){
		if(a.pitch < b.pitch){
			return true;
		}else{//if a.pitch >= b.pitch
			return false;
		}//endif
	}//end operator()
};//endclass LessClusteredFingeringNoteEvt
//sort(unifiedMes.begin(), unifiedMes.end(), LessClusteredFingeringNoteEvt());

class ClusteredFingeringEvt{
public:
	double meanOntime;
	vector<ClusteredFingeringNoteEvt> notes;
};//endclass ClusteredFingeringEvt

class ClusteredFingering{
public:
	vector<ClusteredFingeringEvt> evts;

	ClusteredFingering(){}//end ClusteredFingering
	ClusteredFingering(PianoFingering fingering){
		FromPianoFingering(fingering);
	}//end ClusteredFingering
	~ClusteredFingering(){}//end ~ClusteredFingering

	void FromPianoFingering(PianoFingering fingering){
		evts.clear();
		vector<vector<int> > posCluster;
		vector<double> lastOntime;
{
		vector<int> vi;
		vi.push_back(0);
		double lontime=fingering.evts[0].ontime;
		for(int n=1;n<fingering.evts.size();n+=1){
			if(abs(fingering.evts[n].ontime-fingering.evts[n-1].ontime)>=0.03){
				posCluster.push_back(vi);
				lastOntime.push_back(lontime);
				vi.clear();
				lontime=fingering.evts[n].ontime;
			}//endif
			if(fingering.evts[n].ontime>lontime){lontime=fingering.evts[n].ontime;}
			vi.push_back(n);
		}//endfor n
		posCluster.push_back(vi);
		lastOntime.push_back(lontime);
}//

		for(int i=0;i<posCluster.size();i+=1){
			ClusteredFingeringEvt evt;
			ClusteredFingeringNoteEvt note;
			evt.meanOntime=0;
			for(int j=0;j<posCluster[i].size();j+=1){
				evt.meanOntime+=fingering.evts[posCluster[i][j]].ontime;
				note.pitch=fingering.evts[posCluster[i][j]].pitch;
				note.sitch=fingering.evts[posCluster[i][j]].sitch;
				note.ontime=fingering.evts[posCluster[i][j]].ontime;
				note.channel=fingering.evts[posCluster[i][j]].channel;
				note.finger=fingering.evts[posCluster[i][j]].finger;
				note.fingerNum=fingering.evts[posCluster[i][j]].fingerNum;
				note.tied=0;
				note.orgPos=posCluster[i][j];
				note.ext1=fingering.evts[posCluster[i][j]].ext1;
				evt.notes.push_back(note);
			}//endfor j
			evt.meanOntime/=double(posCluster[i].size());

			if(i==0){evts.push_back(evt); continue;}//endif
	
			for(int j=0;j<evts[i-1].notes.size();j+=1){
				if(fingering.evts[evts[i-1].notes[j].orgPos].offtime>lastOntime[i]+0.2){
					note=evts[i-1].notes[j];
					note.tied=1;
					evt.notes.push_back(note);
				}//endif
			}//endfor j
	
			evts.push_back(evt);
		}//endfor i

		//Ordering
		for(int i=0;i<evts.size();i+=1){
			stable_sort(evts[i].notes.begin(), evts[i].notes.end(), LessClusteredFingeringNoteEvt());
		}//endfor i

	}//end FromPianoFingering

	void WriteFile(string filename){
		ofstream ofs(filename.c_str());
		for(int i=0;i<evts.size();i+=1){
ofs<<"Evt\t"<<i<<"\t"<<evts[i].notes.size()<<"\t"<<evts[i].meanOntime<<"\n";
			for(int j=0;j<evts[i].notes.size();j+=1){
ofs<<"\t"<<j+1<<"\t"<<evts[i].notes[j].tied<<"\t"<<evts[i].notes[j].sitch<<"\t"<<evts[i].notes[j].ontime;
ofs<<"\t"<<evts[i].notes[j].fingerNum<<"\t"<<evts[i].notes[j].channel<<"\t"<<evts[i].notes[j].orgPos<<"\t"<<evts[i].notes[j].ext1<<"\n";
			}//endfor j
		}//endfor i
		ofs.close();
	}//endfor WriteFile
};//endclass ClusteredFingering

class CHMM{
public:
//	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
//	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	vector<vector<Prob<int> > > horTrProb;//[0,1]=Right,Left x 5 x 5
	vector<vector<vector<Prob<int> > > > horOutProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	vector<vector<Prob<int> > > verTrProb;//[0,1]=Right,Left x 5 x 5
	vector<vector<vector<Prob<int> > > > verOutProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1

	int widthX;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [dx+widthX]
	int nOut;//=3*(2*widthX+1)

	vector<vector<vector<vector<int> > > > chordFingerTemplates;//2(R,L) x 6(0,1,2,3,4,5) x (combinations) x (0-2-3 etc.)

	vector<PianoFingering> trainData;
	vector<PianoFingering> testData;
	vector<ClusteredFingering> trainDataClR;
	vector<ClusteredFingering> testDataClR;
	vector<ClusteredFingering> trainDataClL;
	vector<ClusteredFingering> testDataClL;
	vector<string> trainDataID;
	vector<string> testDataID;

	double wVerTr,wVerOut,wHorTr,wHorOut;
	double decay;

	CHMM(string chordFinTemplateFile){
		widthX=15;
		nOut=3*(2*widthX+1);
		wVerTr=1;
		wVerOut=1;
		wHorTr=1;
		wHorOut=1;
		decay=0.1;
{
		chordFingerTemplates.clear();
		chordFingerTemplates.resize(2);
		vector<int> vi,v(100);
		vector<string> s(100);
		ifstream ifs(chordFinTemplateFile.c_str());
		for(int h=0;h<2;h+=1){
			chordFingerTemplates[h].resize(7);
			for(int i=0;i<7;i+=1){
				ifs>>v[1]>>v[2];
				getline(ifs,s[99]);
				for(int j=0;j<v[2];j+=1){
					ifs>>v[0];
					vi.clear();
					vi.resize(i);
					for(int k=0;k<i;k+=1){
						ifs>>vi[k];
					}//endfor k
					chordFingerTemplates[h][i].push_back(vi);
				}//endfor j
			}//endfor i
		}//endfor h
		ifs.close();
}//
	}//end CHMM
	~CHMM(){}//end ~CHMM

	void RandomInit(){
		horTrProb.clear();
		verTrProb.clear();
		horOutProb.clear();
		verOutProb.clear();
		horTrProb.resize(2);
		verTrProb.resize(2);
		horOutProb.resize(2);
		verOutProb.resize(2);
		for(int i=0;i<2;i+=1){
			horTrProb[i].resize(5);
			verTrProb[i].resize(5);
			horOutProb[i].resize(5);
			verOutProb[i].resize(5);
			for(int j=0;j<5;j+=1){
				horTrProb[i][j].Resize(5);
				horTrProb[i][j].Randomize();
				verTrProb[i][j].Resize(5);
				verTrProb[i][j].Randomize();
				horOutProb[i][j].resize(5);
				verOutProb[i][j].resize(5);
				for(int k=0;k<5;k+=1){
					horOutProb[i][j][k].Resize(nOut);
					horOutProb[i][j][k].Randomize();
					verOutProb[i][j][k].Resize(nOut);
					verOutProb[i][j][k].Randomize();
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void SmoothInit(double eps_tr=1E-3,double eps_out=1E-3){
		RandomInit();
		for(int i=0;i<2;i+=1){
			for(int j=0;j<5;j+=1){
				horTrProb[i][j].P.assign(5,eps_tr);
				verTrProb[i][j].P.assign(5,eps_tr);
				for(int k=0;k<5;k+=1){
					horOutProb[i][j][k].P.assign(nOut,eps_out);
					verOutProb[i][j][k].P.assign(nOut,eps_out);
				}//endfor k
			}//endfor j
		}//endfor i
	}//end SmoothInit

	void WriteParamFile(string filename){
		ofstream ofs(filename.c_str());

		ofs<<"### Horizontal Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<horTrProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip
		ofs<<"### Horizontal Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<horTrProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Vertical Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<verTrProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip
		ofs<<"### Vertical Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<verTrProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Horizontal Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<horOutProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip
		ofs<<"### Horizontal Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<horOutProb[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Vertical Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<verOutProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip
		ofs<<"### Vertical Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<verOutProb[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs.close();
	}//end WriteParamFile

	void ReadParamFile(string filename){
		vector<int> v(100);
		vector<double> d(100);
		vector<string> s(100);
		stringstream ss;

		ifstream ifs(filename.c_str());

		RandomInit();

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>horTrProb[h][ip].P[i];
				}//endfor i
				horTrProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>verTrProb[h][ip].P[i];
				}//endfor i
				verTrProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>horOutProb[h][ip][i].P[x];
					}//endfor x
					horOutProb[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>verOutProb[h][ip][i].P[x];
					}//endfor x
					verOutProb[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		ifs.close();

	}//end ReadParamFile

	void ReadTrainOrTestData(string trainORtest,string fileList,string folder){
		assert(trainORtest=="train" || trainORtest=="test");
		if(folder[folder.size()-1]!='/'){folder+="/";}
		vector<int> v(100);
		vector<double> d(100);
		vector<string> s(100);
		stringstream ss;

		if(trainORtest=="train"){trainData.clear();trainDataID.clear();trainDataClR.clear();trainDataClL.clear();
		}else{testData.clear();testDataID.clear();testDataClR.clear();testDataClL.clear();
		}//endif

		vector<string> fileID;
{
		ifstream ifs(fileList.c_str());
		while(ifs>>s[1]){
			fileID.push_back(s[1]);
			getline(ifs,s[99]);
		}//endwhile
		ifs.close();
}
		for(int i=0;i<fileID.size();i+=1){
			ss.str(""); ss<<folder<<fileID[i]<<"_fingering.txt";
			PianoFingering fingering;
			fingering.ReadFile(ss.str());
			fingering.ConvertFingerNumberToInt();
			if(trainORtest=="train"){trainData.push_back(fingering);trainDataID.push_back(fileID[i]);
			}else{testData.push_back(fingering);testDataID.push_back(fileID[i]);
			}//endif
		}//endfor i

		if(trainORtest=="train"){
			for(int i=0;i<trainData.size();i+=1){
				PianoFingering fingering;
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(0);
				ClusteredFingering cfingering(fingering);
				trainDataClR.push_back(cfingering);
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(1);
				cfingering.FromPianoFingering(fingering);
				trainDataClL.push_back(cfingering);
			}//endfor i
		}else{
			for(int i=0;i<testData.size();i+=1){
				PianoFingering fingering;
				fingering=testData[i];
				fingering.SelectHandByFingerNum(0);
				ClusteredFingering cfingering(fingering);
				testDataClR.push_back(cfingering);
				fingering=testData[i];
				fingering.SelectHandByFingerNum(1);
				cfingering.FromPianoFingering(fingering);
				testDataClL.push_back(cfingering);
			}//endfor i
		}//endif

	}//end ReadTrainOrTestData

	void SupervisedLearning(){
		SmoothInit();
		for(int h=0;h<2;h+=1){
			for(int i=0;i<trainData.size();i+=1){
//cout<<trainData[i].evts.size()<<"\t";
				ClusteredFingering cf;
				if(h==0){
					cf=trainDataClR[i];
				}else{
					cf=trainDataClL[i];
				}//endif
//cout<<fingering.evts.size()<<"\t";
				for(int n=0;n<cf.evts.size();n+=1){
					for(int kp=0;kp<cf.evts[n].notes.size();kp+=1){
						for(int k=kp+1;k<cf.evts[n].notes.size();k+=1){
							verTrProb[h][cf.evts[n].notes[kp].finger*(1-2*h)-1].P[cf.evts[n].notes[k].finger*(1-2*h)-1]+=1;

							KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(cf.evts[n].notes[k].pitch),PitchToKeyPos(cf.evts[n].notes[kp].pitch));
							if(keyInt.x<-widthX){keyInt.x=-widthX;}
							if(keyInt.x>widthX){keyInt.x=widthX;}
							verOutProb[h][cf.evts[n].notes[kp].finger*(1-2*h)-1][cf.evts[n].notes[k].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
						}//endfor k
					}//endfor kp

					if(n==0){continue;}//endif

					for(int kp=0;kp<cf.evts[n-1].notes.size();kp+=1){
						if(cf.evts[n-1].notes[kp].tied>0){continue;}
						for(int k=0;k<cf.evts[n].notes.size();k+=1){
							if(cf.evts[n].notes[k].tied>0){continue;}
							horTrProb[h][cf.evts[n-1].notes[kp].finger*(1-2*h)-1].P[cf.evts[n].notes[k].finger*(1-2*h)-1]+=1;
							KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(cf.evts[n].notes[k].pitch),PitchToKeyPos(cf.evts[n-1].notes[kp].pitch));
							if(keyInt.x<-widthX){keyInt.x=-widthX;}
							if(keyInt.x>widthX){keyInt.x=widthX;}
							horOutProb[h][cf.evts[n-1].notes[kp].finger*(1-2*h)-1][cf.evts[n].notes[k].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
						}//endfor k
					}//endfor kp
				}//endfor n
//cout<<endl;
			}//endfor i
		}//endfor h

		for(int h=0;h<2;h+=1){
			for(int ip=0;ip<5;ip+=1){
				horTrProb[h][ip].Normalize();
				verTrProb[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					horOutProb[h][ip][i].Normalize();
					verOutProb[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h

	}//end SupervisedLearning


	void Viterbi(int hand){//0:RH 1:LH
		vector<ClusteredFingering> testDataCl;
		if(hand==0){
			testDataCl=testDataClR;
		}else{
			testDataCl=testDataClL;
		}//endif
//cout<<wVerTr<<endl;
		for(int i=0;i<testDataCl.size();i+=1){

			int len=testDataCl[i].evts.size();

			vector<vector<double> > LP;
			vector<vector<double> > amax;
			LP.resize(len);
			amax.resize(len);
			for(int n=0;n<len;n+=1){
				int nNote=testDataCl[i].evts[n].notes.size();
				assert(nNote<=6);
				LP[n].resize(chordFingerTemplates[hand][nNote].size());
				amax[n].resize(chordFingerTemplates[hand][nNote].size());

				if(n==0){
					for(int g=0;g<LP[n].size();g+=1){
						LP[n][g]=0;
						for(int k=0;k<nNote;k+=1)for(int kp=k+1;kp<nNote;kp+=1){
							KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testDataCl[i].evts[n].notes[kp].pitch),PitchToKeyPos(testDataCl[i].evts[n].notes[k].pitch));
							if(keyInt.x<-widthX){keyInt.x=-widthX;}
							if(keyInt.x>widthX){keyInt.x=widthX;}
							LP[n][g]+=wVerTr*verTrProb[hand][chordFingerTemplates[hand][nNote][g][k]].LP[chordFingerTemplates[hand][nNote][g][kp]];
							LP[n][g]+=wVerOut*verOutProb[hand][chordFingerTemplates[hand][nNote][g][k]][chordFingerTemplates[hand][nNote][g][kp]].LP[3*(keyInt.x+widthX)+keyInt.y+1];
						}//endfor k,kp
					}//endfor g
					continue;
				}//endif

				double logP;
				double fac=1./pow(double(nNote),decay);
				for(int g=0;g<LP[n].size();g+=1){

					for(int gp=0;gp<LP[n-1].size();gp+=1){
						logP=LP[n-1][gp];
						for(int k=0;k<nNote;k+=1)for(int kp=0;kp<testDataCl[i].evts[n-1].notes.size();kp+=1){
							KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testDataCl[i].evts[n].notes[k].pitch),PitchToKeyPos(testDataCl[i].evts[n-1].notes[kp].pitch));
							if(keyInt.x<-widthX){keyInt.x=-widthX;}
							if(keyInt.x>widthX){keyInt.x=widthX;}
							logP+=(fac)*wHorTr*horTrProb[hand][chordFingerTemplates[hand][testDataCl[i].evts[n-1].notes.size()][gp][kp]].LP[chordFingerTemplates[hand][nNote][g][k]];
							logP+=(fac)*wHorOut*horOutProb[hand][chordFingerTemplates[hand][testDataCl[i].evts[n-1].notes.size()][gp][kp]][chordFingerTemplates[hand][nNote][g][k]].LP[3*(keyInt.x+widthX)+keyInt.y+1];
						}//endfor k,kp
						if(gp==0 || logP>LP[n][g]){
							LP[n][g]=logP;
							amax[n][g]=gp;
						}//endif
					}//endfor gp

					for(int k=0;k<nNote;k+=1)for(int kp=k+1;kp<nNote;kp+=1){
						KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testDataCl[i].evts[n].notes[kp].pitch),PitchToKeyPos(testDataCl[i].evts[n].notes[k].pitch));
						if(keyInt.x<-widthX){keyInt.x=-widthX;}
						if(keyInt.x>widthX){keyInt.x=widthX;}
						LP[n][g]+=(fac)*wVerTr*verTrProb[hand][chordFingerTemplates[hand][nNote][g][k]].LP[chordFingerTemplates[hand][nNote][g][kp]];
						LP[n][g]+=(fac)*wVerOut*verOutProb[hand][chordFingerTemplates[hand][nNote][g][k]][chordFingerTemplates[hand][nNote][g][kp]].LP[3*(keyInt.x+widthX)+keyInt.y+1];
					}//endfor k,kp
				}//endfor g

			}//endfor n

			vector<int> optPath(len);
			optPath[len-1]=0;
			for(int g=0;g<LP[len-1].size();g+=1){
				if(LP[len-1][g]>LP[len-1][optPath[len-1]]){optPath[len-1]=g;}
			}//endfor g
			for(int n=len-2;n>=0;n-=1){
				optPath[n]=amax[n+1][optPath[n+1]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<len;n+=1){
				for(int k=0;k<testDataCl[i].evts[n].notes.size();k+=1){
					ss.str(""); ss<<((hand==0)? "":"-")<<(chordFingerTemplates[hand][testDataCl[i].evts[n].notes.size()][optPath[n]][k]+1);
					testDataCl[i].evts[n].notes[k].finger=(1-2*hand)*(chordFingerTemplates[hand][testDataCl[i].evts[n].notes.size()][optPath[n]][k]+1);
					testDataCl[i].evts[n].notes[k].fingerNum=ss.str();
					testData[i].evts[testDataCl[i].evts[n].notes[k].ext1].fingerNum=testDataCl[i].evts[n].notes[k].fingerNum;
					testData[i].evts[testDataCl[i].evts[n].notes[k].ext1].finger=testDataCl[i].evts[n].notes[k].finger;
				}//endfor k
			}//endfor n

		}//endfor i
	}//end Viterbi

	void ViterbiTwoHands(){
		Viterbi(0);
		Viterbi(1);
	}//end ViterbiTwoHands

	void WriteTestData(string folder){
		stringstream ss;
		for(int i=0;i<testDataID.size();i+=1){
			ss.str(""); ss<<folder<<testDataID[i]<<"_fingering.txt";
			testData[i].WriteFile(ss.str());
		}//endfor i
	}//end WriteTestData

 };//endclass CHMM




#endif // CHMM_HPP
