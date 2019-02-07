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

#include <algorithm>
//#include <iostream>
#include "gtest/gtest.h"
#include "./../book.h"
//#include "./../command.h"
#include "./../move.h"
#include "./../position.h"

TEST(bookTest, probeKey){
	Position p;
	
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x463b96181691fc9cll);
	
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x823c9b50fd114196ll);
	
	p.setupFromFen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x0756b94461c50fb0ll);
	
	p.setupFromFen("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x662fafb965db29d4ll);
	
	p.setupFromFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x22a48b5a8e47ff78ll);
	
	p.setupFromFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x652a607ca3f242c1ll);
	
	p.setupFromFen("rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x00fdd303c946bdd9ll);
	
	p.setupFromFen("rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x3c8123ea7b067637ll);
	
	p.setupFromFen("rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4");
	ASSERT_EQ(PolyglotBook::polyglotKey(p), 0x5c3f9b829b279560ll);
}

TEST(bookTest, probeBest){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, true);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(E2,E4));
	}	
}

TEST(bookTest, probeBest1){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	p.doMove(Move(E2,E4));
	p.doMove(Move(C7,C6));
	p.doMove(Move(D2,D4));
	p.doMove(Move(D7,D5));
	p.doMove(Move(B1,C3));
	p.doMove(Move(D5,E4));
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(C3,E4));
	}	
}

TEST(bookTest, probeRandom1){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	p.doMove(Move(E2,E4));
	p.doMove(Move(C7,C6));
	p.doMove(Move(D2,D4));
	p.doMove(Move(D7,D5));
	p.doMove(Move(B1,C3));
	p.doMove(Move(D5,E4));
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(C3,E4));
	}	
}

TEST(bookTest, probeCastle){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
	p.doMove(Move(E2,E4));
	p.doMove(Move(E7,E5));
	p.doMove(Move(G1,F3));
	p.doMove(Move(B8,C6));
	p.doMove(Move(F1,C4));
	p.doMove(Move(F8,C5));
	
	std::vector<Move> v ={ Move(C2,C3), Move(E1,G1,Move::fcastle)};
	
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		auto res = std::find(v.begin(), v.end(), m);
		//if( res != v.end())std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_NE(res, v.end());
	}	
}

TEST(bookTest, probeRandom){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	std::vector<Move> v ={ Move(E7,E5), Move(C7,C5), Move(E7,E6), Move(C7,C6)};
	p.doMove(Move(E2,E4));
	
	PolyglotBook b;
	for (int i = 0;i<1000;++i) {
		Move m = b.probe(p, false);
		auto res = std::find(v.begin(), v.end(), m);
		//if( res == v.end())std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_NE(res, v.end());
	}	
}

/*TEST(bookTest, probeEnPassant){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
	p.doMove(Move(E2,E4));
	p.doMove(Move(G7,G6));
	//p.doMove(Move(D2,D4));
	//p.doMove(Move(F8,G7));
	//p.doMove(Move(F1,C4));
	//p.doMove(Move(F8,C5));
	
	std::vector<Move> v ={ Move(C2,C3), Move(E1,G1,Move::fcastle)};
	
	PolyglotBook b;
	for (int i = 0;i<1;++i) {
		Move m = b.probe(p, false);
		auto res = std::find(v.begin(), v.end(), m);
		//if( res != v.end())std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_NE(res, v.end());
	}	
}*/

TEST(bookTest, probeFail){
	Position p;
	p.setupFromFen("rnbqkbnr/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	Move m = b.probe(p, true);
	//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
	ASSERT_EQ(m, Move::NOMOVE);
}

// todo promotion and enpassant?

