#ifndef FingeringEvaluation_HPP
#define FingeringEvaluation_HPP

#include<iostream>
#include<string>
#include<sstream>
#include<cmath>
#include<vector>
#include<fstream>
#include<cassert>
#include"PianoFingering_v170101_2.hpp"

using namespace std;

inline vector<int> GetMatchPos(vector<PianoFingering> fingerings){
	assert(fingerings.size()>1);
	for(int k=1;k<fingerings.size();k+=1){
		assert(fingerings[k].evts.size()==fingerings[0].evts.size());
	}//endfor k

	vector<int> matchPos;
	for(int n=0;n<fingerings[0].evts.size();n+=1){
		bool allMatch=true;
		for(int k=1;k<fingerings.size();k+=1){
			if(fingerings[k].evts[n].fingerNum!=fingerings[0].evts[n].fingerNum){
				allMatch=false;
				break;
			}//endif
		}//endfor k
		if(allMatch){matchPos.push_back(n);}
	}//endfor n

	return matchPos;
}//end GetMatchPos

inline vector<int> GetContextDepMatchPos(vector<PianoFingering> fingerings,int &nPos, bool lookForward,bool lookBackward){
	assert(fingerings.size()==2);
	assert(fingerings[1].evts.size()==fingerings[0].evts.size());

	nPos=0;
	vector<int> matchPos;
	for(int n=0;n<fingerings[0].evts.size();n+=1){
		if(lookForward && n==0){continue;}
		if(lookBackward && n==fingerings[0].evts.size()-1){continue;}

		if(lookForward && fingerings[0].evts[n-1].fingerNum!=fingerings[1].evts[n-1].fingerNum){continue;}
		if(lookBackward && fingerings[0].evts[n+1].fingerNum!=fingerings[1].evts[n+1].fingerNum){continue;}

		nPos+=1;
		if(fingerings[0].evts[n].fingerNum==fingerings[1].evts[n].fingerNum){
			matchPos.push_back(n);
		}//endif
	}//endfor n

	return matchPos;
}//end GetContextDepMatchPos


inline vector<int> GetMultiGTLooseMissmatchPos(vector<PianoFingering> finsGT,PianoFingering finEst){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();
	int nNotes=finEst.evts.size();

	vector<int> missmatchPos;
	for(int n=0;n<finEst.evts.size();n+=1){
		bool oneMatch=false;
		for(int k=0;k<finsGT.size();k+=1){
			if(finsGT[k].evts[n].fingerNum==finEst.evts[n].fingerNum){
				oneMatch=true;
				break;
			}//endif
		}//endfor k
		if(!oneMatch){missmatchPos.push_back(n);}
	}//endfor n

	return missmatchPos;
}//end GetMultiGTLooseMissmatchPos


inline double MultiGTError(vector<PianoFingering> finsGT,PianoFingering finEst,double substCost=1,double softSwitchCost=0.1,double hardSwitchCost=1000){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();

	vector<vector<int> > seqEst(2);//[0]=RH,[1]=LH
	vector<vector<vector<int> > > seqGT(2);//[0]=RH,[1]=LH
	for(int h=0;h<2;h+=1){seqGT[h].resize(nGT);}

	for(int n=0;n<finEst.evts.size();n+=1){
		int fn=atoi(finEst.evts[n].fingerNum.c_str());
		int hand=0;
		if(fn<0){hand=1;}//endif LH
		seqEst[hand].push_back(fn);
		for(int k=0;k<nGT;k+=1){
			fn=atoi(finsGT[k].evts[n].fingerNum.c_str());
			seqGT[hand][k].push_back(fn);
		}//endfor k
	}//endfor n

	vector<double> cumuCost(2);

	for(int h=0;h<2;h+=1){
		int len=seqEst[h].size();
		vector<double> cost(nGT);
		vector<vector<int> > amin(len);
		vector<int> optPath(len);//association to GT;	

		for(int n=0;n<len;n+=1){
			amin[n].resize(nGT);
			if(n==0){
				cost.assign(nGT,0);
				for(int z=0;z<nGT;z+=1){
					if(seqEst[h][n]!=seqGT[h][z][n]){cost[z]+=substCost;}
				}//endfor z
				continue;
			}//endif

			vector<double> preCost(cost);
			double tmpCost;
			for(int z=0;z<nGT;z+=1){
				cost[z]=preCost[z];
				amin[n][z]=z;
				for(int zp=0;zp<nGT;zp+=1){
					if(zp==z){continue;}
					tmpCost=preCost[zp]+((seqGT[h][zp][n-1]==seqGT[h][z][n-1])? softSwitchCost:hardSwitchCost);
					if(tmpCost<cost[z]){
						cost[z]=tmpCost;
						amin[n][z]=zp;
					}//endif
				}//endfor zp
				if(seqEst[h][n]!=seqGT[h][z][n]){cost[z]+=substCost;}
			}//endfor z
		}//endfor n

		optPath[len-1]=0;
		for(int z=0;z<nGT;z+=1){
			if(cost[z]<cost[optPath[len-1]]){optPath[len-1]=z;}
		}//endfor z
		for(int n=len-2;n>=0;n-=1){
			optPath[n]=amin[n+1][optPath[n+1]];
		}//endfor n
		cumuCost[h]=cost[optPath[len-1]];
	}//endfor h

	return cumuCost[0]+cumuCost[1];
}//end MultiGTError

inline double AveragePairwiseMatchRate(vector<PianoFingering> finsGT,PianoFingering finEst){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();
	int nNotes=finEst.evts.size();

	vector<double> matchRates(nGT);

	for(int k=0;k<finsGT.size();k+=1){
		vector<PianoFingering> pair;
		pair.push_back(finsGT[k]);
		pair.push_back(finEst);
		vector<int> matchPos=GetMatchPos(pair);
		matchRates[k]=double(matchPos.size())/nNotes;
	}//endfor k

	return Mean(matchRates);
}//end AveragePairwiseMatchRate

inline double AveragePairwiseBigramMatchRate(vector<PianoFingering> finsGT,PianoFingering finEst){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();
	int nNotes=finEst.evts.size();

	vector<double> matchRates(nGT);

	vector<vector<int> > handParts(2);//[0]=RH, [1]=LH
	for(int n=0;n<nNotes;n+=1){
		int fn=atoi(finEst.evts[n].fingerNum.c_str());
		if(fn>0){
			handParts[0].push_back(n);
		}else{
			handParts[1].push_back(n);
		}//endif
	}//endfor n

	for(int k=0;k<finsGT.size();k+=1){
		double count=0;
		double match=0;
		for(int h=0;h<2;h+=1){
			for(int n=1;n<handParts[h].size();n+=1){
				count+=1;
				if(finEst.evts[handParts[h][n]].fingerNum == finsGT[k].evts[handParts[h][n]].fingerNum
				   && finEst.evts[handParts[h][n-1]].fingerNum == finsGT[k].evts[handParts[h][n-1]].fingerNum){
					match+=1;
				}//endif
			}//endfor n
		}//endfor h

		matchRates[k]=match/count;
	}//endfor k

	return Mean(matchRates);
}//end AveragePairwiseBigramMatchRate


inline double HighestPairwiseBigramMatchRate(vector<PianoFingering> finsGT,PianoFingering finEst){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();
	int nNotes=finEst.evts.size();

	vector<double> matchRates(nGT);

	vector<vector<int> > handParts(2);//[0]=RH, [1]=LH
	for(int n=0;n<nNotes;n+=1){
		int fn=atoi(finEst.evts[n].fingerNum.c_str());
		if(fn>0){
			handParts[0].push_back(n);
		}else{
			handParts[1].push_back(n);
		}//endif
	}//endfor n

	for(int k=0;k<finsGT.size();k+=1){
		double count=0;
		double match=0;
		for(int h=0;h<2;h+=1){
			for(int n=1;n<handParts[h].size();n+=1){
				count+=1;
				if(finEst.evts[handParts[h][n]].fingerNum == finsGT[k].evts[handParts[h][n]].fingerNum
				   && finEst.evts[handParts[h][n-1]].fingerNum == finsGT[k].evts[handParts[h][n-1]].fingerNum){
					match+=1;
				}//endif
			}//endfor n
		}//endfor h

		matchRates[k]=match/count;
	}//endfor k

	int amax=0;
	for(int k=0;k<matchRates.size();k+=1){
		if(matchRates[k]>matchRates[amax]){amax=k;}
	}//endfor k

	return matchRates[amax];
}//end HighestPairwiseBigramMatchRate


inline double MultiGTLooseBigramMatchRate(vector<PianoFingering> finsGT,PianoFingering finEst){
	assert(finsGT.size()>0);
	for(int k=1;k<finsGT.size();k+=1){
		assert(finsGT[k].evts.size()==finsGT[0].evts.size());
	}//endfor k
	assert(finEst.evts.size()==finsGT[0].evts.size());

	int nGT=finsGT.size();
	int nNotes=finEst.evts.size();

	vector<double> matchRates(nGT);

	vector<vector<int> > handParts(2);//[0]=RH, [1]=LH
	for(int n=0;n<nNotes;n+=1){
		int fn=atoi(finEst.evts[n].fingerNum.c_str());
		if(fn>0){
			handParts[0].push_back(n);
		}else{
			handParts[1].push_back(n);
		}//endif
	}//endfor n

	double count=0;
	double match=0;
	for(int h=0;h<2;h+=1){
		for(int n=1;n<handParts[h].size();n+=1){
			count+=1;
			bool matched=false;
			for(int k=0;k<finsGT.size();k+=1){
				if(finEst.evts[handParts[h][n]].fingerNum == finsGT[k].evts[handParts[h][n]].fingerNum
				   && finEst.evts[handParts[h][n-1]].fingerNum == finsGT[k].evts[handParts[h][n-1]].fingerNum){
					matched=true;
					break;
				}//endif
			}//endfor k
			if(matched){match+=1;}
		}//endfor n
	}//endfor h

	return match/count;
}//end MultiGTLooseBigramMatchRate






#endif // FingeringEvaluation_HPP
