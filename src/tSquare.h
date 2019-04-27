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
#ifndef TSQUARE_H_
#define TSQUARE_H_

#include "eCastle.h"

enum tSquare: int								/*!< square name and directions*/
{
	A1,	B1,	C1,	D1,	E1,	F1,	G1,	H1,
	A2,	B2,	C2,	D2,	E2,	F2,	G2,	H2,
	A3,	B3,	C3,	D3,	E3,	F3,	G3,	H3,
	A4,	B4,	C4,	D4,	E4,	F4,	G4,	H4,
	A5,	B5,	C5,	D5,	E5,	F5,	G5,	H5,
	A6,	B6,	C6,	D6,	E6,	F6,	G6,	H6,
	A7,	B7,	C7,	D7,	E7,	F7,	G7,	H7,
	A8,	B8,	C8,	D8,	E8,	F8,	G8,	H8,
	squareNone,
	squareNumber=64,
	north=8,
	sud=-8,
	est=1,
	ovest=-1,
	square0=0
};

inline tSquare operator+(const tSquare d1, const tSquare d2) { return static_cast<tSquare>(static_cast<int>(d1) + static_cast<int>(d2)); }
inline tSquare operator-(const tSquare d1, const tSquare d2) { return static_cast<tSquare>(static_cast<int>(d1) - static_cast<int>(d2)); }
inline tSquare operator*(int i, const tSquare d) { return static_cast<tSquare>(i * static_cast<int>(d)); }
inline tSquare operator*(const tSquare d, int i) { return static_cast<tSquare>(static_cast<int>(d) * i); }
inline tSquare operator-(const tSquare d) { return static_cast<tSquare>(-static_cast<int>(d)); }
inline tSquare& operator+=(tSquare& d1, const tSquare d2) { d1 = d1 + d2; return d1; }
inline tSquare& operator-=(tSquare& d1, const tSquare d2) { d1 = d1 - d2; return d1; }
inline tSquare& operator*=(tSquare& d, int i) { d = static_cast<tSquare>(static_cast<int>(d) * i); return d; }

inline tSquare& operator++(tSquare& d) { d = static_cast<tSquare>(static_cast<int>(d) + 1); return d; }
inline tSquare& operator--(tSquare& d) { d = static_cast<tSquare>(static_cast<int>(d) - 1); return d; }
inline tSquare operator/(const tSquare d, int i) { return static_cast<tSquare>(static_cast<int>(d) / i); }
inline tSquare& operator/=(tSquare& d, int i) { d = static_cast<tSquare>(static_cast<int>(d) / i); return d; }

enum tRank: int								
{
	RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8, rankNumber
};

inline tRank& operator++(tRank& d) { d = static_cast<tRank>(static_cast<int>(d) + 1); return d; }
inline tRank& operator--(tRank& d) { d = static_cast<tRank>(static_cast<int>(d) - 1); return d; }
inline tRank operator+(const tRank d1, const int d2) { return static_cast<tRank>(static_cast<int>(d1) + d2); }
inline tRank operator-(const tRank d1, const int d2) { return static_cast<tRank>(static_cast<int>(d1) - d2); }

enum tFile: int								
{
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH, fileNumber
};

inline tFile& operator++(tFile& d) { d = static_cast<tFile>(static_cast<int>(d) + 1); return d; }
inline tFile& operator--(tFile& d) { d = static_cast<tFile>(static_cast<int>(d) - 1); return d; }
inline tFile operator+(const tFile d1, const int d2) { return static_cast<tFile>(static_cast<int>(d1) + d2); }
inline tFile operator-(const tFile d1, const int d2) { return static_cast<tFile>(static_cast<int>(d1) - d2); }


inline tRank getRankOf( const tSquare s )
{
	extern const volatile tRank RANKS[squareNumber];
	return RANKS[s];
}

inline tRank getRelativeRankOf( const tSquare s, const Color c )
{
	extern const volatile tRank RANKS[squareNumber];
	return c ? tRank(7 - RANKS[s]) : RANKS[s];
}

inline tFile getFileOf( const tSquare s )
{
	extern const volatile tFile FILES[squareNumber];
	return FILES[s];
}

inline bool isLateralFile( const tFile f )
{
	return f == FILEA || f == FILEH;
}

inline tSquare getSquare( const tFile f, const tRank r)
{
	return static_cast<tSquare>(f + 8 * r);
}

inline Color getSquareColor( const tSquare sq )
{
	extern Color SQUARE_COLOR[squareNumber];
	return SQUARE_COLOR[sq];
}

inline tSquare getPromotionSquareOf( tSquare sq, Color c)
{
	return getSquare( getFileOf( sq ), ( c ? RANK1 : RANK8 ) );
} 


#endif
