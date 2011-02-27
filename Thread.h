/*
  Copyright (C) 2006-2011 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/
#ifndef THREAD_H
#define THREAD_H

#ifdef WINDOWS
    #include <windows.h>
    #include <winbase.h>
#else
    #include <pthread.h>
#endif

class Thread{
    public:
	Thread();
	~Thread();
	bool start();
	void stop();
	bool is_running();
	virtual void run()=0;
    protected:
	bool stopnow;//daca dau stop, atunci stop-now este true
    private:
    #ifdef WINDOWS
	friend DWORD WINAPI thread_function( LPVOID arg );
    #else
	friend void *thread_function(void *arg);
    #endif
	bool running;
	bool stopped;
    #ifdef WINDOWS
	HANDLE hThread;
    #else
	pthread_t thread;
    #endif
};

#endif




