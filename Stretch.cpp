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

#include "Stretch.h"
#include <stdlib.h>
#include <math.h>

unsigned int FFT::start_rand_seed=1;

FFT::FFT(int nsamples_){
	nsamples=nsamples_;
	if (nsamples%2!=0) {
		nsamples+=1;
		printf("WARNING: Odd sample size on FFT::FFT() (%d)",nsamples);
	};
	smp=new REALTYPE[nsamples];for (int i=0;i<nsamples;i++) smp[i]=0.0;
	freq=new REALTYPE[nsamples/2+1];for (int i=0;i<nsamples/2+1;i++) freq[i]=0.0;
	window.data=new REALTYPE[nsamples];for (int i=0;i<nsamples;i++) window.data[i]=0.707;
	window.type=W_RECTANGULAR;

#ifdef KISSFFT
	datar=new kiss_fft_scalar[nsamples+2];
	for (int i=0;i<nsamples+2;i++) datar[i]=0.0;
	datac=new kiss_fft_cpx[nsamples/2+2];
	for (int i=0;i<nsamples/2+2;i++) datac[i].r=datac[i].i=0.0;
	plankfft = kiss_fftr_alloc(nsamples,0,0,0);
	plankifft = kiss_fftr_alloc(nsamples,1,0,0);
#else
	data=new REALTYPE[nsamples];for (int i=0;i<nsamples;i++) data[i]=0.0;
	planfftw=fftwf_plan_r2r_1d(nsamples,data,data,FFTW_R2HC,FFTW_ESTIMATE);
	planifftw=fftwf_plan_r2r_1d(nsamples,data,data,FFTW_HC2R,FFTW_ESTIMATE);
#endif
	rand_seed=start_rand_seed;
	start_rand_seed+=161103;
};

FFT::~FFT(){
	delete []smp;
	delete []freq;
	delete []window.data;
#ifdef KISSFFT
	delete []datar;
	delete []datac;
	free(plankfft);
	free(plankifft);
#else
	delete []data;
	fftwf_destroy_plan(planfftw);
	fftwf_destroy_plan(planifftw);
#endif
};

void FFT::smp2freq(){
#ifdef KISSFFT
	for (int i=0;i<nsamples;i++) datar[i]=smp[i];
	kiss_fftr(plankfft,datar,datac);
#else
	for (int i=0;i<nsamples;i++) data[i]=smp[i];
	fftwf_execute(planfftw);
#endif

	for (int i=1;i<nsamples/2;i++) {
#ifdef KISSFFT
		REALTYPE c=datac[i].r;
		REALTYPE s=datac[i].i;
#else
		REALTYPE c=data[i];
		REALTYPE s=data[nsamples-i];
#endif
		freq[i]=sqrt(c*c+s*s);
	};
	freq[0]=0.0;
};

void FFT::freq2smp(){
	REALTYPE inv_2p15_2pi=1.0/16384.0*M_PI;
	for (int i=1;i<nsamples/2;i++) {
		rand_seed=(rand_seed*1103515245+12345);
		unsigned int rand=(rand_seed>>16)&0x7fff;
		REALTYPE phase=rand*inv_2p15_2pi;
#ifdef KISSFFT
		datac[i].r=freq[i]*cos(phase);
		datac[i].i=freq[i]*sin(phase);
#else
		data[i]=freq[i]*cos(phase);
		data[nsamples-i]=freq[i]*sin(phase);
#endif
	};

#ifdef KISSFFT
	datac[0].r=datac[0].i=0.0;
	kiss_fftri(plankifft,datac,datar);
	for (int i=0;i<nsamples;i++) smp[i]=datar[i]/nsamples;
#else
	data[0]=data[nsamples/2+1]=data[nsamples/2]=0.0;
	fftwf_execute(planifftw);
	for (int i=0;i<nsamples;i++) smp[i]=data[i]/nsamples;
#endif
};

void FFT::applywindow(FFTWindow type){
	if (window.type!=type){
		window.type=type;
		switch (type){
			case W_RECTANGULAR:
				for (int i=0;i<nsamples;i++) window.data[i]=0.707;
				break;
			case W_HAMMING:
				for (int i=0;i<nsamples;i++) window.data[i]=0.53836-0.46164*cos(2*M_PI*i/(nsamples+1.0));
				break;
			case W_HANN:
				for (int i=0;i<nsamples;i++) window.data[i]=0.5*(1.0-cos(2*M_PI*i/(nsamples-1.0)));
				break;
			case W_BLACKMAN:
				for (int i=0;i<nsamples;i++) window.data[i]=0.42-0.5*cos(2*M_PI*i/(nsamples-1.0))+0.08*cos(4*M_PI*i/(nsamples-1.0));
				break;
			case W_BLACKMAN_HARRIS:
				for (int i=0;i<nsamples;i++) window.data[i]=0.35875-0.48829*cos(2*M_PI*i/(nsamples-1.0))+0.14128*cos(4*M_PI*i/(nsamples-1.0))-0.01168*cos(6*M_PI*i/(nsamples-1.0));
				break;

		};
	};
	for (int i=0;i<nsamples;i++) smp[i]*=window.data[i];
};

/*******************************************/


Stretch::Stretch(REALTYPE rap_,int bufsize_,FFTWindow w,bool bypass_,REALTYPE samplerate_,int stereo_mode_){
	freezing=false;
	onset_detection_sensitivity=0.0;

	samplerate=samplerate_;
	rap=rap_;
	bufsize=bufsize_;
	bypass=bypass_;
	stereo_mode=stereo_mode_;
	if (bufsize<8) bufsize=8;

	out_buf=new REALTYPE[bufsize];
	old_freq=new REALTYPE[bufsize];

	very_old_smps=new REALTYPE[bufsize];
	new_smps=new REALTYPE[bufsize];
	old_smps=new REALTYPE[bufsize];

	old_out_smps=new REALTYPE[bufsize*2];
	for (int i=0;i<bufsize*2;i++) {
		old_out_smps[i]=0.0;
	};
	for (int i=0;i<bufsize;i++) {
		old_freq[i]=0.0;
		new_smps[i]=0.0;
		old_smps[i]=0.0;
		very_old_smps[i]=0.0;
	};


	infft=new FFT(bufsize*2);
	fft=new FFT(bufsize*2);
	outfft=new FFT(bufsize*2);
	remained_samples=0.0;
	window_type=w;
	require_new_buffer=false;
	c_pos_percents=0.0;
	extra_onset_time_credit=0.0;
	skip_samples=0;
};

Stretch::~Stretch(){
	delete [] old_freq;
	delete [] out_buf;
	delete [] new_smps;
	delete [] old_smps;
	delete [] very_old_smps;
	delete [] old_out_smps;
	delete fft;
	delete infft;
	delete outfft;
};

void Stretch::set_rap(REALTYPE newrap){
	//if ((rap>=1.0)&&(newrap>=1.0)) 
	rap=newrap;
};
		
void Stretch::do_analyse_inbuf(REALTYPE *smps){
	//get the frequencies
	for (int i=0;i<bufsize;i++) {
		infft->smp[i]=old_smps[i];
		infft->smp[i+bufsize]=smps[i];

		old_freq[i]=infft->freq[i];
	};
	infft->applywindow(window_type);
	infft->smp2freq();
};

void Stretch::do_next_inbuf_smps(REALTYPE *smps){
	for (int i=0;i<bufsize;i++) {
		very_old_smps[i]=old_smps[i];
		old_smps[i]=new_smps[i];
		new_smps[i]=smps[i];
	};
};

REALTYPE Stretch::do_detect_onset(){
	REALTYPE result=0.0;
	if (onset_detection_sensitivity>1e-3){
		REALTYPE os=0.0,osinc=0.0;
		REALTYPE osincold=1e-5;
		int maxk=1+(int)(bufsize*500.0/(samplerate*0.5));
		int k=0;
		for (int i=0;i<bufsize;i++) {
			osinc+=infft->freq[i]-old_freq[i];
			osincold+=old_freq[i];
			if (k>=maxk) {
				k=0;
				os+=osinc/osincold;
				osinc=0;
			};
			k++;
		};
		os+=osinc;
		if (os<0.0) os=0.0;
		//if (os>1.0) os=1.0;

		REALTYPE os_strength=pow(20.0,1.0-onset_detection_sensitivity)-1.0;
		REALTYPE os_strength_h=os_strength*0.75;
		if (os>os_strength_h){
			result=(os-os_strength_h)/(os_strength-os_strength_h);
			if (result>1.0) result=1.0;
		};

		if (result>1.0) result=1.0;
	};
	return result;
};

REALTYPE Stretch::process(REALTYPE *smps,int nsmps){
	REALTYPE onset=0.0;
	if (bypass){
		for (int i=0;i<bufsize;i++) out_buf[i]=smps[i];
		return 0.0;
	};

	if (smps!=NULL){
		if ((nsmps!=0)&&(nsmps!=bufsize)&&(nsmps!=get_max_bufsize())){
			printf("Warning wrong nsmps on Stretch::process() %d,%d\n",nsmps,bufsize);
			return 0.0;
		};
		if (nsmps!=0){//new data arrived: update the frequency components
			do_analyse_inbuf(smps);		
			if (nsmps==get_max_bufsize()) {
				for (int k=bufsize;k<get_max_bufsize();k+=bufsize) do_analyse_inbuf(smps+k);
			};
			if (onset_detection_sensitivity>1e-3) onset=do_detect_onset();
		};


		//move the buffers	
		if (nsmps!=0){//new data arrived: update the frequency components
			do_next_inbuf_smps(smps);		
			if (nsmps==get_max_bufsize()) {
				for (int k=bufsize;k<get_max_bufsize();k+=bufsize) do_next_inbuf_smps(smps+k);

			};
		};
	
		//construct the input fft
		int start_pos=(int)(floor(remained_samples*bufsize));	
		if (start_pos>=bufsize) start_pos=bufsize-1;
		for (int i=0;i<bufsize-start_pos;i++) fft->smp[i]=very_old_smps[i+start_pos];
		for (int i=0;i<bufsize;i++) fft->smp[i+bufsize-start_pos]=old_smps[i];
		for (int i=0;i<start_pos;i++) fft->smp[i+2*bufsize-start_pos]=new_smps[i];
		//compute the output spectrum
		fft->applywindow(window_type);
		fft->smp2freq();
		for (int i=0;i<bufsize;i++) outfft->freq[i]=fft->freq[i];
	


		//for (int i=0;i<bufsize;i++) outfft->freq[i]=infft->freq[i]*remained_samples+old_freq[i]*(1.0-remained_samples);


		process_spectrum(outfft->freq);

		outfft->freq2smp();

		//make the output buffer
		REALTYPE tmp=1.0/(float) bufsize*M_PI;
		REALTYPE hinv_sqrt2=0.853553390593;//(1.0+1.0/sqrt(2))*0.5;

		REALTYPE ampfactor=2.0;
		
		//remove the resulted unwanted amplitude modulation (caused by the interference of N and N+1 windowed buffer and compute the output buffer
		for (int i=0;i<bufsize;i++) {
			REALTYPE a=(0.5+0.5*cos(i*tmp));
			REALTYPE out=outfft->smp[i+bufsize]*(1.0-a)+old_out_smps[i]*a;
			out_buf[i]=out*(hinv_sqrt2-(1.0-hinv_sqrt2)*cos(i*2.0*tmp))*ampfactor;
		};

		//copy the current output buffer to old buffer
		for (int i=0;i<bufsize*2;i++) old_out_smps[i]=outfft->smp[i];

	};

	if (!freezing){
		long double used_rap=rap*get_stretch_multiplier(c_pos_percents);	

		long double r=1.0/used_rap;
		if (extra_onset_time_credit>0){
			REALTYPE credit_get=0.5*r;//must be smaller than r
			extra_onset_time_credit-=credit_get;
			if (extra_onset_time_credit<0.0) extra_onset_time_credit=0.0;
			r-=credit_get;
		};

		long double old_remained_samples_test=remained_samples;
		remained_samples+=r;
		int result=0;
		if (remained_samples>=1.0){
			skip_samples=(int)(floor(remained_samples-1.0)*bufsize);
			remained_samples=remained_samples-floor(remained_samples);
			require_new_buffer=true;
		}else{
			require_new_buffer=false;
		};
	};
//	long double rf_test=remained_samples-old_remained_samples_test;//this value should be almost like "rf" (for most of the time with the exception of changing the "ri" value) for extremely long stretches (otherwise the shown stretch value is not accurate)
	//for stretch up to 10^18x "long double" must have at least 64 bits in the fraction part (true for gcc compiler on x86 and macosx)
	return onset;	
};

void Stretch::here_is_onset(REALTYPE onset){
	if (freezing) return;
	if (onset>0.5){
		require_new_buffer=true;
		extra_onset_time_credit+=1.0-remained_samples;
		remained_samples=0.0;
		skip_samples=0;
	};
};

int Stretch::get_nsamples(REALTYPE current_pos_percents){
	if (bypass) return bufsize;
	if (freezing) return 0;
	c_pos_percents=current_pos_percents;
	return require_new_buffer?bufsize:0;
};

int Stretch::get_nsamples_for_fill(){
	return get_max_bufsize();
};

int Stretch::get_skip_nsamples(){
	if (freezing||bypass) return 0;
	return skip_samples;
};

REALTYPE Stretch::get_stretch_multiplier(REALTYPE pos_percents){
	return 1.0;
};

