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

#define BUFSIZE 4096 //minimum 2 buffere de mp3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MP3InputS.h"
using namespace std;


MP3InputS::MP3InputS(){
	info.nsamples=0;
	info.nchannels=2;
	info.samplerate=44100;
	info.currentsample=0;
	eof=false;
	MP3_opened=false;
	f=NULL;
	buf=NULL;
};

MP3InputS::~MP3InputS(){
	close();
};

bool MP3InputS::open(string filename){
	pcm_remained=0;
	close();//inchide un posibil fisier existent
	eof=true;
	f=fopen(filename.c_str(),"rb");
	if (!f) return false;
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);

	buf=new unsigned char[BUFSIZE];

	MP3_opened=true;
	eof=false;

	fseek(f,0,SEEK_END);
	info.nsamples=ftell(f);
	rewind(f);
	info.currentsample=0;
	info.samplerate=44100;

	{//find the samplerate
		const int n=8192;
		short int tmpsmp[n*2];
		read(n,tmpsmp);//read just a sample (and ignore it) to fill info.samplerate
		seek(0);
		pcm_remained=0;
		info.currentsample=0;
		//reinit the mad
		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);
		
		mad_stream_init(&stream);
		mad_frame_init(&frame);
		mad_synth_init(&synth);
	};

	info.nchannels=2;
	return true;
};

void MP3InputS::close(){
	if (MP3_opened){
		MP3_opened=false;
		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);
		delete [] buf;
		fclose(f);
		f=NULL;
	};
};


int MP3InputS::read(int nsmps,short int *smps){
	int orig_nsmps=nsmps;
	short int *orig_smps=smps;
	if (!MP3_opened) {
		eof=true;
		return 0;
	};
	if (eof) {
		for (int i=0;i<nsmps;i++) smps[i]=0;
		return nsmps;
	};

	if (pcm_remained>0){
		int nconverted=pcm_remained;
		if (nconverted>nsmps) nconverted=nsmps;
		convertsmps(nconverted,smps,synth.pcm.length-pcm_remained);

		nsmps-=nconverted;
		smps+=nconverted*2;

		if (nsmps<=0){
			pcm_remained-=orig_nsmps;
			return orig_nsmps;
		};
	};

	while (nsmps>0){
		int remaining=0;
		if (stream.next_frame!=NULL){
			remaining=stream.bufend-stream.next_frame;
			memmove(buf,stream.next_frame,remaining);	    
		};
		int readsize=BUFSIZE-remaining;
		int readed=fread(buf+remaining,1,readsize,f);
		if (feof(f)) eof=true;
		if ((eof) || (readed<1)){
			for (int i=0;i<orig_nsmps;i++){
				orig_smps[i]=0;
			};
			return orig_nsmps;
		};
		info.currentsample+=readed;


		if (readed!=readsize) {
			for (int i=readed;i<readsize;i++) buf[i+remaining]=0;
		};
		mad_stream_buffer(&stream, buf, BUFSIZE);
		while(1){
			if (mad_frame_decode(&frame, &stream)!=0){
				if (stream.error==MAD_ERROR_BUFLEN) break;
				if (stream.error==MAD_ERROR_LOSTSYNC) continue;
				if (!MAD_RECOVERABLE(stream.error)) {
					printf("Non recoverable MP3 error: 0x%x\n",stream.error);///in caz ca nu este recuperabil
					close();
					return orig_nsmps;
				};
			};
			mad_synth_frame(&synth, &frame);

			int n=synth.pcm.length;
			if (synth.pcm.samplerate!=0) info.samplerate=synth.pcm.samplerate;
			if (nsmps<synth.pcm.length){
				pcm_remained=synth.pcm.length-nsmps;
				n=nsmps;
			}else{
				pcm_remained=0;
			};

			convertsmps(n,smps,0);
			nsmps-=n;
			smps+=n*2;
			if (nsmps<=0) return orig_nsmps;
		};
	};

	return orig_nsmps;
};
void MP3InputS::convertsmps(int nsmps, short int *smps,int pcmstart){
	mad_fixed_t *l=synth.pcm.samples[0],*r=synth.pcm.samples[0];
	if (synth.pcm.channels==2) r=synth.pcm.samples[1];
	l+=pcmstart;r+=pcmstart;
	for (int i=0;i<nsmps;i++){//todo optimize (avoid reconverting r[] for mono samples)
		smps[i*2]=madpcm2short(l[i]);
		smps[i*2+1]=madpcm2short(r[i]);
	};
};


short int MP3InputS::madpcm2short(mad_fixed_t x){
	if (x>=MAD_F_ONE) x=MAD_F_ONE-1;
	else if (x<=-MAD_F_ONE) x=-MAD_F_ONE+1;
	int result= x >> (MAD_F_FRACBITS + 1 - 16);
	return result;
};

void MP3InputS::seek(double pos){
	if (!MP3_opened) return;    
	pcm_remained=0;

	int p=(int)(pos*info.nsamples);
	info.currentsample=p;
	fseek(f,p,SEEK_SET);
	/*    if (p==0) {
		  fseek(f,0,SEEK_SET);
		  }else{
	//todo add other thing here
	fseek(f,p,SEEK_SET);
	};
	*/
};

