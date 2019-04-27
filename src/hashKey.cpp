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

//---------------------------------
//	includes
//---------------------------------
#include <random>

#include "hashKey.h"

//---------------------------------
//	global static hashKeys
//---------------------------------

tKey  HashKey::_keys[squareNumber][30];  	// position, piece (not all the keys are used)
tKey  HashKey::_side;          				// side to move (black)
tKey  HashKey::_ep[squareNumber];        	// ep targets (only 16 used)
tKey  HashKey::_castlingRight[16];			// white king-side castling right
tKey  HashKey::_exclusion;					// position with an exluded move

/*!	\brief init the hashkeys
    \author Marco Belli
	\version 1.0
	\date 27/10/2013
 */
void HashKey::init()
{
	// initialize all random 64-bit numbers
	int i,j;
	tKey temp[4];
	std::mt19937_64 rnd;
	std::uniform_int_distribution<uint64_t> uint_dist;

	// use current time (in seconds) as random seed:
	rnd.seed(19091979);

	for (auto & val :_ep){
		val = uint_dist(rnd);
	}

	for(auto & outerArray :_keys)
	{
		for(auto & val :outerArray)
		{
			val= uint_dist(rnd);
		}

	}

	_side = uint_dist(rnd);
	_exclusion=uint_dist(rnd);

	for(auto & val :_castlingRight ){
		val = 0;
	}


	for(auto & val :temp){
		val=uint_dist(rnd);
	}

	for(i=0;i<16;i++){
		for(j=0;j<4;j++){
			if(i&(1<<j)){
				_castlingRight[i]^=temp[j];
			}
		}
	}

}




