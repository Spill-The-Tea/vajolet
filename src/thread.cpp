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

#include <condition_variable>
#include <thread>
#include <mutex>

#include "vajo_io.h"
#include "position.h"
#include "search.h"
#include "searchLimits.h"
#include "searchTimer.h"
#include "timeManagement.h"
#include "thread.h"
#include "transposition.h"
#include "uciParameters.h"


/*********************************************
* implementation
**********************************************/
class my_thread::impl
{
private:

	const unsigned int _initTimeout = 1;
	
	enum threadStatus
	{
		initalizing,
		ready,
		running
	};
	
	volatile threadStatus _searchStatus = initalizing;
	volatile threadStatus _timerStatus = initalizing;
	
	std::thread _searcher;
	std::thread _timer;
	
	std::mutex _sMutex;
	std::mutex _tMutex;
	
	std::condition_variable _searchCond;
	std::condition_variable _timerCond;
	
	volatile static bool _quit;
	volatile static bool _startThink;
	
		
	SearchLimits _limits; // todo limits belong to threads
	SearchTimer _st;
	Search _src;
	timeManagement _timeMan;
	std::unique_ptr<UciOutput> _UOI;
	long long _lastHasfullMessageTime;


	bool _initThreads();
	void _quitThreads();
	
	void _timerThread();
	void _searchThread();
	void _printTimeDependentOutput( long long int time );
	void _stopPonder();

public:
	explicit impl();
	~impl();
	void startThinking( const Position& p, SearchLimits& l);
	void stopThinking();
	void ponderHit();
	timeManagement& getTimeMan();
};

volatile bool my_thread::impl::_quit = false;
volatile bool my_thread::impl::_startThink = false;


my_thread::impl::impl(): _src(_st, _limits), _timeMan(_limits), _UOI(UciOutput::create())
{
	if( !_initThreads() )
	{
		sync_cout<<"unable to initialize threads, exiting..."<<sync_endl;
		exit(-1);
	}
}

my_thread::impl::~impl()
{
	_quitThreads();
}

timeManagement& my_thread::impl::getTimeMan(){ return _timeMan; }

void my_thread::impl::_printTimeDependentOutput(long long int time) {

	if( time - _lastHasfullMessageTime > 1000 )
	{
		_lastHasfullMessageTime = time;

		_UOI->printGeneralInfo(transpositionTable::getInstance().getFullness(),	_src.getTbHits(), _src.getVisitedNodes(), time);

		if(uciParameters::showCurrentLine)
		{
			_src.showLine();
		}
	}
}

void my_thread::impl::_timerThread()
{
	std::unique_lock<std::mutex> lk(_tMutex);
	
	while (!_quit)
	{
		_timerStatus = ready;
		_timerCond.notify_one();
		
		_timerCond.wait(lk, [&]{return (_startThink && !_timeMan.isSearchFinished() ) || _quit;} );
		
		_timerStatus = running;
		_timerCond.notify_one();

		if (!_quit)
		{
			long long int time = _st.getClockTime();
			
			bool stop = _timeMan.stateMachineStep( time, _src.getVisitedNodes() );
			if( stop )
			{
				_src.stopSearch();
			}

#ifndef DISABLE_TIME_DIPENDENT_OUTPUT
			_printTimeDependentOutput( time );
#endif
			std::this_thread::sleep_for(std::chrono::milliseconds( _timeMan.getResolution() ));
		}
	}
	_src.stopSearch();
}

void my_thread::impl::_searchThread()
{
	std::unique_lock<std::mutex> lk(_sMutex);
	
	while (!_quit)
	{
		_searchStatus = ready;
		_searchCond.notify_one();
		
		_searchCond.wait(lk, [&]{return _startThink||_quit;} );
		
		_searchStatus = running;
		_searchCond.notify_one();
		
		if(!_quit)
		{
			_limits.checkInfiniteSearch();
			_timeMan.initNewSearch( _src.getPosition().getNextTurn() );
			_src.resetStopCondition();
			_st.resetTimers();
			_timerCond.notify_one();
			_src.manageNewSearch();
			_startThink = false;
			
		}
	}
}

bool my_thread::impl::_initThreads()
{
	std::lock( _tMutex, _sMutex );
	std::unique_lock<std::mutex> lckt(_tMutex, std::adopt_lock);
	std::unique_lock<std::mutex> lcks(_sMutex, std::adopt_lock);
	_timer = std::thread(&my_thread::impl::_timerThread, this);
	_searcher = std::thread(&my_thread::impl::_searchThread, this);
	_src.stopSearch();
	
	// wait initialization
	if( !_timerCond.wait_for( lckt, std::chrono::seconds( _initTimeout ), [&]{ return _timerStatus == ready;} ) ) return false;
	if( !_searchCond.wait_for( lcks, std::chrono::seconds( _initTimeout ), [&]{ return _searchStatus == ready;} ) ) return false;

	return true;
}

inline void my_thread::impl::_quitThreads()
{
	std::lock( _tMutex, _sMutex );
	std::unique_lock<std::mutex> lks(_sMutex, std::adopt_lock);
	std::unique_lock<std::mutex> lkt(_tMutex, std::adopt_lock);
	_quit = true;
	lks.unlock();
	lkt.unlock();

	_searchCond.notify_one();
	_timerCond.notify_one();
	_timer.join();
	_searcher.join();
}

inline void my_thread::impl::startThinking( const Position& p, SearchLimits& l)
{
	_src.stopSearch();
	_lastHasfullMessageTime = 0;
	std::lock( _tMutex, _sMutex );
	std::unique_lock<std::mutex> lcks(_sMutex, std::adopt_lock);
	_searchCond.wait( lcks, [&]{ return _searchStatus == ready; } );
	std::unique_lock<std::mutex> lckt(_tMutex, std::adopt_lock);
	_timerCond.wait( lckt, [&]{ return _timerStatus == ready; } );

	_limits = l;
	_src.getPosition() = p;
	_startThink = true;
	_searchCond.notify_one();
	
}

inline void my_thread::impl::stopThinking()
{
	_timeMan.stop();
	_stopPonder();
}

inline void my_thread::impl::ponderHit()
{
	_st.resetClockTimer();
	_stopPonder();
}

inline void my_thread::impl::_stopPonder(){ _limits.setPonder(false);}



/*********************************************
* my_thread class
**********************************************/

my_thread::my_thread(): pimpl{std::make_unique<impl>()}{}

my_thread::~my_thread() = default;

void my_thread::stopThinking() { pimpl->stopThinking();}

void my_thread::ponderHit() { pimpl->ponderHit();}

timeManagement& my_thread::getTimeMan(){ return pimpl->getTimeMan(); }

void my_thread::startThinking( const Position& p, SearchLimits& l){	pimpl->startThinking( p, l); }
