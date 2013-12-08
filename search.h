/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/
#ifndef SEARCH_H_
#define SEARCH_H_

#include "vajolet.h"
#include "position.h"
#include "move.h"
#include <vector>
#include <list>
#include <cmath>

class searchLimits{
public:
	bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,depth,nodes,mate,moveTime;

	std::list<Move> searchMoves;
	searchLimits(){
		ponder=false;
		infinite =false;
		wtime=0;
		btime=0;
		winc=0;
		binc=0;
		movesToGo=0;
		depth=0;
		nodes=0;
		mate=0;
		moveTime=0;
	}

};



inline Score mateIn(int ply) {
  return SCORE_MATE - ply;
}

inline Score matedIn(int ply) {
  return SCORE_MATED + ply;
}


class search{

	static Score futility[5];
	static Score futilityMargin[7];
	static Score FutilityMoveCounts[11];
	static Score PVreduction[32*ONE_PLY][64];
	static Score nonPVreduction[32*ONE_PLY][64];

	searchLimits limits;

public:
	static void initLMRreduction(void){
		for (int d = 1; d < 32*ONE_PLY; d++)
			for (int mc = 1; mc < 64; mc++)
			{
				double    PVRed = 0.08*log(double(d)) * log(double(mc));
				double nonPVRed = 0.33 + 0.213*log(double(d)) * log(double(mc));
				PVreduction[d][mc]=(Score)(PVRed >= 1.0 ? floor(PVRed * int(ONE_PLY)) : 0);
				nonPVreduction[d][mc]=(Score)(nonPVRed >= 1.0 ? floor(nonPVRed * int(ONE_PLY)) : 0);
			}
	};
	struct sSignal{
		bool stop=false;
	}signals;

	typedef enum eNodeType{
		ROOT_NODE,
		PV_NODE,
		ALL_NODE,
		CUT_NODE
	} nodeType;
	void startThinking(Position & p,searchLimits & limits);
	unsigned long long getVisitedNodes(){
		return visitedNodes;
	}
private:
	template<nodeType type>Score alphaBeta(unsigned int ply,Position & p,int depth,Score alpha,Score beta,std::vector<Move> & PV);
	template<nodeType type>Score qsearch(unsigned int ply,Position & p,int depth,Score alpha,Score beta,std::vector<Move> & PV);
	unsigned long long visitedNodes;
	unsigned int selDepth;
	bool stop;
};

#endif /* SEARCH_H_ */
