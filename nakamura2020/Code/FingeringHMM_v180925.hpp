#ifndef FingeringHMM_HPP
#define FingeringHMM_HPP

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

class FingeringHMM{
public:
	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	int widthX;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [dx+widthX]
	int nOut;//=3*(2*widthX+1)

	double w1;//weights for the output probabilities
	double shortTimeCost;

	vector<PianoFingering> trainData;
	vector<PianoFingering> testData;
	vector<string> trainDataID;
	vector<string> testDataID;

	vector<PianoFingering> trainDataTD;//time-dependent pitch ordering
	vector<PianoFingering> testDataTD;//time-dependent pitch ordering

//	vector<vector<vector<vector<double> > > > ForwardVar;
//	vector<vector<vector<double> > > BackwardVar;

	FingeringHMM(double w1_=1,double shortTimeCost_=-5){
		widthX=15;
		nOut=3*(2*widthX+1);
		w1=w1_;
		shortTimeCost=shortTimeCost_;
	}//end FingeringHMM
	~FingeringHMM(){}//end ~FingeringHMM

	void RandomInit(){
		iniProb.clear();
		trProb.clear();
		outProb.clear();
		iniProb.resize(2);
		trProb.resize(2);
		outProb.resize(2);
		for(int i=0;i<2;i+=1){
			iniProb[i].Resize(5);
			iniProb[i].Randomize();
			trProb[i].resize(5);
			outProb[i].resize(5);
			for(int j=0;j<5;j+=1){
				trProb[i][j].Resize(5);
				trProb[i][j].Randomize();
				outProb[i][j].resize(5);
				for(int k=0;k<5;k+=1){
					outProb[i][j][k].Resize(nOut);
					outProb[i][j][k].Randomize();
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void SmoothInit(double eps_ini=1E-3,double eps_tr=1E-3,double eps_out=1E-3){
		RandomInit();
		for(int i=0;i<2;i+=1){
			iniProb[i].P.assign(5,eps_ini);
			for(int j=0;j<5;j+=1){
				trProb[i][j].P.assign(5,eps_tr);
				for(int k=0;k<5;k+=1){
					outProb[i][j][k].P.assign(nOut,eps_out);
				}//endfor k
			}//endfor j
		}//endfor i
	}//end SmoothInit

	void SetUnifTrProb(){
		for(int h=0;h<2;h+=1){
			for(int i=0;i<5;i+=1){
				trProb[h][i].P.assign(5,1);
				trProb[h][i].Normalize();
			}//endfor i
		}//endfor h
	}//end SetUnifTrProb

	void WriteParamFile(string filename){
		ofstream ofs(filename.c_str());

		ofs<<"### Initial Prob Right\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[0].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		ofs<<"### Initial Prob Left\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[1].P[i]<<"\t";
			}//endfor i
ofs<<"\n";

		ofs<<"### Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip
		ofs<<"### Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip
		ofs<<"### Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[1][ip][i].P[x]<<"\t";
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
			getline(ifs,s[99]);//### Initial Prob Right
				for(int i=0;i<5;i+=1){
				ifs>>iniProb[h].P[i];
			}//endfor i
			getline(ifs,s[99]);
			iniProb[h].Normalize();
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>trProb[h][ip].P[i];
				}//endfor i
				trProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb[h][ip][i].P[x];
					}//endfor x
					outProb[h][ip][i].Normalize();
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

		if(trainORtest=="train"){trainData.clear();trainDataID.clear();
		}else{testData.clear();testDataID.clear();
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
			if(trainORtest=="train"){trainData.push_back(fingering);trainDataID.push_back(fileID[i]);
			}else{testData.push_back(fingering);testDataID.push_back(fileID[i]);
			}//endif
		}//endfor i
	}//end ReadTrainOrTestData

	void SupervisedLearning(bool TRSym=false,bool RFSym=false){
		SmoothInit();
		for(int i=0;i<trainData.size();i+=1){
//cout<<trainData[i].evts.size()<<"\t";
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
//cout<<fingering.evts.size()<<"\t";
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
					if(TRSym){
						trProb[h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb[1-h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-1].pitch));
					if(keyInt.x<-widthX){keyInt.x=-widthX;}
					if(keyInt.x>widthX){keyInt.x=widthX;}
					outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
					if(TRSym){
						outProb[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)-keyInt.y+1]+=1;
					}//endif
					if(RFSym){
						outProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)+keyInt.y+1]+=1;
						if(TRSym){
							outProb[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)-keyInt.y+1]+=1;
						}//endif
					}//endif
				}//endfor n
			}//endfor h
//cout<<endl;
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					outProb[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearning

	void Viterbi(int hand){//0:RH 1:LH

		for(int i=0;i<testData.size();i+=1){
// 			testData[i].SetChannelFromFingerNumber();
// 			testData[i].ConvertFingerNumberToInt();
			vector<int> pos;
			for(int n=0;n<testData[i].evts.size();n+=1){
// 				if(hand==0 && testData[i].evts[n].finger>0){pos.push_back(n);}//endif
// 				if(hand==1 && testData[i].evts[n].finger<0){pos.push_back(n);}//endif
				if(hand==0 && testData[i].evts[n].channel==0){pos.push_back(n);}//endif
				if(hand==1 && testData[i].evts[n].channel==1){pos.push_back(n);}//endif
			}//endfor n

			if(pos.size()<2){continue;}

			vector<double> LP(5);
			vector<vector<double> > amax;
			amax.resize(pos.size());

			amax[0].resize(5);
			for(int k=0;k<5;k+=1){
				LP[k]=iniProb[hand].LP[k];
			}//endfor k

			for(int n=1;n<pos.size();n+=1){
				amax[n].resize(5);
				vector<double> preLP(LP);
				double logP;
				KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-1]].pitch));
				if(keyInt.x<-widthX){keyInt.x=-widthX;}
				if(keyInt.x>widthX){keyInt.x=widthX;}

				bool shortTime=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-1]].ontime)<0.03;
				int delPitch=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-1]].pitch;

				for(int k=0;k<5;k+=1){
					LP[k]=preLP[0]+trProb[hand][0].LP[k]+w1*outProb[hand][0][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+((shortTime&&((hand==0&&(k)*delPitch<0)||(hand==1&&(k)*delPitch>0)))? shortTimeCost:0);
					amax[n][k]=0;
					for(int kp=1;kp<5;kp+=1){
						logP=preLP[kp]+trProb[hand][kp].LP[k]+w1*outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0);
						if(logP>LP[k]){
							LP[k]=logP;
							amax[n][k]=kp;
						}//endif
					}//endfor kp
				}//endfor k
			}//endfor n

			vector<int> optPath(pos.size());
			optPath[pos.size()-1]=0;
			for(int k=1;k<5;k+=1){
				if(LP[k]>LP[optPath[pos.size()-1]]){optPath[pos.size()-1]=k;}
			}//endfor k
			for(int n=pos.size()-2;n>=0;n-=1){
				optPath[n]=amax[n+1][optPath[n+1]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<pos.size();n+=1){
				ss.str(""); ss<<((hand==0)? "":"-")<<(optPath[n]+1);
				testData[i].evts[pos[n]].fingerNum=ss.str();
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

	void Forward(){
	}//end Forward

	void Backward(){
	}//end Backward

	void ViterbiTraining(int nIter=3){
		for(int iter=0;iter<nIter;iter+=1){
cout<<"Viterbi training iter: "<<iter+1<<endl;
			testData=trainData;
			ViterbiTwoHands();
			trainData=testData;
			SupervisedLearning();
		}//endfor iter
	}//end ViterbiTraining

};//endclass FingeringHMM



class FingeringHMM_2nd : public FingeringHMM{
public:
//	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
//	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	vector<vector<vector<Prob<int> > > > trProb2;//[0,1]=Right,Left x 5 x 5 x 5
	vector<vector<vector<Prob<int> > > > outProb2;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
//	int widthX;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [dx+widthX]
//	int nOut;//=3*(2*widthX+1)

	double w1,w2;//weights for the output probabilities 
	double lam1;//linear interpolation coefficients for transition probabilities

//	vector<PianoFingering> trainData;
//	vector<PianoFingering> testData;
//	vector<string> trainDataID;
//	vector<string> testDataID;

	FingeringHMM_2nd(double w1_=0.5,double w2_=0.5,double lam1_=0,double shortTimeCost_=-5){
		widthX=15;
		nOut=3*(2*widthX+1);
		w1=w1_;
		w2=w2_;
		lam1=lam1_;
		shortTimeCost=shortTimeCost_;
	}//end FingeringHMM_2nd
	~FingeringHMM_2nd(){}//end ~FingeringHMM_2nd

	void RandomInit(){
		iniProb.clear();
		trProb.clear();
		outProb.clear();
		trProb2.clear();
		outProb2.clear();
		iniProb.resize(2);
		trProb.resize(2);
		outProb.resize(2);
		trProb2.resize(2);
		outProb2.resize(2);
		for(int i=0;i<2;i+=1){
			iniProb[i].Resize(5);
			iniProb[i].Randomize();
			trProb[i].resize(5);
			outProb[i].resize(5);
			trProb2[i].resize(5);
			outProb2[i].resize(5);
			for(int j=0;j<5;j+=1){
				trProb[i][j].Resize(5);
				trProb[i][j].Randomize();
				outProb[i][j].resize(5);
				trProb2[i][j].resize(5);
				outProb2[i][j].resize(5);
				for(int k=0;k<5;k+=1){
					trProb2[i][j][k].Resize(5);
					trProb2[i][j][k].Randomize();
					outProb[i][j][k].Resize(nOut);
					outProb[i][j][k].Randomize();
					outProb2[i][j][k].Resize(nOut);
					outProb2[i][j][k].Randomize();
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void SmoothInit(double eps_ini=1E-3,double eps_tr=1E-3,double eps_out=1E-3){
		RandomInit();
		for(int i=0;i<2;i+=1){
			iniProb[i].P.assign(5,eps_ini);
			for(int j=0;j<5;j+=1){
				trProb[i][j].P.assign(5,eps_tr);
				for(int k=0;k<5;k+=1){
					trProb2[i][j][k].P.assign(5,eps_tr);
					outProb[i][j][k].P.assign(nOut,eps_out);
					outProb2[i][j][k].P.assign(nOut,eps_out);
				}//endfor k
			}//endfor j
		}//endfor i
	}//end SmoothInit

	void SetUnifTrProb(){
		for(int h=0;h<2;h+=1){
			for(int i=0;i<5;i+=1){
				trProb[h][i].P.assign(5,1);
				trProb[h][i].Normalize();
				for(int ip=0;ip<5;ip+=1){
					trProb2[h][i][ip].P.assign(5,1);
					trProb2[h][i][ip].Normalize();
				}//endfor ip
			}//endfor i
		}//endfor h
	}//end SetUnifTrProb

	void WriteParamFile(string filename){
		ofstream ofs(filename.c_str());

		ofs<<"### Initial Prob Right\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[0].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		ofs<<"### Initial Prob Left\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[1].P[i]<<"\t";
			}//endfor i
ofs<<"\n";

		ofs<<"### Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Transition Prob 2nd Right\n";
		for(int ipp=0;ipp<5;ipp+=1){
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
ofs<<trProb2[0][ipp][ip].P[i]<<"\t";
				}//endfor i
ofs<<"\n";
			}//endfor ip
		}//endfor ipp

		ofs<<"### Transition Prob 2nd Left\n";
		for(int ipp=0;ipp<5;ipp+=1){
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
ofs<<trProb2[1][ipp][ip].P[i]<<"\t";
				}//endfor i
ofs<<"\n";
			}//endfor ip
		}//endfor ipp

		ofs<<"### Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 2nd Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb2[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 2nd Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb2[1][ip][i].P[x]<<"\t";
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
			getline(ifs,s[99]);//### Initial Prob Right
				for(int i=0;i<5;i+=1){
				ifs>>iniProb[h].P[i];
			}//endfor i
			getline(ifs,s[99]);
			iniProb[h].Normalize();
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>trProb[h][ip].P[i];
				}//endfor i
				trProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob 2nd Right
			for(int ipp=0;ipp<5;ipp+=1){
				for(int ip=0;ip<5;ip+=1){
					for(int i=0;i<5;i+=1){
						ifs>>trProb2[h][ipp][ip].P[i];
					}//endfor i
					trProb2[h][ipp][ip].Normalize();
				}//endfor ip
			}//endfor ipp
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb[h][ip][i].P[x];
					}//endfor x
					outProb[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob 2nd Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb2[h][ip][i].P[x];
					}//endfor x
					outProb2[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		ifs.close();

		vector<vector<vector<Prob<int> > > > trProb2_tmp;
		trProb2_tmp=trProb2;
		for(int h=0;h<2;h+=1){
			for(int ipp=0;ipp<5;ipp+=1){
				for(int ip=0;ip<5;ip+=1){
					for(int i=0;i<5;i+=1){
						trProb2[h][ipp][ip].P[i]=(1-lam1)*trProb2_tmp[h][ipp][ip].P[i]+lam1*trProb[h][ip].P[i];
					}//endfor i
					trProb2[h][ipp][ip].Normalize();
				}//endfor ip
			}//endfor ipp
		}//endfor h

	}//end ReadParamFile

	void SupervisedLearning(bool TRSym=false,bool RFSym=false){
		SmoothInit();
		for(int i=0;i<trainData.size();i+=1){
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;

					if(TRSym){
						trProb[h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb[1-h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-1].pitch));
					if(keyInt.x<-widthX){keyInt.x=-widthX;}
					if(keyInt.x>widthX){keyInt.x=widthX;}
					outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;

					if(TRSym){
						outProb[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)-keyInt.y+1]+=1;
					}//endif
					if(RFSym){
						outProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)+keyInt.y+1]+=1;
						if(TRSym){
							outProb[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)-keyInt.y+1]+=1;
						}//endif
					}//endif

					if(n<2){continue;}

					trProb2[h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;

					if(TRSym){
						trProb2[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n-2].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb2[1-h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb2[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n-2].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt2=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-2].pitch));
					if(keyInt2.x<-widthX){keyInt2.x=-widthX;}
					if(keyInt2.x>widthX){keyInt2.x=widthX;}
					outProb2[h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt2.x+widthX)+keyInt2.y+1]+=1;

					if(TRSym){
						outProb2[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[3*(-keyInt2.x+widthX)-keyInt2.y+1]+=1;
					}//endif
					if(RFSym){
						outProb2[1-h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt2.x+widthX)+keyInt2.y+1]+=1;
						if(TRSym){
							outProb2[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[3*(keyInt2.x+widthX)-keyInt2.y+1]+=1;
						}//endif
					}//endif

				}//endfor n
			}//endfor h
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					trProb2[h][ip][i].Normalize();
					outProb[h][ip][i].Normalize();
					outProb2[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearning

	void Viterbi(int hand){//0:RH 1:LH
		for(int i=0;i<testData.size();i+=1){
			testData[i].SetChannelFromFingerNumber();
			testData[i].ConvertFingerNumberToInt();
			vector<int> pos;
			for(int n=0;n<testData[i].evts.size();n+=1){
				if(hand==0 && testData[i].evts[n].finger>0){pos.push_back(n);}//endif
				if(hand==1 && testData[i].evts[n].finger<0){pos.push_back(n);}//endif
			}//endfor n

			if(pos.size()<3){continue;}
//			assert(pos.size()>=3);

			vector<vector<double> > LP;//LP[kp][k]
			LP.resize(5);
			for(int k=0;k<5;k+=1){LP[k].resize(5);}
			vector<vector<vector<double> > > amax;//amax[n][kp][k]->kpp
			amax.resize(pos.size());
			for(int n=0;n<pos.size();n+=1){
				amax[n].resize(5);
				for(int k=0;k<5;k+=1){amax[n][k].resize(5);}
			}//endfor n

{
			KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[1]].pitch),PitchToKeyPos(testData[i].evts[pos[0]].pitch));
			if(keyInt.x<-widthX){keyInt.x=-widthX;}
			if(keyInt.x>widthX){keyInt.x=widthX;}
			for(int kp=0;kp<5;kp+=1){
			for(int k=0;k<5;k+=1){
				LP[kp][k]=iniProb[hand].LP[kp]+trProb[hand][kp].LP[k]+outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1];
			}//endfor k
			}//endfor kp
}//

			for(int n=2;n<pos.size();n+=1){
				vector<vector<double> > preLP;//LP[kp][k]
				preLP=LP;
				double logP;
				KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-1]].pitch));
				if(keyInt.x<-widthX){keyInt.x=-widthX;}
				if(keyInt.x>widthX){keyInt.x=widthX;}
				KeyPos keyInt2=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-2]].pitch));
				if(keyInt2.x<-widthX){keyInt2.x=-widthX;}
				if(keyInt2.x>widthX){keyInt2.x=widthX;}

				bool shortTime=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-1]].ontime)<0.03;
				int delPitch=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-1]].pitch;
				bool shortTime2=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-2]].ontime)<0.03;
				int delPitch2=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-2]].pitch;

				for(int kp=0;kp<5;kp+=1){
				for(int k=0;k<5;k+=1){
					LP[kp][k]=preLP[0][kp]+trProb2[hand][0][kp].LP[k]+w1*outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+w2*outProb2[hand][0][k].LP[3*(keyInt2.x+widthX)+keyInt2.y+1]
					          +((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0)+((shortTime2&&((hand==0&&(k)*delPitch2<0)||(hand==1&&(k)*delPitch2>0)))? shortTimeCost:0);
					amax[n][kp][k]=0;
					for(int kpp=1;kpp<5;kpp+=1){
						logP=preLP[kpp][kp]+trProb2[hand][kpp][kp].LP[k]+w1*outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+w2*outProb2[hand][kpp][k].LP[3*(keyInt2.x+widthX)+keyInt2.y+1]
						     +((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0)+((shortTime2&&((hand==0&&(k-kpp)*delPitch2<0)||(hand==1&&(k-kpp)*delPitch2>0)))? shortTimeCost:0);
						if(logP>LP[kp][k]){
							LP[kp][k]=logP;
							amax[n][kp][k]=kpp;
						}//endif
					}//endfor kpp
				}//endfor k
				}//endfor kp
			}//endfor n

			vector<int> optPath(pos.size());
			optPath[pos.size()-1]=0;
			optPath[pos.size()-2]=0;
			for(int kp=0;kp<5;kp+=1){
			for(int k=0;k<5;k+=1){
				if(LP[kp][k]>LP[optPath[pos.size()-2]][optPath[pos.size()-1]]){
					optPath[pos.size()-1]=k;
					optPath[pos.size()-2]=kp;
				}//endif
			}//endfor k
			}//endfor kp
			for(int n=pos.size()-3;n>=0;n-=1){
				optPath[n]=amax[n+2][optPath[n+1]][optPath[n+2]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<pos.size();n+=1){
				ss.str(""); ss<<((hand==0)? "":"-")<<(optPath[n]+1);
				testData[i].evts[pos[n]].fingerNum=ss.str();
			}//endfor n

		}//endfor i
	}//end Viterbi

	void ViterbiTwoHands(){
		Viterbi(0);
		Viterbi(1);
	}//end ViterbiTwoHands

};//endclass FingeringHMM_2nd




class FingeringHMM_3rd : public FingeringHMM_2nd{
public:
//	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
//	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
//	vector<vector<vector<Prob<int> > > > trProb2;//[0,1]=Right,Left x 5 x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb2;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	vector<vector<vector<vector<Prob<int> > > > > trProb3;//[0,1]=Right,Left x 5 x 5 x 5 x 5
	vector<vector<vector<Prob<int> > > > outProb3;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
//	int widthX;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [dx+widthX]
//	int nOut;//=3*(2*widthX+1)

	double w1,w2,w3;//weights for the output probabilities 
	double lam1,lam2;//linear interpolation coefficients for transition probabilities

//	vector<PianoFingering> trainData;
//	vector<PianoFingering> testData;
//	vector<string> trainDataID;
//	vector<string> testDataID;

	FingeringHMM_3rd(double w1_=0.667,double w2_=0.5,double w3_=0.2,double lam1_=0,double lam2_=0.9,double shortTimeCost_=-5){
		widthX=15;
		nOut=3*(2*widthX+1);
		w1=w1_;//1./1.5
		w2=w2_;//1./2.
		w3=w3_;//1./5.
		lam1=lam1_;
		lam2=lam2_;
		shortTimeCost=shortTimeCost_;
	}//end FingeringHMM_3rd
	~FingeringHMM_3rd(){}//end ~FingeringHMM_3rd

	void RandomInit(){
		iniProb.clear();
		trProb.clear();
		outProb.clear();
		trProb2.clear();
		outProb2.clear();
		trProb3.clear();
		outProb3.clear();
		iniProb.resize(2);
		trProb.resize(2);
		outProb.resize(2);
		trProb2.resize(2);
		outProb2.resize(2);
		trProb3.resize(2);
		outProb3.resize(2);
		for(int i=0;i<2;i+=1){
			iniProb[i].Resize(5);
			iniProb[i].Randomize();
			trProb[i].resize(5);
			outProb[i].resize(5);
			trProb2[i].resize(5);
			outProb2[i].resize(5);
			trProb3[i].resize(5);
			outProb3[i].resize(5);
			for(int j=0;j<5;j+=1){
				trProb[i][j].Resize(5);
				trProb[i][j].Randomize();
				outProb[i][j].resize(5);
				trProb2[i][j].resize(5);
				outProb2[i][j].resize(5);
				trProb3[i][j].resize(5);
				outProb3[i][j].resize(5);
				for(int k=0;k<5;k+=1){
					trProb2[i][j][k].Resize(5);
					trProb2[i][j][k].Randomize();
					trProb3[i][j][k].resize(5);
					for(int kp=0;kp<5;kp+=1){
						trProb3[i][j][k][kp].Resize(5);
						trProb3[i][j][k][kp].Randomize();
					}//endfofr kp
					outProb[i][j][k].Resize(nOut);
					outProb[i][j][k].Randomize();
					outProb2[i][j][k].Resize(nOut);
					outProb2[i][j][k].Randomize();
					outProb3[i][j][k].Resize(nOut);
					outProb3[i][j][k].Randomize();
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void SmoothInit(double eps_ini=1E-3,double eps_tr=1E-3,double eps_out=1E-3){
		RandomInit();
		for(int i=0;i<2;i+=1){
			iniProb[i].P.assign(5,eps_ini);
			for(int j=0;j<5;j+=1){
				trProb[i][j].P.assign(5,eps_tr);
				for(int k=0;k<5;k+=1){
					trProb2[i][j][k].P.assign(5,eps_tr);
					for(int kp=0;kp<5;kp+=1){
						trProb3[i][j][k][kp].P.assign(5,eps_tr);
					}//endfor kp
					outProb[i][j][k].P.assign(nOut,eps_out);
					outProb2[i][j][k].P.assign(nOut,eps_out);
					outProb3[i][j][k].P.assign(nOut,eps_out);
				}//endfor k
			}//endfor j
		}//endfor i
	}//end SmoothInit

	void SetUnifTrProb(){
		for(int h=0;h<2;h+=1){
			for(int i=0;i<5;i+=1){
				trProb[h][i].P.assign(5,1);
				trProb[h][i].Normalize();
				for(int ip=0;ip<5;ip+=1){
					trProb2[h][i][ip].P.assign(5,1);
					trProb2[h][i][ip].Normalize();
					for(int ipp=0;ipp<5;ipp+=1){
						trProb3[h][i][ip][ipp].P.assign(5,1);
						trProb3[h][i][ip][ipp].Normalize();
					}//endfor ipp
				}//endfor ip
			}//endfor i
		}//endfor h
	}//end SetUnifTrProb

	void WriteParamFile(string filename){
		ofstream ofs(filename.c_str());

		ofs<<"### Initial Prob Right\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[0].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		ofs<<"### Initial Prob Left\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[1].P[i]<<"\t";
			}//endfor i
ofs<<"\n";

		ofs<<"### Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Transition Prob 2nd Right\n";
		for(int ipp=0;ipp<5;ipp+=1){
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
ofs<<trProb2[0][ipp][ip].P[i]<<"\t";
				}//endfor i
ofs<<"\n";
			}//endfor ip
		}//endfor ipp

		ofs<<"### Transition Prob 2nd Left\n";
		for(int ipp=0;ipp<5;ipp+=1){
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
ofs<<trProb2[1][ipp][ip].P[i]<<"\t";
				}//endfor i
ofs<<"\n";
			}//endfor ip
		}//endfor ipp

		ofs<<"### Transition Prob 3rd Right\n";
		for(int ippp=0;ippp<5;ippp+=1){
			for(int ipp=0;ipp<5;ipp+=1){
				for(int ip=0;ip<5;ip+=1){
					for(int i=0;i<5;i+=1){
ofs<<trProb3[0][ippp][ipp][ip].P[i]<<"\t";
					}//endfor i
ofs<<"\n";
				}//endfor ip
			}//endfor ipp
		}//endfor ippp

		ofs<<"### Transition Prob 3rd Left\n";
		for(int ippp=0;ippp<5;ippp+=1){
			for(int ipp=0;ipp<5;ipp+=1){
				for(int ip=0;ip<5;ip+=1){
					for(int i=0;i<5;i+=1){
ofs<<trProb3[1][ippp][ipp][ip].P[i]<<"\t";
					}//endfor i
ofs<<"\n";
				}//endfor ip
			}//endfor ipp
		}//endfor ippp

		ofs<<"### Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 2nd Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb2[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 2nd Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb2[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 3rd Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb3[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob 3rd Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb3[1][ip][i].P[x]<<"\t";
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
			getline(ifs,s[99]);//### Initial Prob Right
				for(int i=0;i<5;i+=1){
				ifs>>iniProb[h].P[i];
			}//endfor i
			getline(ifs,s[99]);
			iniProb[h].Normalize();
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>trProb[h][ip].P[i];
				}//endfor i
				trProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob 2nd Right
			for(int ipp=0;ipp<5;ipp+=1){
				for(int ip=0;ip<5;ip+=1){
					for(int i=0;i<5;i+=1){
						ifs>>trProb2[h][ipp][ip].P[i];
					}//endfor i
					trProb2[h][ipp][ip].Normalize();
				}//endfor ip
			}//endfor ipp
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob 3rd Right,Left
			for(int ippp=0;ippp<5;ippp+=1){
				for(int ipp=0;ipp<5;ipp+=1){
					for(int ip=0;ip<5;ip+=1){
						for(int i=0;i<5;i+=1){
							ifs>>trProb3[h][ippp][ipp][ip].P[i];
						}//endfor i
						trProb3[h][ippp][ipp][ip].Normalize();
					}//endfor ip
				}//endfor ipp
			}//endfor ippp
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb[h][ip][i].P[x];
					}//endfor x
					outProb[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob 2nd Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb2[h][ip][i].P[x];
					}//endfor x
					outProb2[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob 3rd Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb3[h][ip][i].P[x];
					}//endfor x
					outProb3[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		ifs.close();

//cout<<lam1<<"\t"<<lam2<<endl;

		vector<vector<vector<vector<Prob<int> > > > > trProb3_tmp;
		trProb3_tmp=trProb3;
		for(int h=0;h<2;h+=1){
			for(int ippp=0;ippp<5;ippp+=1){
				for(int ipp=0;ipp<5;ipp+=1){
					for(int ip=0;ip<5;ip+=1){
						for(int i=0;i<5;i+=1){
							trProb3[h][ippp][ipp][ip].P[i]=(1-lam2-lam1)*trProb3_tmp[h][ippp][ipp][ip].P[i]+lam2*trProb2[h][ipp][ip].P[i]+lam1*trProb[h][ip].P[i];
						}//endfor i
						trProb3[h][ippp][ipp][ip].Normalize();
					}//endfor ip
				}//endfor ipp
			}//endfor ippp
		}//endfor h

	}//end ReadParamFile

	void SupervisedLearning(bool TRSym=false,bool RFSym=false){
		SmoothInit();
		for(int i=0;i<trainData.size();i+=1){
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;

					if(TRSym){
						trProb[h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb[1-h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-1].pitch));
					if(keyInt.x<-widthX){keyInt.x=-widthX;}
					if(keyInt.x>widthX){keyInt.x=widthX;}
					outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;

					if(TRSym){
						outProb[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)-keyInt.y+1]+=1;
					}//endif
					if(RFSym){
						outProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)+keyInt.y+1]+=1;
						if(TRSym){
							outProb[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)-keyInt.y+1]+=1;
						}//endif
					}//endif


					if(n<2){continue;}

					trProb2[h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;

					if(TRSym){
						trProb2[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n-2].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb2[1-h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb2[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n-2].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt2=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-2].pitch));
					if(keyInt2.x<-widthX){keyInt2.x=-widthX;}
					if(keyInt2.x>widthX){keyInt2.x=widthX;}
					outProb2[h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt2.x+widthX)+keyInt2.y+1]+=1;

					if(TRSym){
						outProb2[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[3*(-keyInt2.x+widthX)-keyInt2.y+1]+=1;
					}//endif
					if(RFSym){
						outProb2[1-h][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt2.x+widthX)+keyInt2.y+1]+=1;
						if(TRSym){
							outProb2[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[3*(keyInt2.x+widthX)-keyInt2.y+1]+=1;
						}//endif
					}//endif

					if(n<3){continue;}

					trProb3[h][fingering.evts[n-3].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;

					if(TRSym){
						trProb3[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[fingering.evts[n-3].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb3[1-h][fingering.evts[n-3].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb3[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n-2].finger*(1-2*h)-1].P[fingering.evts[n-3].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					KeyPos keyInt3=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-3].pitch));
					if(keyInt3.x<-widthX){keyInt3.x=-widthX;}
					if(keyInt3.x>widthX){keyInt3.x=widthX;}
					outProb3[h][fingering.evts[n-3].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt3.x+widthX)+keyInt3.y+1]+=1;

					if(TRSym){
						outProb3[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-3].finger*(1-2*h)-1].P[3*(-keyInt3.x+widthX)-keyInt3.y+1]+=1;
					}//endif
					if(RFSym){
						outProb3[1-h][fingering.evts[n-3].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(-keyInt3.x+widthX)+keyInt3.y+1]+=1;
						if(TRSym){
							outProb3[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-3].finger*(1-2*h)-1].P[3*(keyInt3.x+widthX)-keyInt3.y+1]+=1;
						}//endif
					}//endif

				}//endfor n
			}//endfor h
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					trProb2[h][ip][i].Normalize();
					for(int k=0;k<5;k+=1){
						trProb3[h][k][ip][i].Normalize();
					}//endfor k
					outProb[h][ip][i].Normalize();
					outProb2[h][ip][i].Normalize();
					outProb3[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearning

	void Viterbi(int hand){//0:RH 1:LH
		for(int i=0;i<testData.size();i+=1){
// 			testData[i].SetChannelFromFingerNumber();
// 			testData[i].ConvertFingerNumberToInt();
			vector<int> pos;
			for(int n=0;n<testData[i].evts.size();n+=1){
// 				if(hand==0 && testData[i].evts[n].finger>0){pos.push_back(n);}//endif
// 				if(hand==1 && testData[i].evts[n].finger<0){pos.push_back(n);}//endif
				if(hand==0 && testData[i].evts[n].channel==0){pos.push_back(n);}//endif
				if(hand==1 && testData[i].evts[n].channel==1){pos.push_back(n);}//endif
			}//endfor n

			if(pos.size()<4){continue;}

			vector<vector<vector<double> > > LP;//LP[kpp][kp][k]
			LP.resize(5);
			for(int k=0;k<5;k+=1){
				LP[k].resize(5);
				for(int kp=0;kp<5;kp+=1){
					LP[k][kp].resize(5);
				}//endfor kp
			}//endfor k
			vector<vector<vector<vector<double> > > > amax;//amax[n][kpp][kp][k]->kppp
			amax.resize(pos.size());
			for(int n=0;n<pos.size();n+=1){
				amax[n].resize(5);
				for(int k=0;k<5;k+=1){
					amax[n][k].resize(5);
					for(int kp=0;kp<5;kp+=1){
						amax[n][k][kp].resize(5);
					}//endfor kp
				}//endfor k
			}//endfor n

{
			KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[1]].pitch),PitchToKeyPos(testData[i].evts[pos[0]].pitch));
			if(keyInt.x<-widthX){keyInt.x=-widthX;}
			if(keyInt.x>widthX){keyInt.x=widthX;}
			KeyPos keyInt2=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[2]].pitch),PitchToKeyPos(testData[i].evts[pos[0]].pitch));
			if(keyInt2.x<-widthX){keyInt2.x=-widthX;}
			if(keyInt2.x>widthX){keyInt2.x=widthX;}
			KeyPos keyInt3=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[2]].pitch),PitchToKeyPos(testData[i].evts[pos[1]].pitch));
			if(keyInt3.x<-widthX){keyInt3.x=-widthX;}
			if(keyInt3.x>widthX){keyInt3.x=widthX;}
			for(int kpp=0;kpp<5;kpp+=1){
			for(int kp=0;kp<5;kp+=1){
			for(int k=0;k<5;k+=1){
				LP[kpp][kp][k]=iniProb[hand].LP[kpp]+trProb[hand][kpp].LP[kp]+trProb[hand][kp].LP[k]
				               +outProb[hand][kpp][kp].LP[3*(keyInt.x+widthX)+keyInt.y+1]
				               +outProb[hand][kp][k].LP[3*(keyInt3.x+widthX)+keyInt3.y+1];
			}//endfor k
			}//endfor kp
			}//endfor kpp
}//

			for(int n=3;n<pos.size();n+=1){
				vector<vector<vector<double> > > preLP;//LP[kpp][kp][k]
				preLP=LP;
				double logP;
				KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-1]].pitch));
				if(keyInt.x<-widthX){keyInt.x=-widthX;}
				if(keyInt.x>widthX){keyInt.x=widthX;}
				KeyPos keyInt2=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-2]].pitch));
				if(keyInt2.x<-widthX){keyInt2.x=-widthX;}
				if(keyInt2.x>widthX){keyInt2.x=widthX;}
				KeyPos keyInt3=SubtrKeyPos(PitchToKeyPos(testData[i].evts[pos[n]].pitch),PitchToKeyPos(testData[i].evts[pos[n-3]].pitch));
				if(keyInt3.x<-widthX){keyInt3.x=-widthX;}
				if(keyInt3.x>widthX){keyInt3.x=widthX;}
				bool shortTime=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-1]].ontime)<0.03;
				int delPitch=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-1]].pitch;
				bool shortTime2=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-2]].ontime)<0.03;
				int delPitch2=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-2]].pitch;
				bool shortTime3=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-3]].ontime)<0.03;
				int delPitch3=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-3]].pitch;

				for(int kpp=0;kpp<5;kpp+=1){
				for(int kp=0;kp<5;kp+=1){
				for(int k=0;k<5;k+=1){
					LP[kpp][kp][k]=preLP[0][kpp][kp]+trProb3[hand][0][kpp][kp].LP[k]+w1*outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]
					               +w2*outProb2[hand][kpp][k].LP[3*(keyInt2.x+widthX)+keyInt2.y+1]+w3*outProb3[hand][0][k].LP[3*(keyInt3.x+widthX)+keyInt3.y+1]
						           +((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0)
						           +((shortTime2&&((hand==0&&(k-kpp)*delPitch2<0)||(hand==1&&(k-kpp)*delPitch2>0)))? shortTimeCost:0)
						           +((shortTime3&&((hand==0&&(k)*delPitch3<0)||(hand==1&&(k)*delPitch3>0)))? shortTimeCost:0);
					amax[n][kpp][kp][k]=0;
					for(int kppp=1;kppp<5;kppp+=1){
						logP=preLP[kppp][kpp][kp]+trProb3[hand][kppp][kpp][kp].LP[k]+w1*outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]
						     +w2*outProb2[hand][kpp][k].LP[3*(keyInt2.x+widthX)+keyInt2.y+1]+w3*outProb3[hand][kppp][k].LP[3*(keyInt3.x+widthX)+keyInt3.y+1]
						     +((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0)
						     +((shortTime2&&((hand==0&&(k-kpp)*delPitch2<0)||(hand==1&&(k-kpp)*delPitch2>0)))? shortTimeCost:0)
						     +((shortTime3&&((hand==0&&(k-kppp)*delPitch3<0)||(hand==1&&(k-kppp)*delPitch3>0)))? shortTimeCost:0);
						if(logP>LP[kpp][kp][k]){
							LP[kpp][kp][k]=logP;
							amax[n][kpp][kp][k]=kppp;
						}//endif
					}//endfor kppp
				}//endfor k
				}//endfor kp
				}//endfor kpp
			}//endfor n

			vector<int> optPath(pos.size());
			optPath[pos.size()-1]=0;
			optPath[pos.size()-2]=0;
			optPath[pos.size()-3]=0;
			for(int kpp=0;kpp<5;kpp+=1){
			for(int kp=0;kp<5;kp+=1){
			for(int k=0;k<5;k+=1){
				if(LP[kpp][kp][k]>LP[optPath[pos.size()-3]][optPath[pos.size()-2]][optPath[pos.size()-1]]){
					optPath[pos.size()-1]=k;
					optPath[pos.size()-2]=kp;
					optPath[pos.size()-3]=kpp;
				}//endif
			}//endfor k
			}//endfor kp
			}//endfor kpp
			for(int n=pos.size()-4;n>=0;n-=1){
				optPath[n]=amax[n+3][optPath[n+1]][optPath[n+2]][optPath[n+3]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<pos.size();n+=1){
				ss.str(""); ss<<((hand==0)? "":"-")<<(optPath[n]+1);
				testData[i].evts[pos[n]].fingerNum=ss.str();
			}//endfor n

		}//endfor i
	}//end Viterbi

	void ViterbiTwoHands(){
		Viterbi(0);
		Viterbi(1);
	}//end ViterbiTwoHands

};//endclass FingeringHMM_3rd




class FingeringHMMTD : public FingeringHMM{
public:
//	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
//	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1
	vector<vector<Prob<int> > > trProbS;//[0,1]=Right,Left x 5 x 5//short time
	vector<vector<vector<Prob<int> > > > outProbS;//[0,1]=Right,Left x 5 x 5 x ((2*widthX+1)*3), dkey%3=dY+1, dkey/3=dX+widthX, dkey=3*(dX+widthX)+dY+1//short time
//	int widthX;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [dx+widthX]
//	int nOut;//=3*(2*widthX+1)

//	vector<PianoFingering> trainData;
//	vector<PianoFingering> testData;
//	vector<string> trainDataID;
//	vector<string> testDataID;

	vector<PianoFingering> trainDataTD;//time-dependent pitch ordering
	vector<PianoFingering> testDataTD;//time-dependent pitch ordering

	FingeringHMMTD(){
		widthX=15;
		nOut=3*(2*widthX+1);
	}//end FingeringHMMTD
	~FingeringHMMTD(){}//end ~FingeringHMMTD

	void RandomInit(){
		iniProb.clear();
		trProb.clear();
		outProb.clear();
		iniProb.resize(2);
		trProb.resize(2);
		outProb.resize(2);
		trProbS.resize(2);
		outProbS.resize(2);
		for(int i=0;i<2;i+=1){
			iniProb[i].Resize(5);
			iniProb[i].Randomize();
			trProb[i].resize(5);
			outProb[i].resize(5);
			trProbS[i].resize(5);
			outProbS[i].resize(5);
			for(int j=0;j<5;j+=1){
				trProb[i][j].Resize(5);
				trProb[i][j].Randomize();
				trProbS[i][j].Resize(5);
				trProbS[i][j].Randomize();
				outProb[i][j].resize(5);
				outProbS[i][j].resize(5);
				for(int k=0;k<5;k+=1){
					outProb[i][j][k].Resize(nOut);
					outProb[i][j][k].Randomize();
					outProbS[i][j][k].Resize(nOut);
					outProbS[i][j][k].Randomize();
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void SmoothInit(double eps_ini=1E-3,double eps_tr=1E-3,double eps_out=1E-3){
		RandomInit();
		for(int i=0;i<2;i+=1){
			iniProb[i].P.assign(5,eps_ini);
			for(int j=0;j<5;j+=1){
				trProb[i][j].P.assign(5,eps_tr);
				trProbS[i][j].P.assign(5,eps_tr);
				for(int k=0;k<5;k+=1){
					outProb[i][j][k].P.assign(nOut,eps_out);
					outProbS[i][j][k].P.assign(nOut,eps_out);
				}//endfor k
			}//endfor j
		}//endfor i
	}//end RandomInit

	void WriteParamFile(string filename){
		ofstream ofs(filename.c_str());

		ofs<<"### Initial Prob Right\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[0].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		ofs<<"### Initial Prob Left\n";
			for(int i=0;i<5;i+=1){
ofs<<iniProb[1].P[i]<<"\t";
			}//endfor i
ofs<<"\n";

		ofs<<"### Transition Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip
		ofs<<"### Transition Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProb[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Transition Prob Right short time\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProbS[0][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip
		ofs<<"### Transition Prob Left short time\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<trProbS[1][ip].P[i]<<"\t";
			}//endfor i
ofs<<"\n";
		}//endfor ip

		ofs<<"### Output Prob Right\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip
		ofs<<"### Output Prob Left\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProb[1][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip

		ofs<<"### Output Prob Right short time\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProbS[0][ip][i].P[x]<<"\t";
				}//endfor x
ofs<<"\n";
			}//endfor i
		}//endfor ip
		ofs<<"### Output Prob Left short time\n";
		for(int ip=0;ip<5;ip+=1){
			for(int i=0;i<5;i+=1){
ofs<<ip+1<<" "<<i+1<<"\t";
				for(int x=0;x<nOut;x+=1){
ofs<<outProbS[1][ip][i].P[x]<<"\t";
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
			getline(ifs,s[99]);//### Initial Prob Right
				for(int i=0;i<5;i+=1){
				ifs>>iniProb[h].P[i];
			}//endfor i
			getline(ifs,s[99]);
			iniProb[h].Normalize();
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>trProb[h][ip].P[i];
				}//endfor i
				trProb[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Transition Prob Right short time
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>trProbS[h][ip].P[i];
				}//endfor i
				trProbS[h][ip].Normalize();
			}//endfor ip
			getline(ifs,s[99]);
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProb[h][ip][i].P[x];
					}//endfor x
					outProb[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		for(int h=0;h<2;h+=1){
			getline(ifs,s[99]);//### Output Prob Right short time
			for(int ip=0;ip<5;ip+=1){
				for(int i=0;i<5;i+=1){
					ifs>>v[1]>>v[2];
					for(int x=0;x<nOut;x+=1){
						ifs>>outProbS[h][ip][i].P[x];
					}//endfor x
					outProbS[h][ip][i].Normalize();
				}//endfor i
			getline(ifs,s[99]);
			}//endfor ip
		}//endfor h

		ifs.close();

	}//end ReadParamFile

	void OrderData(){
		trainDataTD=trainData;
		testDataTD=testData;
		for(int i=0;i<trainDataTD.size();i+=1){
			for(int n=0;n<trainDataTD[i].evts.size();n+=1){
				trainDataTD[i].evts[n].ext1=n;//Original pos
			}//endfor n
			trainDataTD[i].TimeDepPitchOrder();
		}//endfor i
		for(int i=0;i<testDataTD.size();i+=1){
			for(int n=0;n<testDataTD[i].evts.size();n+=1){
				testDataTD[i].evts[n].ext1=n;//Original pos
			}//endfor n
			testDataTD[i].TimeDepPitchOrder();
		}//endfor i
	}//end OrderData

	void SupervisedLearning(){
		SmoothInit();
		OrderData();
		for(int i=0;i<trainDataTD.size();i+=1){
//cout<<trainData[i].evts.size()<<"\t";
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainDataTD[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
//cout<<fingering.evts.size()<<"\t";
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-1].pitch));
					if(keyInt.x<-widthX){keyInt.x=-widthX;}
					if(keyInt.x>widthX){keyInt.x=widthX;}
					if(abs(fingering.evts[n].ontime-fingering.evts[n-1].ontime)>=0.03){
						trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
					}else{
						trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
						trProbS[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						outProbS[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
					}//endif
				}//endfor n
			}//endfor h
//cout<<endl;
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				trProbS[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					outProb[h][ip][i].Normalize();
					outProbS[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearning

	void SupervisedLearningTR(){//Time reversal symmetry
		SmoothInit();
		OrderData();
		for(int i=0;i<trainDataTD.size();i+=1){
//cout<<trainData[i].evts.size()<<"\t";
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainDataTD[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
//cout<<fingering.evts.size()<<"\t";
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(fingering.evts[n].pitch),PitchToKeyPos(fingering.evts[n-1].pitch));
					if(keyInt.x<-widthX){keyInt.x=-widthX;}
					if(keyInt.x>widthX){keyInt.x=widthX;}
					if(abs(fingering.evts[n].ontime-fingering.evts[n-1].ontime)>=0.03){
						trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						trProb[h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
						outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
						outProb[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[3*(-keyInt.x+widthX)-keyInt.y+1]+=1;
					}else{
						trProbS[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						outProbS[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[3*(keyInt.x+widthX)+keyInt.y+1]+=1;
					}//endif
				}//endfor n
			}//endfor h
//cout<<endl;
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				trProbS[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					outProb[h][ip][i].Normalize();
					outProbS[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearningTR

	void Viterbi(int hand){//0:RH 1:LH
		for(int i=0;i<testDataTD.size();i+=1){
// 			testDataTD[i].SetChannelFromFingerNumber();
// 			testDataTD[i].ConvertFingerNumberToInt();
			vector<int> pos;
			for(int n=0;n<testDataTD[i].evts.size();n+=1){
// 				if(hand==0 && testDataTD[i].evts[n].finger>0){pos.push_back(n);}//endif
// 				if(hand==1 && testDataTD[i].evts[n].finger<0){pos.push_back(n);}//endif
				if(hand==0 && testData[i].evts[n].channel==0){pos.push_back(n);}//endif
				if(hand==1 && testData[i].evts[n].channel==1){pos.push_back(n);}//endif
			}//endfor n

			vector<double> LP(5);
			vector<vector<double> > amax;
			amax.resize(pos.size());

			amax[0].resize(5);
			for(int k=0;k<5;k+=1){
				LP[k]=iniProb[hand].LP[k];
			}//endfor k

			for(int n=1;n<pos.size();n+=1){
				amax[n].resize(5);
				vector<double> preLP(LP);
				double logP;
				KeyPos keyInt=SubtrKeyPos(PitchToKeyPos(testDataTD[i].evts[pos[n]].pitch),PitchToKeyPos(testDataTD[i].evts[pos[n-1]].pitch));
				if(keyInt.x<-widthX){keyInt.x=-widthX;}
				if(keyInt.x>widthX){keyInt.x=widthX;}

				bool shortTime=abs(testDataTD[i].evts[pos[n]].ontime-testDataTD[i].evts[pos[n-1]].ontime)<0.03;
				int delPitch=testDataTD[i].evts[pos[n]].pitch-testDataTD[i].evts[pos[n-1]].pitch;

				for(int k=0;k<5;k+=1){
					if(shortTime){
						LP[k]=preLP[0]+trProb[hand][0].LP[k]+outProb[hand][0][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+(((hand==0&&k*delPitch<0)|(hand==1&&k*delPitch>0))? -1:0);
					}else{
						LP[k]=preLP[0]+trProb[hand][0].LP[k]+outProb[hand][0][k].LP[3*(keyInt.x+widthX)+keyInt.y+1];
					}//endif
					amax[n][k]=0;
					for(int kp=1;kp<5;kp+=1){
						if(shortTime){
							logP=preLP[kp]+trProb[hand][kp].LP[k]+outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1]+(((hand==0&&(k-kp)*delPitch<0)|(hand==1&&(k-kp)*delPitch>0))? -1:0);
						}else{
							logP=preLP[kp]+trProb[hand][kp].LP[k]+outProb[hand][kp][k].LP[3*(keyInt.x+widthX)+keyInt.y+1];
						}//endif
						if(logP>LP[k]){
							LP[k]=logP;
							amax[n][k]=kp;
						}//endif
					}//endfor kp
				}//endfor k
			}//endfor n

			vector<int> optPath(pos.size());
			optPath[pos.size()-1]=0;
			for(int k=1;k<5;k+=1){
				if(LP[k]>LP[optPath[pos.size()-1]]){optPath[pos.size()-1]=k;}
			}//endfor k
			for(int n=pos.size()-2;n>=0;n-=1){
				optPath[n]=amax[n+1][optPath[n+1]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<pos.size();n+=1){
				ss.str(""); ss<<((hand==0)? "":"-")<<(optPath[n]+1);
				testDataTD[i].evts[pos[n]].fingerNum=ss.str();
				testData[i].evts[testDataTD[i].evts[pos[n]].ext1].fingerNum=ss.str();
			}//endfor n

		}//endfor i
	}//end Viterbi

	void ViterbiTwoHands(){
		Viterbi(0);
		Viterbi(1);
	}//end ViterbiTwoHands

};//endclass FingeringHMMTD


class FingeringHMMIntPitch : public FingeringHMM{
public:
//	vector<Prob<int> > iniProb;//[0,1]=Right,Left 0,...,4 <-> finger 1,...,5
//	vector<vector<Prob<int> > > trProb;//[0,1]=Right,Left x 5 x 5
//	vector<vector<vector<Prob<int> > > > outProb;//[0,1]=Right,Left x 5 x 5 x (2*width+1) var=interval+width
	int width;//15 <-> -inf=-15,-14,...,0,...,14,15=inf (dim = 2*widthX + 1) [interval+width]
//	int nOut;//=2*widthX+1

//	double w1;//weights for the output probabilities
//	double shortTimeCost;

// 	vector<PianoFingering> trainData;
// 	vector<PianoFingering> testData;
// 	vector<string> trainDataID;
// 	vector<string> testDataID;
// 
// 	vector<PianoFingering> trainDataTD;//time-dependent pitch ordering
// 	vector<PianoFingering> testDataTD;//time-dependent pitch ordering

//	vector<vector<vector<vector<double> > > > ForwardVar;
//	vector<vector<vector<double> > > BackwardVar;

	FingeringHMMIntPitch(double w1_=1,double shortTimeCost_=-5){
		width=15;
		nOut=2*width+1;
		w1=w1_;
		shortTimeCost=shortTimeCost_;
	}//end FingeringHMMIntPitch
	~FingeringHMMIntPitch(){}//end ~FingeringHMMIntPitch

	void SupervisedLearning(bool TRSym=false,bool RFSym=false){
		SmoothInit();
		for(int i=0;i<trainData.size();i+=1){
//cout<<trainData[i].evts.size()<<"\t";
			for(int h=0;h<2;h+=1){
				PianoFingering fingering;
				fingering=trainData[i];
				fingering.SelectHandByFingerNum(h);
				fingering.ConvertFingerNumberToInt();
//cout<<fingering.evts.size()<<"\t";
				for(int n=0;n<fingering.evts.size();n+=1){
					if(n==0){
						iniProb[h].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						continue;
					}//endif
					trProb[h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
					if(TRSym){
						trProb[h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
					}//endif
					if(RFSym){
						trProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1].P[fingering.evts[n].finger*(1-2*h)-1]+=1;
						if(TRSym){
							trProb[1-h][fingering.evts[n].finger*(1-2*h)-1].P[fingering.evts[n-1].finger*(1-2*h)-1]+=1;
						}//endif
					}//endif

					int iterval=fingering.evts[n].pitch-fingering.evts[n-1].pitch;
					if(iterval<-width){iterval=-width;}
					if(iterval>width){iterval=width;}
					outProb[h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[iterval+width]+=1;
					if(TRSym){
						outProb[h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[iterval+width]+=1;
					}//endif
					if(RFSym){
						outProb[1-h][fingering.evts[n-1].finger*(1-2*h)-1][fingering.evts[n].finger*(1-2*h)-1].P[iterval+width]+=1;
						if(TRSym){
							outProb[1-h][fingering.evts[n].finger*(1-2*h)-1][fingering.evts[n-1].finger*(1-2*h)-1].P[iterval+width]+=1;
						}//endif
					}//endif
				}//endfor n
			}//endfor h
//cout<<endl;
		}//endfor i

		for(int h=0;h<2;h+=1){
			iniProb[h].Normalize();
			for(int ip=0;ip<5;ip+=1){
				trProb[h][ip].Normalize();
				for(int i=0;i<5;i+=1){
					outProb[h][ip][i].Normalize();
				}//endfor i
			}//endfor ip
		}//endfor h
	}//end SupervisedLearning

	void Viterbi(int hand){//0:RH 1:LH

		for(int i=0;i<testData.size();i+=1){
// 			testData[i].SetChannelFromFingerNumber();
// 			testData[i].ConvertFingerNumberToInt();
			vector<int> pos;
			for(int n=0;n<testData[i].evts.size();n+=1){
				if(hand==0 && testData[i].evts[n].channel==0){pos.push_back(n);}//endif
				if(hand==1 && testData[i].evts[n].channel==1){pos.push_back(n);}//endif
			}//endfor n

			vector<double> LP(5);
			vector<vector<double> > amax;
			amax.resize(pos.size());

			amax[0].resize(5);
			for(int k=0;k<5;k+=1){
				LP[k]=iniProb[hand].LP[k];
			}//endfor k

			for(int n=1;n<pos.size();n+=1){
				amax[n].resize(5);
				vector<double> preLP(LP);
				double logP;
				int iterval=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-1]].pitch;
				if(iterval<-width){iterval=-width;}
				if(iterval>width){iterval=width;}

				bool shortTime=abs(testData[i].evts[pos[n]].ontime-testData[i].evts[pos[n-1]].ontime)<0.03;
				int delPitch=testData[i].evts[pos[n]].pitch-testData[i].evts[pos[n-1]].pitch;

				for(int k=0;k<5;k+=1){
					LP[k]=preLP[0]+trProb[hand][0].LP[k]+w1*outProb[hand][0][k].LP[iterval+width]+((shortTime&&((hand==0&&(k)*delPitch<0)||(hand==1&&(k)*delPitch>0)))? shortTimeCost:0);
					amax[n][k]=0;
					for(int kp=1;kp<5;kp+=1){
						logP=preLP[kp]+trProb[hand][kp].LP[k]+w1*outProb[hand][kp][k].LP[iterval+width]+((shortTime&&((hand==0&&(k-kp)*delPitch<0)||(hand==1&&(k-kp)*delPitch>0)))? shortTimeCost:0);
						if(logP>LP[k]){
							LP[k]=logP;
							amax[n][k]=kp;
						}//endif
					}//endfor kp
				}//endfor k
			}//endfor n

			vector<int> optPath(pos.size());
			optPath[pos.size()-1]=0;
			for(int k=1;k<5;k+=1){
				if(LP[k]>LP[optPath[pos.size()-1]]){optPath[pos.size()-1]=k;}
			}//endfor k
			for(int n=pos.size()-2;n>=0;n-=1){
				optPath[n]=amax[n+1][optPath[n+1]];
			}//endfor n

			stringstream ss;
			for(int n=0;n<pos.size();n+=1){
				ss.str(""); ss<<((hand==0)? "":"-")<<(optPath[n]+1);
				testData[i].evts[pos[n]].fingerNum=ss.str();
			}//endfor n

		}//endfor i
	}//end Viterbi

	void ViterbiTwoHands(){
		Viterbi(0);
		Viterbi(1);
	}//end ViterbiTwoHands

};//endclass FingeringHMMIntPitch





#endif // FingeringHMM_HPP
