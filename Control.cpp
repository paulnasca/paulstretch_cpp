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
#include <math.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include "globals.h"
#include "Control.h"
#include "XMLwrapper.h"
using namespace std;

Control::Control(){
	player=new Player();
	player->start();

	wavinfo.samplerate=44100;
	wavinfo.nsamples=0;
	wavinfo.intype=FILE_WAV;
	wav32bit=false;

	process.bufsize=16384;
	process.stretch=4.0;
	process.onset_detection_sensitivity=0.0;

	seek_pos=0.0;
	window_type=W_HANN;
	info.render_percent=-1.0;
	info.cancel_render=false;
	volume=1.0;


	gui_sliders.fftsize_s=0.5;
	gui_sliders.stretch_s=0.5;
	gui_sliders.mode_s=0;


///#warning test
///	process.transient.enable=true;
};

Control::~Control(){
	//    delete player; face crash daca il las
};
	

bool Control::set_input_filename(string filename,FILE_TYPE intype){
	InputS *ai=NULL;
	if (intype==FILE_VORBIS) ai=new VorbisInputS;
	if (intype==FILE_MP3) ai=new MP3InputS;
	if (intype==FILE_WAV) ai=new AInputS;
	if (intype==FILE_FLAC) ai=new AInputS;
	if (!ai) return false;
	wavinfo.filename=filename;
	wavinfo.intype=intype;
	bool result=ai->open(wavinfo.filename);
	if (result) {
		wavinfo.samplerate=ai->info.samplerate;
		wavinfo.nsamples=ai->info.nsamples;
///		if (process.transient.enable) {
///			pre_analyse_whole_audio(ai);
///		};

		delete ai;
	}else{
		wavinfo.filename="";
		wavinfo.samplerate=0;
		wavinfo.nsamples=0;
		delete ai;
	};
	return result;
};

string Control::get_input_filename(){
	return wavinfo.filename;
};
string Control::get_input_filename_and_info(){
	int seconds=wavinfo.nsamples/wavinfo.samplerate;
	const int size=200;
	char tmp[size];tmp[size-1]=0;
	snprintf(tmp,size-1," ( samplerate=%d; duration=%02d:%02d:%02d )",wavinfo.samplerate,seconds/3600,(seconds/60)%60,seconds%60);

	string filename=wavinfo.filename;
	int len=filename.length();
	if (len>70)filename=filename.substr(0,25)+"..."+filename.substr(len-35);
	return filename+tmp;
};
/*string Control::get_recommanded_output_filename(){
  return "none";
  };
  */


std::string Control::get_stretch_info(){
	const int size=200;
	char tmp[size];tmp[size-1]=0;

	if (wavinfo.nsamples==0) return "Stretch: ";


	double realduration=wavinfo.nsamples/wavinfo.samplerate*process.stretch;

	if (realduration>(365.25*86400.0*1.0e12)){//more than 1 trillion years
		double duration=(realduration/(365.25*86400.0*1.0e12));//my
		snprintf(tmp,size,"Stretch: %.7gx (%g trillion years)",process.stretch,duration);
		return tmp;
	};
	if (realduration>(365.25*86400.0*1.0e9)){//more than 1 billion years
		double duration=(realduration/(365.25*86400.0*1.0e9));//my
		snprintf(tmp,size,"Stretch: %.7gx (%g billion years)",process.stretch,duration);
		return tmp;
	};
	if (realduration>(365.25*86400.0*1.0e6)){//more than 1 million years
		double duration=(realduration/(365.25*86400.0*1.0e6));//my
		snprintf(tmp,size,"Stretch: %.7gx (%g million years)",process.stretch,duration);
		return tmp;
	};
	if (realduration>(365.25*86400.0*2000.0)){//more than two millenniums
		int duration=(int)(realduration/(365.25*86400.0));//years
		int years=duration%1000;
		int milleniums=duration/1000;

		char stryears[size];stryears[0]=0;
		if (years!=0){
			if (years==1) snprintf(stryears,size," 1 year");
			else snprintf(stryears,size," %d years",years);
		};
		snprintf(tmp,size,"Stretch: %.7gx (%d milleniums%s)",process.stretch,milleniums,stryears);
		return tmp;
	};
	if (realduration>(365.25*86400.0)){//more than 1 year
		int duration=(int) (realduration/3600.0);//hours
		int hours=duration%24;
		int days=(duration/24)%365;
		int years=duration/(365*24);

		char stryears[size];stryears[0]=0;
		if (years==1) snprintf(stryears,size,"1 year ");
		else snprintf(stryears,size,"%d years ",years);

		char strdays[size];strdays[0]=0;
		if (days>0){
			if (days==1) snprintf(strdays,size,"1 day");
			else snprintf(strdays,size,"%d days",days);
		};	
		if (years>=10) hours=0;
		char strhours[size];strhours[0]=0;
		if (hours>0){
			snprintf(strhours,size," %d h",hours);
		};	

		snprintf(tmp,size,"Stretch: %.7gx (%s%s%s)",process.stretch,stryears,strdays,strhours);
		return tmp;
	}else{//less than 1 year
		int duration=(int)(realduration);//seconds

		char strdays[size];strdays[0]=0;
		int days=duration/86400;
		if (days>0){
			if (days==1) snprintf(strdays,size,"1 day ");
			else snprintf(strdays,size,"%d days ",duration/86400);
		};	
		REALTYPE stretch=process.stretch;
		if (stretch>=1.0){
			stretch=((int) (stretch*100.0))*0.01;
		};
		snprintf(tmp,size,"Stretch: %.7gx (%s%.2d:%.2d:%.2d)",
				stretch,strdays,(duration/3600)%24,(duration/60)%60,duration%60);
		return tmp;
	};
	return "";
};

string Control::get_fftsize_info(){
	const int size=200;
	char tmp[size];tmp[size-1]=0;

	string fftsizelabel;
	fftsizelabel+="Window size (samples): ";

	if (wavinfo.nsamples==0) return fftsizelabel;
	fftsizelabel+=getfftsizestr(process.bufsize);

	return fftsizelabel;
};

string Control::get_fftresolution_info(){
	string resolution="Resolution: ";
	if (wavinfo.nsamples==0) return resolution;

	//todo: unctime and uncfreq are correct computed? Need to check later.
	REALTYPE unctime=process.bufsize/(REALTYPE)wavinfo.samplerate*sqrt(2.0);
	REALTYPE uncfreq=1.0/unctime*sqrt(2.0);
	char tmp[100];
	snprintf(tmp,100,"%.5g seconds",unctime);resolution+=tmp;
	snprintf(tmp,100," (%.5g Hz)",uncfreq);resolution+=tmp;
	return resolution;
};


void Control::startplay(bool bypass){
	if ((!player->info.playing)||(player->info.samplerate!=wavinfo.samplerate)){
		stopplay();
		sleep(200);
#ifdef HAVE_JACK
		JACKaudiooutputinit(player,wavinfo.samplerate);
#else
		PAaudiooutputinit(player,wavinfo.samplerate);
#endif
	};
	if (wavinfo.filename!="") player->startplay(wavinfo.filename,seek_pos,process.stretch,process.bufsize,wavinfo.intype,bypass,&ppar,&bbpar);
//	sleep(100);
//	update_process_parameters();
};
void Control::stopplay(){
	player->stop();
	player->seek(0.0);
	seek_pos=0;
#ifdef HAVE_JACK
	JACKclose();
#else
	PAfinish();
#endif
};
void Control::pauseplay(){
	player->pause();
};

void Control::freezeplay(){
	player->freeze();
};

void Control::set_volume(REALTYPE vol){
	volume=vol;
	player->set_volume(vol);
};


void Control::set_seek_pos(REALTYPE x){
	seek_pos=x;
	player->seek(x);
};
REALTYPE Control::get_seek_pos(){
	if (player->getmode()==Player::MODE_PLAY) seek_pos=player->info.position;
	return seek_pos;
};


void Control::set_stretch_controls(double stretch_s,int mode,double fftsize_s,double onset_detection_sensitivity){
	gui_sliders.stretch_s=stretch_s;
	gui_sliders.mode_s=mode;
	gui_sliders.fftsize_s=fftsize_s;

	double stretch=1.0;
	switch(mode){
		case 0:
			stretch_s=pow(stretch_s,1.2);
			stretch=pow(10.0,stretch_s*4.0);
			break;
		case 1:
			stretch_s=pow(stretch_s,1.5);
			stretch=pow(10.0,stretch_s*18.0);
			break;
		case 2:
			stretch=1.0/pow(10.0,stretch_s*2.0);
			break;
	};


	fftsize_s=pow(fftsize_s,1.5);
	int bufsize=(int)(pow(2.0,fftsize_s*12.0)*512.0);

	bufsize=optimizebufsize(bufsize);

	process.stretch=stretch;
	process.bufsize=bufsize;
	process.onset_detection_sensitivity=onset_detection_sensitivity;

};

double Control::get_stretch_control(double stretch,int mode){
	double result=1.0;
	switch(mode){
		case 0:
			if (stretch<1.0) return -1;
			stretch=(log(stretch)/log(10))*0.25;
			result=pow(stretch,1.0/1.2);
			break;
		case 1:
			if (stretch<1.0) return -1;
			stretch=(log(stretch)/log(10))/18.0;
			result=pow(stretch,1.0/1.5);
			break;
		case 2:
			if (stretch>1.0) return -1;
			result=2.0/(log(stretch)/log(10));
			break;
	};
	return result;
};


void Control::update_player_stretch(){
	player->setrap(process.stretch);
	player->set_onset_detection_sensitivity(process.onset_detection_sensitivity);
};


int abs_val(int x){
	if (x<0) return -x;
	else return x;
};


int Control::get_optimized_updown(int n,bool up){
	int orig_n=n;
	while(true){
		n=orig_n;
#ifndef KISSFFT
		while (!(n%11)) n/=11;
		while (!(n%7)) n/=7;
#endif
		while (!(n%5)) n/=5;
		while (!(n%3)) n/=3;
		while (!(n%2)) n/=2;
		if (n<2) break;
		if (up) orig_n++;
		else orig_n--;
		if (orig_n<4) return 4;
	};
	return orig_n;
};
int Control::optimizebufsize(int n){
	int n1=get_optimized_updown(n,false);
	int n2=get_optimized_updown(n,true);
	if ((n-n1)<(n2-n)) return n1;
	else return n2;
};



void Control::set_window_type(FFTWindow window){
	window_type=window;
	if (player) player->set_window_type(window);
};


string Control::Render(string inaudio,string outaudio,FILE_TYPE outtype,FILE_TYPE intype,REALTYPE pos1,REALTYPE pos2){
	if (pos2<pos1){
		REALTYPE tmp=pos2;
		pos2=pos1;
		pos1=tmp;
	};
	InputS *ai=NULL;
	switch(intype){
		case FILE_VORBIS:ai=new VorbisInputS;
						 break;
		case FILE_MP3:ai=new MP3InputS;
					  break;
		default:ai=new AInputS;
	};
	AOutputS ao;
	VorbisOutputS vorbisout;
	info.cancel_render=false;
	if (!ai->open(inaudio)){
		return "Error: Could not open audio file (or file format not recognized) :"+inaudio;
	};
	BinauralBeats bb(ai->info.samplerate);
	bb.pars=bbpar;
	if (outtype==FILE_WAV) ao.newfile(outaudio,ai->info.samplerate,wav32bit);
	if (outtype==FILE_VORBIS) vorbisout.newfile(outaudio,ai->info.samplerate);

	ai->seek(pos1);

	int inbufsize=process.bufsize;

	if (inbufsize<32) inbufsize=32;
	short int *inbuf_i=new short int[inbufsize*8];
	int outbufsize;
	struct{
		REALTYPE *l,*r;
	}inbuf;
	ProcessedStretch *stretchl=new ProcessedStretch(process.stretch,inbufsize,window_type,false,ai->info.samplerate,1);
	ProcessedStretch *stretchr=new ProcessedStretch(process.stretch,inbufsize,window_type,false,ai->info.samplerate,2);
	stretchl->set_onset_detection_sensitivity(process.onset_detection_sensitivity);
	stretchr->set_onset_detection_sensitivity(process.onset_detection_sensitivity);
	stretchl->set_parameters(&ppar);
	stretchr->set_parameters(&ppar);

	outbufsize=stretchl->get_bufsize();
	int *outbuf=new int[outbufsize*2];

	int poolsize=stretchl->get_max_bufsize();

	inbuf.l=new REALTYPE[poolsize];
	inbuf.r=new REALTYPE[poolsize];
	for (int i=0;i<poolsize;i++) inbuf.l[i]=inbuf.r[i]=0.0;

	int readsize=0;
	const int pause_max_write=65536;
	int pause_write=0;
	bool firstbuf=true;
	while(!ai->eof){
		float in_pos=(REALTYPE) ai->info.currentsample/(REALTYPE)ai->info.nsamples;
		if (firstbuf){
			readsize=stretchl->get_nsamples_for_fill();
			firstbuf=false;
		}else{
			readsize=stretchl->get_nsamples(in_pos*100.0);
		};
		int readed=0;
		if (readsize!=0) readed=ai->read(readsize,inbuf_i);
		
		for (int i=0;i<readed;i++) {
			inbuf.l[i]=inbuf_i[i*2]/32768.0;
			inbuf.r[i]=inbuf_i[i*2+1]/32768.0;
		};
		REALTYPE onset_l=stretchl->process(inbuf.l,readed);
		REALTYPE onset_r=stretchr->process(inbuf.r,readed);
		REALTYPE onset=(onset_l>onset_r)?onset_l:onset_r;
		stretchl->here_is_onset(onset);
		stretchr->here_is_onset(onset);
		bb.process(stretchl->out_buf,stretchr->out_buf,outbufsize,in_pos*100.0);
		for (int i=0;i<outbufsize;i++) {
			stretchl->out_buf[i]*=volume;
			stretchr->out_buf[i]*=volume;
		};
		int nskip=stretchl->get_skip_nsamples();
		if (nskip>0) ai->skip(nskip);

		if (outtype==FILE_WAV){	
			for (int i=0;i<outbufsize;i++) {
				REALTYPE l=stretchl->out_buf[i],r=stretchr->out_buf[i];
				if (l<-1.0) l=-1.0;
				else if (l>1.0) l=1.0;
				if (r<-1.0) r=-1.0;
				else if (r>1.0) r=1.0;
				outbuf[i*2]=(int)(l*32767.0*65536.0);
				outbuf[i*2+1]=(int)(r*32767.0*65536.0);
			};
			ao.write(outbufsize,outbuf);
		};
		if (outtype==FILE_VORBIS) vorbisout.write(outbufsize,stretchl->out_buf,stretchr->out_buf);

		REALTYPE totalf=ai->info.currentsample/(REALTYPE)ai->info.nsamples-pos1;
		if (totalf>(pos2-pos1)) break;

		info.render_percent=(totalf*100.0/(pos2-pos1+0.001));
		pause_write+=outbufsize;
		if (pause_write>pause_max_write){
			float tmp=outbufsize/1000000.0;
			if (tmp>0.1) tmp=0.1;
			Fl::wait(0.01+tmp);
			pause_write=0;
			if (info.cancel_render) break;
		};
	};

	delete stretchl;
	delete stretchr;    
	delete []outbuf;
	delete []inbuf_i;
	delete []inbuf.l;
	delete []inbuf.r;

	info.render_percent=-1.0;
	return "";
};



string Control::getfftsizestr(int fftsize){
	int size=100;
	char tmp[size];tmp[size-1]=0;
	if (fftsize<1024.0) snprintf(tmp,size-1,"%d",fftsize);
	else if (fftsize<(1024.0*1024.0)) snprintf(tmp,size-1,"%.4gK",fftsize/1024.0);
	else if (fftsize<(1024.0*1024.0*1024.0)) snprintf(tmp,size-1,"%.4gM",fftsize/(1024.0*1024.0));
	else snprintf(tmp,size-1,"%.7gG",fftsize/(1024.0*1024.0*1024.0));
	return tmp;
};

void Control::update_process_parameters(){
	if (player) player->set_process_parameters(&ppar,&bbpar);
};

bool Control::save_parameters(const char *filename){
	XMLwrapper *xml=new XMLwrapper();
	
	xml->beginbranch("PAULSTRETCH");
		xml->beginbranch("STRETCH_PARAMETERS");
			xml->beginbranch("BASE");
				xml->addpar("bufsize",process.bufsize);
				xml->addparreal("stretch",process.stretch);
	
				xml->addparreal("fftsize_s",gui_sliders.fftsize_s);
				xml->addparreal("stretch_s",gui_sliders.stretch_s);
				xml->addpar("mode_s",gui_sliders.mode_s);
				
				xml->addpar("window_type",window_type);
				xml->addparreal("volume",volume);
				xml->addparreal("onset_detection_sensitivity",process.onset_detection_sensitivity);

			xml->endbranch();
			
			xml->beginbranch("PROCESS");
				ppar.add2XML(xml);
			xml->endbranch();

			xml->beginbranch("BINAURAL_BEATS");
				bbpar.add2XML(xml);
			xml->endbranch();

		xml->endbranch();
	xml->endbranch();

	int result=xml->saveXMLfile(filename);
	delete xml;
	return true;
};

bool Control::load_parameters(const char *filename){
	XMLwrapper *xml=new XMLwrapper();

	if (xml->loadXMLfile(filename)<0) {
		delete xml;
		return false;
	};


	if (xml->enterbranch("PAULSTRETCH")==0) {
		delete xml;
		return false;
	};

	if (xml->enterbranch("STRETCH_PARAMETERS")){
		if (xml->enterbranch("BASE")){
			process.bufsize=xml->getpar("bufsize",process.bufsize,16,2e9);
			process.stretch=xml->getparreal("stretch",process.stretch);
			gui_sliders.fftsize_s=xml->getparreal("fftsize_s",gui_sliders.fftsize_s);
			gui_sliders.stretch_s=xml->getparreal("stretch_s",gui_sliders.stretch_s);
			gui_sliders.mode_s=xml->getpar("mode_s",gui_sliders.mode_s,0,2);
			window_type=(FFTWindow)xml->getpar("window_type",window_type,0,4);
			process.onset_detection_sensitivity=xml->getparreal("onset_detection_sensitivity",0.0);
			volume=xml->getparreal("volume",1.0);
			xml->exitbranch();
		};

		if (xml->enterbranch("PROCESS")){
				ppar.getfromXML(xml);
			xml->exitbranch();
		};

		if (xml->enterbranch("BINAURAL_BEATS")){
				bbpar.getfromXML(xml);
			xml->exitbranch();
		};

		xml->exitbranch();
	};
	delete xml;

	set_stretch_controls(gui_sliders.stretch_s,gui_sliders.mode_s,gui_sliders.fftsize_s,process.onset_detection_sensitivity);
	
	set_window_type(window_type);
	set_volume(volume);
	update_process_parameters();
	return true;
};
	

