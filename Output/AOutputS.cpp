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

#include <stdio.h>
#include <stdlib.h>
#include "AOutputS.h"
using namespace std;

AOutputS::AOutputS(){
    handle=AF_NULL_FILEHANDLE;
};

AOutputS::~AOutputS(){
    close();
};

bool AOutputS::newfile(string filename,int samplerate,bool use32bit){
    close();//inchide un posibil fisier existent

    AFfilesetup afsetup=afNewFileSetup();
    if (use32bit) afInitSampleFormat(afsetup,AF_DEFAULT_TRACK,AF_SAMPFMT_TWOSCOMP,32);
    afInitChannels(afsetup,AF_DEFAULT_TRACK,2);
    afInitRate(afsetup,AF_DEFAULT_TRACK,samplerate);    
    
    handle=afOpenFile(filename.c_str(),"w",afsetup);
    if (handle==AF_NULL_FILEHANDLE){//eroare
	afFreeFileSetup(afsetup);
	return(false);
    };
    
    afSetVirtualSampleFormat(handle,AF_DEFAULT_TRACK,AF_SAMPFMT_TWOSCOMP,32);
    afFreeFileSetup(afsetup);
    return(true);
};

void AOutputS::close(){
    if (handle!=AF_NULL_FILEHANDLE){
	afCloseFile(handle);
	handle=AF_NULL_FILEHANDLE;
    };
};

void AOutputS::write(int nsmps,int *smps){
    if (handle==AF_NULL_FILEHANDLE) return;
    afWriteFrames(handle,AF_DEFAULT_TRACK,smps,nsmps);
};

