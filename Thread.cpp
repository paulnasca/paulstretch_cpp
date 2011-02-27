/*
  Copyright (C) 2009 Nasca Octavian Paul
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
#include <unistd.h>
#include "Thread.h"
#include "globals.h"

#ifdef WINDOWS
DWORD WINAPI thread_function( LPVOID arg ) {
#else
void *thread_function(void *arg){
#endif
    Thread *thr=(Thread *) arg;
    thr->run();
    thr->stopped=true;
    thr->running=false;
    return 0;
};

Thread::Thread(){
    running=false;
    stopnow=false;
    stopped=false;
};

Thread::~Thread(){
    stop();
};

bool Thread::start(){
    if (running) return false;
#ifdef WINDOWS
    hThread=CreateThread(NULL,0,thread_function,this,0,NULL);
#else
    if (pthread_create(&thread,NULL,thread_function,this)!=0)return false;
#endif
    running=true;
    return true;
};

void Thread::stop(){
    if (!running) return;
    running=false;
    stopped=false;
    int maxwait=1000;
    stopnow=true;
    while(!stopped){
	sleep(10);
    };
};

bool Thread::is_running(){
    return running;
};



