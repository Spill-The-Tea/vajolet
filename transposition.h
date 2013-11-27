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
#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "vajolet.h"
#include <cassert>
#include <stdlib.h>
#include <cstring>

enum ttType{
	typeExact,
	typeAlpha,
	typeBeta
};

struct ttEntry{
private :
	unsigned int key; 			/*! 32 bit for the upper part of the key*/
	Score value;				/*! 32 bit for the value*/
	Score staticValue;			/*! 32 bit for the static evalutation (eval())*/
	unsigned short packedMove;	/*!	16 bit for the move*/
	signed short int depth;		/*! 16 bit for depth*/
	unsigned char searchId;		/*! 8 bit for the generation id*/
	unsigned char type;			/*! 8 bit for the type of the entry*/
								/*  144 bits total =18 bytes*/
public:
	void save(unsigned int k,Score val,unsigned char t,signed short int d,unsigned short m,Score sv){
		key=k;
		value=val;
		staticValue=sv;
		packedMove= m;
		depth=d;
		type=t;
	}
	void setGeneration(unsigned char gen){
		searchId=gen;
	}

	unsigned int getKey() const{return key;}
	Score getValue()const {return value;}
	Score getStaticValue()const {return staticValue;}
	unsigned short getPackedMove()const {return packedMove;}
	signed short int getDepth()const {return depth;}
	unsigned char getSearchId()const {return searchId;}
	unsigned char getType()const {return type;}
	unsigned char getGeneration()const {return searchId;}


};

struct ttCluster{
	ttEntry data[4];
};


class transpositionTable{
	ttCluster* table;
	unsigned int elements;
	unsigned char generation;
public:
	~transpositionTable(){
		if(table){
			free(table);
		}
	}
	void newSearch() { generation++; }
	void setSize(unsigned int mbSize);
	void clear();

	inline ttCluster* findCluster(U64 key) const {
		return table + (key % elements);
	}

	inline void refresh(const ttEntry* tte) const {
		const_cast<ttEntry*>(tte)->setGeneration(generation);
	}

	ttEntry* probe(const U64 key) const;
	void store(const U64 key, Score v, unsigned char b, signed short int d, unsigned short m, Score statV);
};




#endif /* TRANSPOSITION_H_ */
