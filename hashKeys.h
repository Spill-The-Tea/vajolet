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
#ifndef HASHKEYS_H_
#define HASHKEYS_H_


//---------------------------------
//	includes
//---------------------------------
#include <cstdint>
#include "tSquare.h"

//---------------------------------
//	structure
//---------------------------------
struct HashKeys
{
	static uint64_t keys[squareNumber][30];	// position, piece (not all the keys are used)
	static uint64_t side;					// side to move (black)
	static uint64_t ep[squareNumber];		// ep targets (only 16 used)
	static uint64_t castlingRight[16];		// white king-side castling right
	static uint64_t exclusion;


	static void init();       // initialize the random data
};



#endif /* HASHKEYS_H_ */
