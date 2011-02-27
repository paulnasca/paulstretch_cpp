/*
  Copyright (C) 2006-2009 Nasca Octavian Paul
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


#define BUFSIZE 4096

#include <stdio.h>
#include <stdlib.h>
#include "VorbisInputS.h"
using namespace std;



VorbisInputS::VorbisInputS(){
    info.nsamples=0;
    info.nchannels=0;
    info.samplerate=1;
    info.currentsample=0;
    eof=false;
    vorbis_opened=false;
    real_channels=1;
    buf=NULL;
    bufsize=0;
};

VorbisInputS::~VorbisInputS(){
    close();
};

bool VorbisInputS::open(string filename){
    close();//inchide un posibil fisier existent
    eof=true;
    FILE *f=fopen(filename.c_str(),"rb");
    if (!f) return false;

    int result=ov_open(f,&vorbisfile,NULL,0);
    if (result<0){
	fclose(f);
	return false;
    };
    vorbis_opened=true;
    
    vorbis_info *vinfo=ov_info(&vorbisfile,0);
    if (!vinfo) return false;
    real_channels=vinfo->channels;
    if ((real_channels!=1)&&(real_channels!=2))return false;//only mono&stereo oggs are allowed
    info.nsamples=ov_pcm_total(&vorbisfile,0);
    info.nchannels=2;
    bufsize=real_channels*BUFSIZE;
    buf=new short int[bufsize];
    info.samplerate=vinfo->rate;
    info.currentsample=0;
    
    eof=false;
        	
    
    return(true);
};

void VorbisInputS::close(){
    if (vorbis_opened){
	delete [] buf;
	ov_clear(&vorbisfile);
	vorbis_opened=false;
    };
};

int VorbisInputS::read(int nsmps,short int *smps){
    if (!vorbis_opened) {
	eof=true;
	return 0;
    };
    if (eof) {
	for (int i=0;i<nsmps;i++) smps[i]=0;
	return nsmps;
    };

    int current_section=0;
    int nsmps_todo=nsmps;
    int nsmps_done=0;
    short int *smps_orig=smps;
    while (nsmps_todo>=0){
	int chunk_size=(nsmps_todo<BUFSIZE)?nsmps_todo:BUFSIZE;
	if (chunk_size==0) break;
	int readed=ov_read(&vorbisfile,(char *)buf,chunk_size*sizeof(short int)*real_channels,0,2,1,&current_section);
        if ((readed<=0)||(current_section!=0)){
	    eof=true;
	    for (int i=0;i<nsmps_todo;i++) smps[i]=0;
	    return nsmps;
	};
	readed/=sizeof(short int);//from bytes to short int samples
	switch(real_channels){
	    case 1:
		for (int i=0;i<readed;i++){
		    smps[i*2]=smps[i*2+1]=buf[i];
		};
		smps+=readed*2;
		nsmps_done+=readed;
		nsmps_todo-=readed;
		break;
	    case 2:
		for (int i=0;i<readed;i++){
		    smps[i]=buf[i];
		};
		nsmps_done+=readed/2;
		nsmps_todo-=readed/2;
		smps+=readed;
		break;
	};
	info.currentsample+=readed/real_channels;
    };
    
    return nsmps;
};

void VorbisInputS::seek(double pos){
    if (!vorbis_opened) return;
    long int p=(long int)(pos*info.nsamples);
    ov_pcm_seek(&vorbisfile,p);
    info.currentsample=p;
};

