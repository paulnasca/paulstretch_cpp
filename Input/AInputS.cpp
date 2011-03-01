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


#include <stdio.h>
#include <stdlib.h>
#include "AInputS.h"
using namespace std;

AInputS::AInputS(){
    handle=AF_NULL_FILEHANDLE;
    info.nsamples=0;
    info.nchannels=0;
    info.samplerate=1;
    info.currentsample=0;
    eof=false;
};

AInputS::~AInputS(){
    close();
};

bool AInputS::open(string filename){
    close();//inchide un posibil fisier existent
    handle=afOpenFile(filename.c_str(),"r",0);
	if (handle==AF_NULL_FILEHANDLE){//eroare
		eof=true;
		return false;
	};
    
  
    afSetVirtualChannels(handle,AF_DEFAULT_TRACK,2);

    eof=false;
    info.nsamples=afGetFrameCount(handle,AF_DEFAULT_TRACK);
    info.nchannels=afGetVirtualChannels(handle,AF_DEFAULT_TRACK);
    info.samplerate=(int) afGetRate(handle,AF_DEFAULT_TRACK);
    info.currentsample=0;
	if (info.samplerate==0) return false;
        	
    //fac ca intrarea sa fie pe 16 biti
    afSetVirtualSampleFormat(handle,AF_DEFAULT_TRACK,AF_SAMPFMT_TWOSCOMP,16);
    
    return true;
};

void AInputS::close(){
    if (handle!=AF_NULL_FILEHANDLE){
	afCloseFile(handle);
	handle=AF_NULL_FILEHANDLE;
    };
};

int AInputS::read(int nsmps,short int *smps){
    if (handle==AF_NULL_FILEHANDLE) return 0;

    int readed=afReadFrames(handle,AF_DEFAULT_TRACK,smps,nsmps);
    info.currentsample+=readed;
    if (readed!=nsmps) {
		if (readed>=0) for (int i=readed;i<nsmps;i++) smps[i]=0;
		info.currentsample=info.nsamples;
		eof=true;
    };
	return nsmps;
};

void AInputS::seek(double pos){
    if (handle==AF_NULL_FILEHANDLE) return;
    AFframecount p=(AFframecount)(pos*info.nsamples);
	if (p==info.currentsample) return;
    AFframecount seek=afSeekFrame(handle,AF_DEFAULT_TRACK,p);
	//
	printf("AInputS::seek %d %d %d - possible audiofile bug (it always seeks to the end of the file) \n",(int)p,(int)seek,(int)afGetFrameCount(handle,AF_DEFAULT_TRACK));
    info.currentsample=p;
};

