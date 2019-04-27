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
#include <sstream>

#include "command.h"
#include "vajo_io.h"
#include "parameters.h"
#include "position.h"
#include "vajolet.h"

/* PST data */
const int Center[8]	= { -3, -1, +0, +1, +1, +0, -1, -3};
const int KFile[8]	= { +3, +4, +2, +0, +0, +2, +4, +3};
const int KRank[8]	= { +1, +0, -2, -3, -4, -5, -6, -7};

simdScore Position::pieceValue[lastBitboard];
simdScore Position::pstValue[lastBitboard][squareNumber];
simdScore Position::nonPawnValue[lastBitboard];
eCastle Position::castleRightsMask[squareNumber];
std::array<bitMap,9> Position::_castlePath;

void Position::initPstValues(void)
{
	for(bitboardIndex piece = occupiedSquares; piece < lastBitboard; ++piece)
	{
		for(tSquare s = A1; s < squareNumber; ++s)
		{
			assert(s<squareNumber);
			nonPawnValue[piece] = simdScore{0,0,0,0};
			pstValue[piece][s] = simdScore{0,0,0,0};
			tRank rank = getRankOf(s);
			tFile file = getFileOf(s);

			if(piece > occupiedSquares && piece < whitePieces )
			{

				if( isPawn( piece ) )
				{
					pstValue[piece][s] = simdScore{0,0,0,0};
					if(s==D3)
					{
						pstValue[piece][s] = PawnD3;
					}
					if(s==D4)
					{
						pstValue[piece][s] = PawnD4;
					}
					if(s==D5)
					{
						pstValue[piece][s] = PawnD5;
					}
					if(s==E3)
					{
						pstValue[piece][s] = PawnE3;
					}
					if(s==E4)
					{
						pstValue[piece][s] = PawnE4;
					}
					if(s==E5)
					{
						pstValue[piece][s] = PawnE5;
					}
					pstValue[piece][s] += PawnRankBonus * static_cast<int>(rank - 2);
					pstValue[piece][s] += Center[file] * PawnCentering;
				}
				if( isKnight( piece ) )
				{
					pstValue[piece][s] = KnightPST * (Center[file] + Center[rank]);
					if(rank==RANK1)
					{
						pstValue[piece][s] -= KnightBackRankOpening;
					}
				}
				if( isBishop( piece ) )
				{
					pstValue[piece][s] = BishopPST * (Center[file] + Center[rank]);
					if(rank==RANK1)
					{
						pstValue[piece][s] -= BishopBackRankOpening;
					}
					if(((int)file==(int)rank) || (file+rank==7))
					{
						pstValue[piece][s] += BishopOnBigDiagonals;
					}
				}
				if( isRook( piece ) )
				{
					pstValue[piece][s] = RookPST * (Center[file]);
					if(rank==RANK1)
					{
						pstValue[piece][s] -= RookBackRankOpening;
					}
				}
				if( isQueen( piece) )
				{
					pstValue[piece][s] = QueenPST * (Center[file] + Center[rank]);
					if(rank==RANK1)
					{
						pstValue[piece][s] -= QueenBackRankOpening;
					}
				}
				if( isKing( piece ) )
				{
					pstValue[piece][s] = simdScore{
							(KFile[file]+KRank[rank]) * KingPST[0],
							(Center[file]+Center[rank]) * KingPST[1],
							0,0};
				}
				if(!isKing( piece ) )
				{
					pstValue[piece][s] += pieceValue[piece];
				}

				if( !isPawn( piece ) && !isKing( piece ) )
				{
					nonPawnValue[piece][0] = pieceValue[piece][0];
					nonPawnValue[piece][1] = pieceValue[piece][1];
				}

			}
			else if( isBlackPiece( piece ) && piece <blackPieces )
			{
				tRank r = getRelativeRankOf( s, black );
				tFile f = file;
				pstValue[piece][s] = -pstValue[ piece - separationBitmap ][getSquare(f,r)];

				if( !isPawn( piece ) && !isKing( piece ) )
				{
					nonPawnValue[piece][2] = pieceValue[piece][0];
					nonPawnValue[piece][3] = pieceValue[piece][1];
				}
			}
			else{
				pstValue[piece][s] = simdScore{0,0,0,0};
			}
		}
	}

}


/*! \brief setup a position from a fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
const Position& Position::setupFromFen(const std::string& fenStr)
{
	char col,row,token;
	tSquare sq = A8;
	std::istringstream ss(fenStr);

	clear();
	ss >> std::noskipws;

	while ((ss >> token) && !std::isspace(token))
	{
		if (isdigit(token))
			sq += tSquare(token - '0'); // Advance the given number of files
		else if (token == '/')
			sq -= tSquare(16);
		else
		{
			switch (token)
			{
			case 'P':
				putPiece(whitePawns,sq);
				break;
			case 'N':
				putPiece(whiteKnights,sq);
				break;
			case 'B':
				putPiece(whiteBishops,sq);
				break;
			case 'R':
				putPiece(whiteRooks,sq);
				break;
			case 'Q':
				putPiece(whiteQueens,sq);
				break;
			case 'K':
				putPiece(whiteKing,sq);
				break;
			case 'p':
				putPiece(blackPawns,sq);
				break;
			case 'n':
				putPiece(blackKnights,sq);
				break;
			case 'b':
				putPiece(blackBishops,sq);
				break;
			case 'r':
				putPiece(blackRooks,sq);
				break;
			case 'q':
				putPiece(blackQueens,sq);
				break;
			case 'k':
				putPiece(blackKing,sq);
				break;
			}
			++sq;
		}
	}

	state &x= getActualState();


	ss >> token;
	x.setNextTurn( token == 'w' ? whiteTurn : blackTurn );

	updateUsThem();

	ss >> token;

	x.clearCastleRight();
	while ((ss >> token) && !isspace(token))
	{
		switch(token){
		case 'K':
			x.setCastleRight( wCastleOO );
			break;
		case 'Q':
			x.setCastleRight( wCastleOOO );
			break;
		case 'k':
			x.setCastleRight( bCastleOO );
			break;
		case 'q':
			x.setCastleRight( bCastleOOO );
			break;
		}
	}

	x.resetEpSquare();
	if (((ss >> col) && (col >= 'a' && col <= 'h'))
		&& ((ss >> row) && (row == '3' || row == '6')))
	{
		x.setEpSquare( (tSquare) ( ( (int) col - 'a') + 8 * (row - '1') ) );
		if (!( getAttackersTo( x.getEpSquare() ) & bitBoard[ x.getPawnsOfActivePlayer() ] ) )
		{
			x.resetEpSquare();
		}
	}


	unsigned int _50moveCount;
	ss >> std::skipws >> _50moveCount;
	x.setIrreversibleMoveCount( _50moveCount );
	if(ss.eof()){
		_ply = int( x.isBlackTurn() );
		x.resetIrreversibleMoveCount();

	}else{
		ss>> _ply;
		_ply = std::max(2 * (_ply - 1), (unsigned int)0) + int( x.isBlackTurn() );
	}

	x.resetPliesFromNullCount();
	x.setCurrentMove( Move::NOMOVE );
	x.resetCapturedPiece();

	x.setMaterialValue( calcMaterialValue() );
	x.setNonPawnValue( calcNonPawnMaterialValue() );

	x.setKey( calcKey() );
	x.setPawnKey( calcPawnKey() );
	x.setMaterialKey( calcMaterialKey() );

	calcCheckingSquares();

	x.setHiddenCheckers( getHiddenCheckers<true>() );
	x.setPinnedPieces( getHiddenCheckers<false>() );
	x.setCheckers( getAttackersTo( getSquareOfOurKing() ) & bitBoard[x.getPiecesOfOtherPlayer()] );

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif
	return *this;
}

const Position& Position::setup(const std::string& code, const Color c)
{

  assert(code.length() > 0 && code.length() < 8);
  assert(code[0] == 'K');

  std::string sides[] = { code.substr(code.find('K', 1)), // Weak
                     code.substr(0, code.find('K', 1)) }; // Strong

  std::transform(sides[c].begin(), sides[c].end(), sides[c].begin(), tolower);

  std::string fenStr =  sides[0] + char(8 - sides[0].length() + '0') + "/8/8/8/8/8/8/"
                 + sides[1] + char(8 - sides[1].length() + '0') + " w - - 0 10";

  return setupFromFen(fenStr);
}




/*! \brief init the score value in the static const
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initScoreValues(void)
{
	for(auto &val: pieceValue)
	{
		val = simdScore{0,0,0,0};
	}
	pieceValue[whitePawns] = initialPieceValue[whitePawns];
	pieceValue[whiteKnights] = initialPieceValue[whiteKnights];
	pieceValue[whiteBishops] = initialPieceValue[whiteBishops];
	pieceValue[whiteRooks] = initialPieceValue[whiteRooks];
	pieceValue[whiteQueens] = initialPieceValue[whiteQueens];
	pieceValue[whiteKing] = initialPieceValue[whiteKing];

	pieceValue[blackPawns] = pieceValue[whitePawns];
	pieceValue[blackKnights] = pieceValue[whiteKnights];
	pieceValue[blackBishops] = pieceValue[whiteBishops];
	pieceValue[blackRooks] = pieceValue[whiteRooks];
	pieceValue[blackQueens] = pieceValue[whiteQueens];
	pieceValue[blackKing] = pieceValue[whiteKing];

	initPstValues();
}
/*! \brief clear a position and his history
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::clear()
{
	for (tSquare sq = square0; sq < squareNumber; ++sq)
	{
		squares[sq] = empty;
	}
	for (bitboardIndex i = occupiedSquares; i < lastBitboard; ++i)
	{
		bitBoard[i] = 0;
	}
	stateInfo.clear();
	stateInfo.emplace_back(state());

}


/*	\brief display a board for debug purpose
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
void Position::display()const
{
	sync_cout<<getFen()<<sync_endl;

	const state& st = getActualState();
	sync_cout;
	{
		for (tRank rank = RANK8; rank >= RANK1; --rank)
		{
			std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
			std::cout << rank+1 <<  " |";
			for (tFile file = FILEA; file <= FILEH; ++file)
			{
				std::cout << " " << UciManager::getPieceName(getPieceAt(getSquare(file,rank))) << " |";
			}
			std::cout << std::endl;
		}
		std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
		std::cout << "    a   b   c   d   e   f   g   h" << std::endl << std::endl;

	}
	std::cout <<(st.isBlackTurn() ? "BLACK TO MOVE" : "WHITE TO MOVE" ) <<std::endl;
	std::cout <<"50 move counter "<<st.getIrreversibleMoveCount()<<std::endl;
	std::cout <<"castleRights ";
	if( st.hasCastleRight(wCastleOO) ) std::cout<<"K";
	if( st.hasCastleRight(wCastleOOO) ) std::cout<<"Q";
	if( st.hasCastleRight(bCastleOO) ) std::cout<<"k";
	if( st.hasCastleRight(bCastleOOO) ) std::cout<<"q";
	if( !st.hasCastleRights() ) std::cout<<"-";
	std::cout<<std::endl;
	std::cout <<"epsquare ";

	std::cout<< _printEpSquare( st ) <<std::endl;

	std::cout<<"material "<<st.getMaterialValue()[0]/10000.0<<std::endl;
	std::cout<<"white material "<<st.getNonPawnValue()[0]/10000.0<<std::endl;
	std::cout<<"black material "<<st.getNonPawnValue()[2]/10000.0<<std::endl;

	std::cout<<sync_endl;

}


/*	\brief display the fen string of the position
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
std::string  Position::getFen() const {

	std::string s;
	int emptyFiles = 0;
	const state& st = getActualState();
	for ( tRank rank = RANK8; rank >= RANK1; --rank)
	{
		emptyFiles = 0;
		for ( tFile file = FILEA; file <= FILEH; ++file)
		{
			if(getPieceAt(getSquare(file,rank)) != empty)
			{
				if(emptyFiles!=0)
				{
					s+=std::to_string(emptyFiles);
				}
				emptyFiles=0;
				s += UciManager::getPieceName( getPieceAt(getSquare(file,rank)) );
			}
			else
			{
				emptyFiles++;
			}
		}
		if(emptyFiles!=0)
		{
			s += std::to_string(emptyFiles);
		}
		if(rank != 0)
		{
			s += '/';
		}
	}
	s += ' ';
	if(st.isBlackTurn() )
	{
		s += 'b';
	}
	else
	{
		s += 'w';
	}
	s += ' ';

	if( st.hasCastleRight(wCastleOO) )
	{
		s += "K";
	}
	if( st.hasCastleRight(wCastleOOO) )
	{
		s += "Q";
	}
	if( st.hasCastleRight(bCastleOO) )
	{
		s += "k";
	}
	if( st.hasCastleRight(bCastleOOO) )
	{
		s += "q";
	}
	if( !st.hasCastleRights() ){
		s += "-";
	}
	s += ' ';
	s += _printEpSquare( st );
	s += ' ';
	s += std::to_string(st.getIrreversibleMoveCount());
	s += " " + std::to_string(1 + ( _ply - int( st.isBlackTurn() ) ) / 2);


	return s;
}

#ifdef DEBUG_EVAL_SIMMETRY
std::string Position::getSymmetricFen() const {

	std::string s;
	int emptyFiles=0;
	const state& st =getActualState();
	for (tRank rank = RANK1; rank <=RANK8 ; rank++)
	{
		emptyFiles=0;
		for (tFile file = FILEA; file <=FILEH; file++)
		{
			if(getPieceAt(getSquare(file,rank))!=empty)
			{
				if(emptyFiles!=0)
				{
					s += std::to_string(emptyFiles);
				}
				emptyFiles = 0;
				bitboardIndex xx = getPieceAt(getSquare(file,rank));
				if( isBlackPiece(xx) )
				{
					xx = (bitboardIndex)(xx - separationBitmap);
				}
				else
				{
					xx = (bitboardIndex)(xx + separationBitmap);
				}
				s += UciManager::getPieceName( xx );
			}
			else
			{
				emptyFiles++;
			}
		}
		if(emptyFiles!=0){
			s += std::to_string(emptyFiles);
		}
		if(rank!=7)
		{
			s += '/';
		}
	}
	s += ' ';
	if( st.isBlackTurn() )
	{
		s += 'w';
	}
	else
	{
		s += 'b';
	}
	s += ' ';

	if( st.hasCastleRight(wCastleOO) )
	{
		s += "k";
	}
	if( st.hasCastleRight(wCastleOOO) )
	{
		s += "q";
	}
	if( st.hasCastleRight(bCastleOO) )
	{
		s += "K";
	}
	if( st.hasCastleRight(bCastleOOO) )
	{
		s += "Q";
	}
	if( !st.hasCastleRights() )
	{
		s += "-";
	}
	s += ' ';
	s += _printEpSquare( st );
	s += ' ';
	s += std::to_string(st.getIrreversibleMoveCount());
	s += " " + std::to_string(1 + ( _ply - int( st.isBlackTurn() ) ) / 2);

	return s;
}
#endif

/*! \brief calc the hash key of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
HashKey Position::calcKey(void) const
{
	HashKey hash(0);
	const state& st =getActualState();

	for( tSquare sq = A1; sq < squareNumber; ++sq)
	{
		if( bitboardIndex p = getPieceAt( sq ); p != empty )
		{
			hash.updatePiece( sq, p );
		}
	}

	if( st.isBlackTurn() )
	{
		hash.changeSide();
	}
	hash.setCastlingRight( st.getCastleRights() );


	if( st.hasEpSquare() )
	{
		hash.changeEp( st.getEpSquare() );
	}

	return hash;
}

/*! \brief calc the hash key of the pawn formation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
HashKey Position::calcPawnKey(void) const
{
	HashKey hash(1);
	bitMap b= getBitmap(whitePawns);
	while(b)
	{
		tSquare n = iterateBit(b);
		hash.updatePiece( n, whitePawns );
	}
	b= getBitmap(blackPawns);
	while(b)
	{
		tSquare n = iterateBit(b);
		hash.updatePiece( n, blackPawns );
	}

	return hash;
}

/*! \brief calc the hash key of the material signature
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
HashKey Position::calcMaterialKey(void) const
{
	HashKey hash(0);
	for ( bitboardIndex i = whiteKing; i < lastBitboard; i++)
	{
		if ( isValidPiece( i ) )
		{
			for (unsigned int cnt = 0; cnt < getPieceCount( i ); cnt++)
			{
				// todo invertire? hash.add( cnt, i );
				hash.updatePiece( (tSquare)i, (bitboardIndex)cnt );
			}
		}
	}

	return hash;
}

/*! \brief calc the material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
simdScore Position::calcMaterialValue(void) const{
	simdScore score = {0,0,0,0};
	bitMap b = getOccupationBitmap();
	while(b)
	{
		tSquare s = iterateBit(b);
		bitboardIndex val = getPieceAt(s);
		score += pstValue[val][s];
	}
	return score;

}
/*! \brief calc the non pawn material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
simdScore Position::calcNonPawnMaterialValue() const
{

	simdScore t[2] ={{0,0,0,0},{0,0,0,0}};
	simdScore res;

	bitMap b = getOccupationBitmap();
	while(b)
	{
		tSquare n = iterateBit(b);
		bitboardIndex val = getPieceAt(n);
		if(!isPawn(val) && !isKing(val) )
		{
			t[ isBlackPiece( val ) ] += pieceValue[val];
		}
	}
	res = simdScore{t[0][0],t[0][1],t[1][0],t[1][1]};
	return res;

}
/*! \brief do a null move
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::doNullMove()
{

	insertState(getActualState());
	state &x = getActualState();

	x.setCurrentMove( Move::NOMOVE );
	if( x.hasEpSquare() )
	{
		x.getKey().changeEp( x.getEpSquare() );
		x.resetEpSquare();
	}
	x.getKey().changeSide();
	x.incrementIrreversibleMoveCount();
	x.resetPliesFromNullCount();
	x.changeNextTurn();


	++_ply;
	x.resetCapturedPiece();

	std::swap(Us,Them);


	calcCheckingSquares();
	x.setHiddenCheckers( getHiddenCheckers<true>() );
	x.setPinnedPieces( getHiddenCheckers<false>() );

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}
/*! \brief do a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::doMove(const Move & m)
{
#ifdef	ENABLE_CHECK_CONSISTENCY
	if( ! isMoveLegal(m) )
	{
		std::cerr<<"illegal move "<<UciManager::displayUci(m)<<std::endl;
		exit(-1);
	}
#endif
	assert( m != Move::NOMOVE );

	const bool moveIsCheck = moveGivesCheck(m);

	insertState(getActualState());
	state &x = getActualState();

	x.setCurrentMove( m );

	const tSquare from = m.getFrom();
	const tSquare to = m.getTo();
	const bitboardIndex piece = getPieceAt(from);
	assert( isValidPiece( piece ));

	bitboardIndex captured = ( m.isEnPassantMove() ? (x.isBlackTurn() ? whitePawns : blackPawns ) : getPieceAt(to) );
	assert( isValidPiece( captured ) || captured == empty );

	// change side
	x.getKey().changeSide();
	++_ply;

	// update counter
	x.incrementIrreversibleMoveCount();
	x.incrementPliesFromNullCount();

	// reset ep square
	if( x.hasEpSquare() )
	{
		x.getKey().changeEp( x.getEpSquare() );
		x.resetEpSquare();
	}

	// do castle additional instruction
	if( m.isCastleMove() )
	{
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		assert(rFrom<squareNumber);
		bitboardIndex rook = getPieceAt(rFrom);
		assert( isRook(rook) );
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rTo<squareNumber);
		movePiece(rook,rFrom,rTo);
		x.addMaterial( pstValue[rook][rTo] - pstValue[rook][rFrom] );

		x.getKey().updatePiece( rFrom, rook );
		x.getKey().updatePiece( rTo, rook );

	}
	else if( captured ) // do capture
	{
		tSquare captureSquare = to;
		if(isPawn(captured))
		{

			if( m.isEnPassantMove() )
			{
				captureSquare-=pawnPush( x.isBlackTurn() );
			}
			assert(captureSquare<squareNumber);
			x.getPawnKey().updatePiece( captureSquare, captured );
		}

		// remove piece
		removePiece(captured,captureSquare);
		// update material
		x.removeMaterial( pstValue[captured][captureSquare] );
		x.removeNonPawnMaterial( nonPawnValue[captured] );

		// update keys
		x.getKey().updatePiece( captureSquare, captured);
		assert(getPieceCount(captured)<30);
		x.getMaterialKey().updatePiece( (tSquare)captured, (bitboardIndex)getPieceCount(captured) ); // ->after removing the piece

		// reset fifty move counter
		x.resetIrreversibleMoveCount();
	}

	// update hashKey
	x.getKey().updatePiece( from, piece );
	x.getKey().updatePiece( to, piece );
	movePiece(piece, from, to);

	x.addMaterial( pstValue[piece][to] - pstValue[piece][from] );

	// Update castle rights if needed
	if ( x.hasCastleRights() && (castleRightsMask[from] | castleRightsMask[to]))
	{
		eCastle cr = castleRightsMask[from] | castleRightsMask[to];
		assert((x.getCastleRights() & cr)<16);
		x.getKey().setCastlingRight( x.getCastleRights() & cr );
		x.clearCastleRight( cr );
	}

	if(isPawn(piece))
	{
		// set en-passant
		if(
				abs(from-to)==16	// double push
				&& (getAttackersTo((tSquare)((from+to)>>1))  & Them[Pawns])
		)
		{
			x.setEpSquare( (tSquare)((from+to)>>1) );
			x.getKey().changeEp( x.getEpSquare() );
		}
		else if( m.isPromotionMove() )
		{
			bitboardIndex promotedPiece = getPieceOfPlayer( m.getPromotedPiece(), x.getNextTurn() );
			assert( isValidPiece(promotedPiece) );

			removePiece(piece,to);
			putPiece(promotedPiece,to);

			x.addMaterial( pstValue[promotedPiece][to] - pstValue[piece][to] );
			x.addNonPawnMaterial( nonPawnValue[promotedPiece] );


			x.getKey().updatePiece( to, piece );
			x.getKey().updatePiece( to, promotedPiece );
			x.getPawnKey().updatePiece( to,piece );
			x.getMaterialKey().updatePiece( (tSquare)promotedPiece, (bitboardIndex)( getPieceCount(promotedPiece) - 1 ) );
			x.getMaterialKey().updatePiece( (tSquare)piece, (bitboardIndex)getPieceCount(piece) );
		}
		x.getPawnKey().updatePiece( from, piece );
		x.getPawnKey().updatePiece( to, piece );
		x.resetIrreversibleMoveCount();
	}

	x.setCapturedPiece( captured );
	x.changeNextTurn();

	std::swap(Us,Them);


	x.setCheckers( 0 );
	if(moveIsCheck)
	{

		if( !m.isStandardMove() )
		{
			assert( getSquareOfOurKing() <squareNumber);
			x.addCheckers( getAttackersTo( getSquareOfOurKing() ) & Them[Pieces] );
		}
		else
		{
			if( isSquareSet( x.getCheckingSquares( piece ), to ) ) // should be old state, but checkingSquares has not been changed so far
			{
				x.addCheckers( bitSet(to) );
			}
			if( x.thereAreHiddenCheckers() && (x.isHiddenChecker( from ) ) )	// should be old state, but hiddenCheckersCandidate has not been changed so far
			{
				if(!isRook(piece))
				{
					x.addCheckers( Movegen::attackFrom<whiteRooks>( getSquareOfOurKing(), getOccupationBitmap() ) & (Them[Queens] |Them[Rooks]) );
				}
				if(!isBishop(piece))
				{
					x.addCheckers( Movegen::attackFrom<whiteBishops>( getSquareOfOurKing(), getOccupationBitmap() ) & (Them[Queens] |Them[Bishops]) );
				}
			}
		}
	}

	calcCheckingSquares();
	x.setHiddenCheckers( getHiddenCheckers<true>() );
	x.setPinnedPieces( getHiddenCheckers<false>() );

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}

/*! \brief undo a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::undoMove()
{
	--_ply;

	const state& x = getActualState();
	const Move &m = x.getCurrentMove();
	assert( m );
	const tSquare to = m.getTo();
	const tSquare from = m.getFrom();
	bitboardIndex piece = getPieceAt(to);
	assert( isValidPiece( piece ) );

	if( m.isPromotionMove() ){
		removePiece(piece,to);
		piece = isBlackPiece( piece) ? blackPawns : whitePawns;
		putPiece(piece,to);
	}
	else if( m.isCastleMove() )
	{
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);
		bitboardIndex rook = getPieceAt(rTo);
		assert( isRook(rook) );
		movePiece(rook,rTo,rFrom);

	}

	movePiece(piece,to,from);

	assert( isValidPiece( x.getCapturedPiece() ) || x.getCapturedPiece() == empty );
	if( bitboardIndex p = x.getCapturedPiece() )
	{
		tSquare capSq = to;
		if( m.isEnPassantMove() )
		{
			capSq += pawnPush( x.isBlackTurn() );
		}
		assert( capSq < squareNumber );
		putPiece( p, capSq );
	}
	removeState();

	std::swap(Us,Them);


#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(0);
#endif

}

/*! \brief undo a null move
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::undoNullMove()
{
	--_ply;
	removeState();
	std::swap( Us, Them );

#ifdef ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(0);
#endif
}
/*! \brief init the helper castle mash
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initCastleRightsMask(void)
{
	for(int i=0;i<squareNumber;i++)
	{
		castleRightsMask[i] = eCastle(0);
	}
	castleRightsMask[E1] = wCastleOO | wCastleOOO;
	castleRightsMask[A1] = wCastleOOO;
	castleRightsMask[H1] = wCastleOO;
	castleRightsMask[E8] = bCastleOO | bCastleOOO;
	castleRightsMask[A8] = bCastleOOO;
	castleRightsMask[H8] = bCastleOO;
	
	for( auto& x : _castlePath )
	{
		x = 0;
	}
	_castlePath.at( wCastleOO  ) = bitSet(F1) | bitSet(G1);
	_castlePath.at( wCastleOOO ) = bitSet(D1) | bitSet(C1) | bitSet(B1);
	_castlePath.at( bCastleOO  ) = bitSet(F8) | bitSet(G8);
	_castlePath.at( bCastleOOO ) = bitSet(D8) | bitSet(C8) | bitSet(B8);
}


#ifdef	ENABLE_CHECK_CONSISTENCY
static void block( std::string errorString, unsigned int type )
{
	std::cerr<< errorString <<std::endl;
	std::cerr<<( type ? "DO error" : "undoError" ) <<std::endl;
	exit(-1);
}

/*! \brief do a sanity check on the board
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::checkPosConsistency(int nn) const
{
	const state &x = getActualState();
	if( x.getNextTurn() != whiteTurn && x.getNextTurn() != blackTurn)
	{
		block( "nextMove error", nn );
	}

	// check board
	if(bitBoard[whitePieces] & bitBoard[blackPieces])
	{
		block( "white piece & black piece intersected", nn );
	}
	if((bitBoard[whitePieces] | bitBoard[blackPieces]) !=bitBoard[occupiedSquares])
	{
		display();
		displayBitmap(bitBoard[whitePieces]);
		displayBitmap(bitBoard[blackPieces]);
		displayBitmap(bitBoard[occupiedSquares]);

		block( "all piece problem", nn );
	}
	for(tSquare sq = square0; sq < squareNumber; sq++)
	{
		bitboardIndex id = getPieceAt(sq);

		if( id != empty && !isSquareSet( bitBoard[id], sq ) )
		{
			block( "board inconsistency", nn );
		}
	}

	for (int i = whiteKing; i <= blackPawns; i++)
	{
		for (int j = whiteKing; j <= blackPawns; j++)
		{
			if(i!=j && i!= whitePieces && i!= separationBitmap && j!= whitePieces && j!= separationBitmap && (bitBoard[i] & bitBoard[j]))
			{
				block( "bitboard intersection", nn );
			}
		}
	}
	for (int i = whiteKing; i <= blackPawns; i++)
	{
		if(i!= whitePieces && i!= separationBitmap)
		{
			if(getPieceCount((bitboardIndex)i) != bitCnt(bitBoard[i]))
			{
				block( "pieceCount Error", nn );
			}
		}
	}


	bitMap test=0;
	for (int i = whiteKing; i < whitePieces; i++)
	{
		test |=bitBoard[i];
	}
	if(test!= bitBoard[whitePieces])
	{
		block( "white piece error", nn );
	}
	test=0;
	for (int i = blackKing; i < blackPieces; i++)
	{
		test |=bitBoard[i];
	}
	if(test!= bitBoard[blackPieces])
	{
		block( "black piece error", nn );
	}
	if( x.getKey() != calcKey() )
	{
		display();
		block( "hashKey error", nn );
	}
	if(x.getPawnKey() != calcPawnKey())
	{
		display();
		block( "pawnKey error", nn );
	}
	if(x.getMaterialKey() != calcMaterialKey())
	{
		block( "materialKey error", nn );
	}

	simdScore sc=calcMaterialValue();
	if((sc[0]!=x.getMaterialValue()[0]) || (sc[1]!=x.getMaterialValue()[1]))
	{
		display();
		sync_cout<<sc[0]<<":"<<x.getMaterialValue()[0]<<sync_endl;
		sync_cout<<sc[1]<<":"<<x.getMaterialValue()[1]<<sync_endl;
		block( "material error", nn );
	}
	simdScore score = calcNonPawnMaterialValue();
	if(score[0]!= x.getNonPawnValue()[0] ||
		score[1]!= x.getNonPawnValue()[1] ||
		score[2]!= x.getNonPawnValue()[2] ||
		score[3]!= x.getNonPawnValue()[3]
	){
		display();
		sync_cout<<score[0]<<":"<<x.getNonPawnValue()[0]<<sync_endl;
		sync_cout<<score[1]<<":"<<x.getNonPawnValue()[1]<<sync_endl;
		sync_cout<<score[2]<<":"<<x.getNonPawnValue()[2]<<sync_endl;
		sync_cout<<score[3]<<":"<<x.getNonPawnValue()[3]<<sync_endl;
		block( "non pawn material error", nn );
	}
}
#endif

/*! \brief calculate the checking squares given the king position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline void Position::calcCheckingSquares(void)
{
	state &s = getActualState();
	const eNextMove& attackingPieces = s.getNextTurn();
	tSquare kingSquare = getSquareOfTheirKing();

	const bitMap occupancy = getOccupationBitmap();

	s.resetCheckingSquares( getPieceOfPlayer(King, attackingPieces) );
	assert(kingSquare<squareNumber);
	assert( isValidPiece( getPieceOfPlayer( whitePawns, attackingPieces ) ) );
	s.setCheckingSquares( getPieceOfPlayer( Rooks, attackingPieces ), Movegen::attackFrom<whiteRooks>(kingSquare,occupancy) );
	s.setCheckingSquares( getPieceOfPlayer( Bishops, attackingPieces ), Movegen::attackFrom<whiteBishops>(kingSquare,occupancy) );
	s.setCheckingSquares( getPieceOfPlayer( Queens, attackingPieces ), 
		s.getCheckingSquares( getPieceOfPlayer( Rooks, attackingPieces) ) 
		| s.getCheckingSquares( getPieceOfPlayer( Bishops, attackingPieces) ) );
	s.setCheckingSquares( getPieceOfPlayer( Knights, attackingPieces ), Movegen::attackFrom<whiteKnights>(kingSquare) );

	s.setCheckingSquares( getPieceOfPlayer( Pawns, attackingPieces ), attackingPieces? Movegen::attackFrom<whitePawns>(kingSquare) : Movegen::attackFrom<blackPawns>(kingSquare) );

	assert( getPieceOfOpponent( Pawns, attackingPieces ) >=0 );
	s.resetCheckingSquares( getPieceOfOpponent( King,    attackingPieces ) );
	s.resetCheckingSquares( getPieceOfOpponent( Rooks,   attackingPieces ) );
	s.resetCheckingSquares( getPieceOfOpponent( Bishops, attackingPieces ) );
	s.resetCheckingSquares( getPieceOfOpponent( Queens,  attackingPieces ) );
	s.resetCheckingSquares( getPieceOfOpponent( Knights, attackingPieces ) );
	s.resetCheckingSquares( getPieceOfOpponent( Pawns,   attackingPieces ) );

}

/*! \brief get the hidden checkers/pinners of a position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
template<bool our>
bitMap Position::getHiddenCheckers() const
{
	const state &x  = getActualState();
	const tSquare kingSquare = our? getSquareOfTheirKing(): getSquareOfOurKing();
	const eNextMove next = our? x.getNextTurn(): x.getSwitchedTurn();
	
	assert(kingSquare<squareNumber);
	assert( isValidPiece( getPieceOfPlayer( Pawns, next ) ) );
	bitMap result = 0;
	bitMap pinners = Movegen::getBishopPseudoAttack(kingSquare) &(bitBoard[ getPieceOfPlayer( Bishops, next )]| bitBoard[getPieceOfPlayer( Queens, next) ] );
	pinners |= Movegen::getRookPseudoAttack(kingSquare) &(bitBoard[getPieceOfPlayer( Rooks, next )]| bitBoard[getPieceOfPlayer( Queens, next ) ] );

	while(pinners)
	{
		bitMap b = getSquaresBetween( kingSquare, iterateBit(pinners) ) & getOccupationBitmap();
		if ( !moreThanOneBit(b) )
		{
			result |= b & bitBoard[ x.getPiecesOfActivePlayer() ];
		}
	}
	return result;

}

/*! \brief get all the attackers/defender of a given square with a given occupancy
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bitMap Position::getAttackersTo(const tSquare to, const bitMap occupancy) const
{
	assert(to<squareNumber);
	bitMap res = (Movegen::attackFrom<blackPawns>(to) & bitBoard[whitePawns])
			|(Movegen::attackFrom<whitePawns>(to) & bitBoard[blackPawns])
			|(Movegen::attackFrom<whiteKnights>(to) & (bitBoard[blackKnights]|bitBoard[whiteKnights]))
			|(Movegen::attackFrom<whiteKing>(to) & (bitBoard[blackKing]|bitBoard[whiteKing]));
	bitMap mask = (bitBoard[blackBishops]|bitBoard[whiteBishops]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	res |= Movegen::attackFrom<whiteBishops>(to,occupancy) & mask;
	mask = (bitBoard[blackRooks]|bitBoard[whiteRooks]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	res |=Movegen::attackFrom<whiteRooks>(to,occupancy) & mask;

	return res;
}


/*! \brief tell us if a move gives check before doing the move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bool Position::moveGivesCheck(const Move& m)const
{
	assert( m );
	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	bitboardIndex piece = getPieceAt(from);
	assert( isValidPiece( piece ) );
	const state &s = getActualState();

	// Direct check ?
	if( isSquareSet( s.getCheckingSquares( piece ), to ) )
	{
		return true;
	}

	// Discovery check ?
	if(s.thereAreHiddenCheckers() && s.isHiddenChecker(from) )
	{
		// For pawn and king moves we need to verify also direction
		if ( (!isPawn(piece)&& !isKing(piece)) || !squaresAligned(from, to, getSquareOfTheirKing() ) )
			return true;
	}
	if( m.isStandardMove() )
	{
		return false;
	}

	tSquare kingSquare = getSquareOfTheirKing();
	if( m.isPromotionMove() )
	{
		assert( isValidPiece( getPieceOfPlayer( m.getPromotedPiece(), s.getNextTurn() ) ) );
		if( isSquareSet( s.getCheckingSquares( getPieceOfPlayer( m.getPromotedPiece(), s.getNextTurn() ) ), to ) )
		{
			return true;
		}
		bitMap occ= bitBoard[occupiedSquares] ^ bitSet(from);
		assert( isValidPiece( (bitboardIndex)(whiteQueens + m.getPromotionType() ) ) );
		switch(m.getPromotionType())
		{
			case Move::promQueen:
				return isSquareSet( Movegen::attackFrom<whiteQueens>(to, occ), kingSquare );

			case Move::promRook:
				return isSquareSet( Movegen::attackFrom<whiteRooks>(to, occ), kingSquare );

			case Move::promBishop:
				return isSquareSet( Movegen::attackFrom<whiteBishops>(to, occ), kingSquare );

			case Move::promKnight:
				return isSquareSet( Movegen::attackFrom<whiteKnights>(to, occ), kingSquare );
		}

	}
	else if( m.isCastleMove() )
	{
		tSquare kFrom = from;
		tSquare kTo = to;
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide ? to + est : to + ovest + ovest;
		tSquare rTo = kingSide ? to + ovest : to + est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);

		bitMap occ = (bitBoard[occupiedSquares] ^ bitSet(kFrom) ^ bitSet(rFrom)) | bitSet(rTo) | bitSet(kTo);
		return   isSquareSet( Movegen::getRookPseudoAttack(rTo), kingSquare )
			     && isSquareSet( Movegen::attackFrom<whiteRooks>(rTo,occ), kingSquare );
	}
	else if( m.isEnPassantMove() )
	{
		bitMap captureSquare = fileMask(m.getTo()) & rankMask(m.getFrom());
		bitMap occ = bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(m.getTo())^captureSquare;
		return
				(Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Us[Queens] |Us[Rooks]))
			   | (Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Us[Queens] |Us[Bishops]));

	}

	return false;
}

bool Position::moveGivesDoubleCheck(const Move& m)const
{
	assert( m );
	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	bitboardIndex piece = getPieceAt( from );
	const state &s = getActualState();

	// Direct check ?
	return isSquareSet( s.getCheckingSquares( piece ), to) && ( s.thereAreHiddenCheckers() && s.isHiddenChecker( from ) );
}

bool Position::moveGivesSafeDoubleCheck(const Move& m)const
{
	assert( m );
	tSquare to = m.getTo();
	tSquare kingSquare = getSquareOfTheirKing();
	return !isSquareSet( Movegen::attackFrom<whiteKing>(kingSquare), to ) && moveGivesDoubleCheck(m);
}

bool Position::hasRepeated(bool isPVline) const
{
	const state &s = getActualState();
	unsigned int counter = 1;
	const HashKey& actualkey = s.getKey();
	auto it = stateInfo.rbegin();
	

	int e = std::min( s.getIrreversibleMoveCount(), s.getPliesFromNullCount() );
	if( e >= 4)
	{
		std::advance( it, 2 );
	}
	for(int i = 4 ;	i<=e; i+=2 )
	{
		std::advance( it, 2 );
		if(it->getKey() == actualkey)
		{
			counter++;
			if(!isPVline || counter>=3)
			{
				return true;
			}
		}
	}
	return false;
}

bool Position::isDraw(bool isPVline) const
{

	// Draw by material?

	const state & s = getActualState();
	if (  !bitBoard[whitePawns] && !bitBoard[blackPawns]
		&&( ( (s.getNonPawnValue()[0]<= pieceValue[whiteBishops][0]) && s.getNonPawnValue()[2] == 0)
		|| ( (s.getNonPawnValue()[2]<= pieceValue[whiteBishops][0]) && s.getNonPawnValue()[0] == 0)
		)
	)
	{
		return true;
	}
	// Draw by the 50 moves rule?
	if ( s.getIrreversibleMoveCount() > 99 )
	{
		if(!isInCheck())
		{
			return true;
		}

		if( getNumberOfLegalMoves() )
		{
			return true;
		}
	}

	// Draw by repetition?
	return hasRepeated(isPVline);
}

bool Position::isMoveLegal(const Move &m)const
{

	if( !m )
	{
		return false;
	}

	const state &s = getActualState();
	const bitboardIndex piece = getPieceAt(m.getFrom());
	assert( isValidPiece( piece ) || piece == empty );

	// pezzo inesistente
	if(piece == empty)
	{
		return false;
	}

	// pezzo del colore sbagliato
	if( s.isBlackTurn() ? !isblack(piece) : isblack(piece) )
	{
		return false;
	}

	//casa di destinazione irraggiungibile
	if( isSquareSet( Us[Pieces], m.getTo() ) )
	{
		return false;
	}

	//scacco
	if( s.isInCheck() )
	{
		if( s.isInDoubleCheck() )  //scacco doppio posso solo muovere il re
		{
			if(!isKing(piece))
			{
				return false;
			}
		}
		else // scacco singolo i pezzi che non sono re possono catturare il pezzo attaccante oppure mettersi nel mezzo
		{

			if( !isKing(piece)
				&& !(
					isSquareSet( s.getCheckers(), (tSquare)(m.getTo()-( m.isEnPassantMove() ? pawnPush( isBlackTurn() ) : 0) ) )
					|| isSquareSet( getSquaresBetween( getSquareOfOurKing(), firstOne( s.getCheckers() ) ), m.getTo() )
				)
			)
			{
				return false;
			}
		}
	}
	if( s.isPinned( m.getFrom() ) && !squaresAligned(m.getFrom(), m.getTo(), getSquareOfOurKing() ) )
	{
		return false;
	}


	// promozione impossibile!!
	if(m.isPromotionMove() && ((getRankOf(m.getFrom())!=(s.isBlackTurn() ? RANK2 : RANK7 )) || !(isPawn(piece))))
	{
		return false;
	}

	// mossa mal formata
	if( !m.isPromotionMove() && m.getPromotionType() != Move::promQueen )
	{
		return false;
	}
	//arrocco impossibile
	if( m.isCastleMove() )
	{
		if(!isKing(piece) || (getFileOf(m.getFrom())!=FILEE) || (abs(m.getFrom()-m.getTo())!=2 ) || (getRankOf(m.getFrom())!=RANK1 && getRankOf(m.getFrom())!=RANK8))
		{
			return false;
		}
	}

	//en passant impossibile
	if( m.isEnPassantMove()  && (!isPawn(piece) || !s.isEpSquare( m.getTo() ) ) )
	{
		return false;
	}

	//en passant impossibile
	if( !m.isEnPassantMove()  && isPawn(piece) && s.isEpSquare( m.getTo() ) )
	{
		return false;
	}




	switch(piece)
	{
		case whiteKing:
		case blackKing:
		{
			if( m.isCastleMove() )
			{
				Color color = s.isBlackTurn()? black : white;
				eCastle cs = state::calcCastleRight( m.isKingSideCastle() ? castleOO: castleOOO, color );
				if( !s.hasCastleRight( cs )
					|| !isCastlePathFree( cs )
				)
				{
					return false;
				}
				if(m.getTo()>m.getFrom())
				{
					for(tSquare x=m.getFrom(); x<=m.getTo(); ++x){
						if(getAttackersTo(x,bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces])
						{
							return false;
						}
					}
				}else{
					for(tSquare x=m.getTo(); x<=m.getFrom(); ++x)
					{
						if(getAttackersTo(x,bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces])
						{
							return false;
						}
					}
				}
			}
			else{
				if(! isSquareSet( Movegen::attackFrom<whiteKing>((tSquare)m.getFrom()), m.getTo() ) || isSquareSet( Us[Pieces], m.getTo() ) )
				{
					return false;
				}
				//king moves should not leave king in check
				if((getAttackersTo(m.getTo(),bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces]))
				{
					return false;
				}
			}
		}
			break;

		case whiteRooks:
		case blackRooks:
			assert(m.getFrom()<squareNumber);
			if( !isSquareSet( Movegen::getRookPseudoAttack( m.getFrom() ), m.getTo() ) || !isSquareSet( Movegen::attackFrom<whiteRooks>( m.getFrom(), bitBoard[occupiedSquares] ), m.getTo() ) )
			{
				return false;
			}
			break;

		case whiteQueens:
		case blackQueens:
			assert(m.getFrom()<squareNumber);
			if( !isSquareSet( Movegen::getBishopPseudoAttack( m.getFrom() ) | Movegen::getRookPseudoAttack( m.getFrom() ), m.getTo() )
				||
				!isSquareSet( 
					Movegen::attackFrom<whiteBishops>(m.getFrom(),bitBoard[occupiedSquares]) 
					| Movegen::attackFrom<whiteRooks>(m.getFrom(),bitBoard[occupiedSquares]), m.getTo()
				)
			)
			{
				return false;
			}
			break;

		case whiteBishops:
		case blackBishops:
			if( !isSquareSet(Movegen::getBishopPseudoAttack(m.getFrom()), m.getTo() )
				|| !isSquareSet(Movegen::attackFrom<whiteBishops>(m.getFrom(),bitBoard[occupiedSquares]), m.getTo() )
			)
			{
				return false;
			}
			break;

		case whiteKnights:
		case blackKnights:
			if( !isSquareSet( Movegen::attackFrom<whiteKnights>( m.getFrom() ), m.getTo() ) )
			{
				return false;
			}

			break;

		case whitePawns:

			if(
				// not valid pawn push
				( m.getFrom() + pawnPush(s.isBlackTurn())!= m.getTo() || isSquareSet( bitBoard[occupiedSquares], m.getTo() ) )
				// not valid pawn double push
				&& ((m.getFrom() + 2 * pawnPush(s.isBlackTurn())!= m.getTo()) || (getRankOf(m.getFrom())!=RANK2) || ((bitSet(m.getTo()) | bitSet((tSquare)(m.getTo()-8)))&bitBoard[occupiedSquares]))
				// not valid pawn attack
				&& ( !isSquareSet( Movegen::attackFrom<whitePawns>( m.getFrom() ), m.getTo() ) || !isSquareSet( Them[Pieces] | bitSet( s.getEpSquare() ), m.getTo() ) )
			){
				return false;
			}
			if(getRankOf(m.getFrom())==RANK7 && !m.isPromotionMove())
			{
				return false;

			}
			if( m.isEnPassantMove() )
			{
				auto epSq = s.getEpSquare();
				bitMap captureSquare= fileMask(epSq) & rankMask(m.getFrom());
				bitMap occ= bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(epSq)^captureSquare;
				tSquare kingSquare = getSquareOfOurKing();
				assert(kingSquare<squareNumber);
				if((Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Them[Queens] | Them[Rooks]))|
							(Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Them[Queens] | Them[Bishops])))
				{
				return false;
				}
			}

			break;
		case blackPawns:
			if(
				// not valid pawn push
				( m.getFrom() + pawnPush(s.isBlackTurn())!= m.getTo() || isSquareSet(bitBoard[occupiedSquares], m.getTo() ) )
				// not valid pawn double push
				&& ((m.getFrom() + 2 * pawnPush(s.isBlackTurn())!= m.getTo()) || (getRankOf(m.getFrom())!=RANK7) || ((bitSet(m.getTo()) | bitSet((tSquare)(m.getTo()+8)))&bitBoard[occupiedSquares]))
				// not valid pawn attack
				&& ( !isSquareSet( Movegen::attackFrom<blackPawns>( m.getFrom() ), m.getTo() ) || !isSquareSet( Them[Pieces] | bitSet(s.getEpSquare()), m.getTo() ) )
			){
				return false;
			}

			if(getRankOf(m.getFrom())==RANK2 && !

					m.isPromotionMove()){
				return false;

			}
			if( m.isEnPassantMove() )
			{
				auto epSq = s.getEpSquare();
				bitMap captureSquare = fileMask(epSq) & rankMask(m.getFrom());
				bitMap occ = bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(epSq)^captureSquare;
				tSquare kingSquare = getSquareOfOurKing();
				assert(kingSquare<squareNumber);
				if((Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Them[Queens] | Them[Rooks]))|
							(Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Them[Queens] | Them[Bishops])))
				{
				return false;
				}
			}
			break;
		default:
			return false;

	}


	return true;
}


Position::Position():_ply(0), _mg(*this)
{
	stateInfo.clear();
	stateInfo.emplace_back(state());
	stateInfo[0].setNextTurn( whiteTurn );

	updateUsThem();

}


Position::Position(const Position& other): _ply(other._ply), _mg(*this), stateInfo(other.stateInfo), squares(other.squares), bitBoard(other.bitBoard)
{
	updateUsThem();
}


Position& Position::operator=(const Position& other)
{
	if (this == &other)
	{
		return *this;
	}

	_ply = other._ply;

	stateInfo = other.stateInfo;
	squares = other.squares;
	bitBoard = other.bitBoard;

	updateUsThem();

	return *this;
}

inline void Position::updateUsThem()
{
	auto turn = getNextTurn();
	Us = &bitBoard[ turn ];
	Them = &bitBoard[ blackTurn - turn ];
}

/*! \brief put a piece on the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::putPiece(const bitboardIndex piece,const tSquare s)
{
	assert(s<squareNumber);
	assert( isValidPiece( piece ) );
	bitboardIndex color = isBlackPiece( piece )? blackPieces: whitePieces;
	bitMap b = bitSet(s);

	assert( squares[s] == empty );

	squares[s] = piece;
	bitBoard[piece] |= b;
	bitBoard[occupiedSquares] |= b;
	bitBoard[color] |= b;
}

/*! \brief move a piece on the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::movePiece(const bitboardIndex piece,const tSquare from,const tSquare to)
{
	assert(from<squareNumber);
	assert(to<squareNumber);
	assert( isValidPiece( piece ) );
	// index[from] is not updated and becomes stale. This works as long
	// as index[] is accessed just by known occupied squares.
	assert(getPieceAt(from)!=empty);
	assert(getPieceAt(to)==empty);
	bitMap fromTo = bitSet(from) ^ bitSet(to);
	bitboardIndex color = isBlackPiece( piece ) ? blackPieces : whitePieces;
	bitBoard[occupiedSquares] ^= fromTo;
	bitBoard[piece] ^= fromTo;
	bitBoard[color] ^= fromTo;
	squares[from] = empty;
	squares[to] = piece;


}
/*! \brief remove a piece from the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::removePiece(const bitboardIndex piece,const tSquare s)
{

	assert(!isKing(piece));
	assert(s<squareNumber);
	assert( isValidPiece( piece) );
	assert(getPieceAt(s)!=empty);

	// WARNING: This is not a reversible operation. If we remove a piece in
	// do_move() and then replace it in undo_move() we will put it at the end of
	// the list and not in its original place, it means index[] and pieceList[]
	// are not guaranteed to be invariant to a do_move() + undo_move() sequence.
	bitboardIndex color = isBlackPiece( piece ) ? blackPieces : whitePieces;
	bitMap b = bitSet(s);
	bitBoard[occupiedSquares] ^= b;
	bitBoard[piece] ^= b;
	bitBoard[color] ^= b;

	squares[s] = empty;

}

std::string Position::_printEpSquare( const state& st)
{
	std::string s;
	if( st.hasEpSquare() )
	{
		// todo we have a similar method in command
		s += char('a'+getFileOf( st.getEpSquare() ) );
		s += char('1'+getRankOf( st.getEpSquare() ) );
	}
	else
	{
		s += '-';
	}
	return s;
}

/*! \brief insert a new state in memory
	\author Marco Belli
	\version 1.0
	\version 1.1 get rid of continuos malloc/free
	\date 21/11/2013
*/
inline void Position::insertState( state & s )
{
	stateInfo.emplace_back(s);
}

/*! \brief  remove the last state
	\author Marco Belli
	\version 1.0
	\version 1.1 get rid of continuos malloc/free
	\date 21/11/2013
*/
inline void  Position::removeState()
{
	stateInfo.pop_back();
}

unsigned int Position::getNumberOfLegalMoves() const
{
	MoveList<MAX_MOVE_PER_POSITION> moveList;
	_mg.generateMoves<Movegen::allMg>( moveList );
	return moveList.size();
}

bool Position::isCastlePathFree( const eCastle c ) const
{
	assert( c < 9);
	return !( _castlePath[c] & getOccupationBitmap() );
}
