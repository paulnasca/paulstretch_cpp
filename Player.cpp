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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "Player.h"
#include "globals.h"

static int player_count=0;

Player::Player():Thread(){
	player_count++;
	if (player_count>1) {
		printf("Error: Player class multiples instances.\n");
		exit(1);
	};

	stretchl=NULL;
	stretchr=NULL;
	binaural_beats=NULL;

	ai=NULL;    

	newtask.mode=TASK_NONE;
	newtask.startpos=0.0;
	newtask.rap=1.0;
	newtask.fftsize=4096;

	task=newtask;
	mode=MODE_STOP;

	outbuf.n=0;
	outbuf.datal=NULL;
	outbuf.datar=NULL;
	outbuf.size=0;
	outbuf.computek=0;
	outbuf.outk=0;
	outbuf.outpos=0;
	outbuf.nfresh=0;
	outbuf.in_position=0;
	inbuf_i=NULL;
	info.position=0;
	freeze_mode=false;
	bypass_mode=false;
	first_in_buf=true;
	window_type=W_HANN;
	inbuf.l=NULL;
	inbuf.r=NULL;

	paused=false;

	info.playing=false;
	info.samplerate=44100;
	info.eof=true;
	volume=1.0;
	onset_detection_sensitivity=0.0;
};

Player::~Player(){
	player_count--;

	stop();
	if (stretchl) delete stretchl;stretchl=NULL;
	if (stretchr) delete stretchr;stretchr=NULL;
	if (outbuf.in_position) delete outbuf.in_position;
	if (inbuf.l) delete inbuf.l;
	if (inbuf.r) delete inbuf.r;
	if (inbuf_i) delete inbuf_i;
	if (ai) delete ai;
	if (binaural_beats) delete binaural_beats;binaural_beats=NULL;
};

Player::ModeType Player::getmode(){
	return mode;
};

void Player::startplay(std::string filename, REALTYPE startpos,REALTYPE rap, int fftsize,FILE_TYPE intype,bool bypass,ProcessParameters *ppar,BinauralBeatsParameters *bbpar){
	info.playing=false;
	info.eof=false;
	bypass_mode=bypass;
	if (bypass) freeze_mode=false;
	paused=false;
	taskmutex.lock();
	newtask.mode=TASK_START;
	newtask.filename=filename;
	newtask.startpos=startpos;
	newtask.rap=rap;
	newtask.fftsize=fftsize;
	newtask.intype=intype;
	newtask.bypass=bypass;
	newtask.ppar=ppar;
	newtask.bbpar=bbpar;
	taskmutex.unlock();
};

void Player::stop(){
	//pun 0 la outbuf
	info.playing=false;
	/*    taskmutex.lock();
		  newtask.mode=TASK_STOP;
		  taskmutex.unlock();*/
};
void Player::pause(){
	paused=!paused;
};

void Player::seek(REALTYPE pos){
	taskmutex.lock();
	newtask.mode=TASK_SEEK;
	newtask.startpos=pos;
	taskmutex.unlock();
};
void Player::freeze(){
	freeze_mode=!freeze_mode;
	if (bypass_mode) freeze_mode=false;
};

void Player::setrap(REALTYPE newrap){
	taskmutex.lock();
	newtask.mode=TASK_RAP;
	newtask.rap=newrap;
	taskmutex.unlock();
};

void Player::set_process_parameters(ProcessParameters *ppar,BinauralBeatsParameters *bbpar){
	taskmutex.lock();
	newtask.mode=TASK_PARAMETERS;
	newtask.ppar=ppar;
	newtask.bbpar=bbpar;
	taskmutex.unlock();
};


void Player::set_window_type(FFTWindow window){
	window_type=window;
};

void Player::set_volume(REALTYPE vol){
	volume=vol;
};
void Player::set_onset_detection_sensitivity(REALTYPE onset){
	onset_detection_sensitivity=onset;
};

void Player::getaudiobuffer(int nsamples, float *out){
	if (mode==MODE_PREPARING){
		for (int i=0;i<nsamples*2;i++){
			out[i]=0.0;
		};
		return;
	};
	if (paused){
		for (int i=0;i<nsamples*2;i++){
			out[i]=0.0;
		};
		return;
	};

	bufmutex.lock();
	if ((outbuf.n==0)||(outbuf.nfresh==0)){
		bufmutex.unlock();
		for (int i=0;i<nsamples*2;i++){
			out[i]=0.0;
		};
		return;
	};
	int k=outbuf.outk,pos=outbuf.outpos;

	//     printf("%d in_pos=%g\n",info.eof,outbuf.in_position[k]);
	if (info.eof) mode=MODE_STOP;
	else info.position=outbuf.in_position[k];
	for (int i=0;i<nsamples;i++){
		out[i*2]=outbuf.datal[k][pos]*volume;
		out[i*2+1]=outbuf.datar[k][pos]*volume;

		pos++;
		if (pos>=outbuf.size) {
			pos=0;
			k++;
			if (k>=outbuf.n) k=0;

			outbuf.nfresh--;
			//printf("%d %d\n",k,outbuf.nfresh);

			if (outbuf.nfresh<0){//Underflow
				outbuf.nfresh=0;
				for (int j=i;j<nsamples;j++){
					out[j*2]=0.0;
					out[j*2+1]=0.0;
				};
				break;
			};

		};
	};
	outbuf.outk=k;
	outbuf.outpos=pos;
	bufmutex.unlock();


};


void Player::run(){
	while(1){
		newtaskcheck();

		if (mode==MODE_STOP) sleep(10);
		if ((mode==MODE_PLAY)||(mode==MODE_PREPARING)){
			computesamples();
		};


		task.mode=TASK_NONE;

	};
};

void Player::newtaskcheck(){
	TaskMode newmode=TASK_NONE;
	taskmutex.lock();
	if (task.mode!=newtask.mode) {
		newmode=newtask.mode;
		task=newtask;
	};
	newtask.mode=TASK_NONE;
	taskmutex.unlock();

	if (newmode==TASK_START){
		if (current_filename!=task.filename){
			current_filename=task.filename;
			task.startpos=0.0;
		};
		switch (task.intype){
			case FILE_VORBIS:ai=new VorbisInputS;
							 break;
			case FILE_MP3:ai=new MP3InputS;
						  break;
			default: ai=new AInputS;
		};
		if (ai->open(task.filename)){
			info.samplerate=ai->info.samplerate;
			mode=MODE_PREPARING;
			ai->seek(task.startpos);

			bufmutex.lock();

			if (stretchl) delete stretchl;stretchl=NULL;
			if (stretchr) delete stretchr;stretchr=NULL;
			stretchl=new ProcessedStretch(task.rap,task.fftsize,window_type,task.bypass,ai->info.samplerate,1);
			stretchr=new ProcessedStretch(task.rap,task.fftsize,window_type,task.bypass,ai->info.samplerate,2);
			if (binaural_beats) delete binaural_beats;binaural_beats=NULL;
			binaural_beats=new BinauralBeats(ai->info.samplerate);

			if (stretchl) stretchl->set_parameters(task.ppar);
			if (stretchr) stretchr->set_parameters(task.ppar);
			if (binaural_beats) binaural_beats->pars=*(task.bbpar);

			inbufsize=stretchl->get_max_bufsize();
			if (inbuf.l) delete []inbuf.l;inbuf.l=NULL;
			if (inbuf.r) delete []inbuf.r;inbuf.r=NULL;
			inbuf.l=new REALTYPE[inbufsize];
			inbuf.r=new REALTYPE[inbufsize];
			for (int i=0;i<inbufsize;i++) inbuf.l[i]=inbuf.r[i]=0.0;

			if (outbuf.datal){
				for (int j=0;j<outbuf.n;j++){
					delete [] outbuf.datal[j];
				};
				delete [] outbuf.datal;
				outbuf.datal=NULL;
			};
			if (outbuf.datar){
				for (int j=0;j<outbuf.n;j++){
					delete [] outbuf.datar[j];
				};
				delete [] outbuf.datar;
				outbuf.datar=NULL;
			};
			delete[] inbuf_i;

			if (outbuf.in_position) {
				delete outbuf.in_position;
				outbuf.in_position=NULL;
			};

			inbuf_i=new short int[inbufsize*2];//for left and right
			for (int i=0;i<inbufsize;i++){
				inbuf_i[i*2]=inbuf_i[i*2+1]=0;
			};
			first_in_buf=true;

			outbuf.size=stretchl->get_bufsize();

			int min_samples=ai->info.samplerate*2;
			int n=2*PA_SOUND_BUFFER_SIZE/outbuf.size;
			if (n<3) n=3;//min 3 buffers
			if (n<(min_samples/outbuf.size)) n=(min_samples/outbuf.size);//the internal buffers sums "min_samples" amount
			outbuf.n=n;
			outbuf.nfresh=0;
			outbuf.datal=new float *[outbuf.n];
			outbuf.datar=new float *[outbuf.n];
			outbuf.computek=0;
			outbuf.outk=0;
			outbuf.outpos=0;
			outbuf.in_position=new float[outbuf.n];
			for (int j=0;j<outbuf.n;j++){
				outbuf.datal[j]=new float[outbuf.size];
				for (int i=0;i<outbuf.size;i++) outbuf.datal[j][i]=0.0;
				outbuf.datar[j]=new float[outbuf.size];
				for (int i=0;i<outbuf.size;i++) outbuf.datar[j][i]=0.0;
				outbuf.in_position[j]=0.0;
			};

			bufmutex.unlock();	    	    

		};
	};
	if (newmode==TASK_SEEK){
		if (ai) ai->seek(task.startpos);
		first_in_buf=true;
	};
	if (newmode==TASK_RAP){
		if (stretchl) stretchl->set_rap(task.rap);
		if (stretchl) stretchr->set_rap(task.rap);
	};
	if (newmode==TASK_PARAMETERS){
		if (stretchl) stretchl->set_parameters(task.ppar);
		if (stretchr) stretchr->set_parameters(task.ppar);
		if (binaural_beats) binaural_beats->pars=*(task.bbpar);
	};
};

void Player::computesamples(){
	bufmutex.lock();
	bool exitnow=(outbuf.n==0);
	if (outbuf.nfresh>=(outbuf.n-1)) exitnow=true;//buffers are full

	bufmutex.unlock();
	if (exitnow) {
		if (mode==MODE_PREPARING) {
			info.playing=true;
			mode=MODE_PLAY;
		};
		sleep(10);
		return;
	};

	bool eof=false;
	if (!ai) eof=true;
	else if (ai->eof) eof=true;
	if (eof){
		for (int i=0;i<inbufsize;i++){
			inbuf_i[i*2]=inbuf_i[i*2+1]=0;
		};
		outbuf.nfresh++;
		bufmutex.lock();
		for (int i=0;i<outbuf.size;i++){
			outbuf.datal[outbuf.computek][i]=0.0;
			outbuf.datar[outbuf.computek][i]=0.0;
		};
		outbuf.computek++;
		if (outbuf.computek>=outbuf.n){
			outbuf.computek=0;
		};
		bufmutex.unlock();
		info.eof=true;
		return;
	};  

	bool result=true;
	float in_pos_100=(REALTYPE) ai->info.currentsample/(REALTYPE)ai->info.nsamples*100.0;
	int readsize=stretchl->get_nsamples(in_pos_100);


	if (first_in_buf) readsize=stretchl->get_nsamples_for_fill();
		else if (freeze_mode) readsize=0;
	if (readsize) result=(ai->read(readsize,inbuf_i)==(readsize));
	if (result){
		float in_pos=(REALTYPE) ai->info.currentsample/(REALTYPE)ai->info.nsamples;
		if (ai->eof) in_pos=0.0;

		REALTYPE tmp=1.0/32768.0;
		for (int i=0;i<readsize;i++){
			inbuf.l[i]=inbuf_i[i*2]*tmp;
			inbuf.r[i]=inbuf_i[i*2+1]*tmp;
		};

		stretchl->window_type=window_type;
		stretchr->window_type=window_type;
		REALTYPE s_onset=onset_detection_sensitivity;
		stretchl->set_onset_detection_sensitivity(s_onset);
		stretchr->set_onset_detection_sensitivity(s_onset);
		bool freezing=freeze_mode&&(!first_in_buf);
		stretchl->set_freezing(freezing);
		stretchr->set_freezing(freezing);

		REALTYPE onset_l=stretchl->process(inbuf.l,readsize);
		REALTYPE onset_r=stretchr->process(inbuf.r,readsize);
		REALTYPE onset=(onset_l>onset_r)?onset_l:onset_r;
		stretchl->here_is_onset(onset);
		stretchr->here_is_onset(onset);
		
		binaural_beats->process(stretchl->out_buf,stretchr->out_buf,stretchl->get_bufsize(),in_pos_100);
		//	stretchl->process_output(stretchl->out_buf,stretchl->out_bufsize);
		//	stretchr->process_output(stretchr->out_buf,stretchr->out_bufsize);
		int nskip=stretchl->get_skip_nsamples();
		if (nskip>0) ai->skip(nskip);


		first_in_buf=false;
		bufmutex.lock();

		for (int i=0;i<outbuf.size;i++){
			REALTYPE l=stretchl->out_buf[i],r=stretchr->out_buf[i];
			if (l<-1.0) l=-1.0;
			else if (l>1.0) l=1.0;
			if (r<-1.0) r=-1.0;
			else if (r>1.0) r=1.0;

			outbuf.datal[outbuf.computek][i]=l;
			outbuf.datar[outbuf.computek][i]=r;
		};
		outbuf.in_position[outbuf.computek]=in_pos;
		outbuf.computek++;
		if (outbuf.computek>=outbuf.n){
			outbuf.computek=0;
		};
		bufmutex.unlock();
		outbuf.nfresh++;
	}else{
		info.eof=true;
		mode=MODE_STOP;
		stop();
	};
};

