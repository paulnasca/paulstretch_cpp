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


Stretch::Stretch(REALTYPE rap_,int in_bufsize_,FFTWindow w,bool bypass_,REALTYPE samplerate_,int stereo_mode_){
	samplerate=samplerate_;
	rap=rap_;
	in_bufsize=in_bufsize_;
	bypass=bypass_;
	stereo_mode=stereo_mode_;
	if (rap>=1.0){//stretch
		out_bufsize=in_bufsize;
	}else{
		//shorten
		out_bufsize=(int)(in_bufsize*rap);
	};
	if (out_bufsize<8) out_bufsize=8;

	if (bypass) out_bufsize=in_bufsize;

	out_buf=new REALTYPE[out_bufsize];
	old_out_smp_buf=new REALTYPE[out_bufsize*2];for (int i=0;i<out_bufsize*2;i++) old_out_smp_buf[i]=0.0;

	poolsize=in_bufsize_*2;
	in_pool=new REALTYPE[poolsize];for (int i=0;i<poolsize;i++) in_pool[i]=0.0;

	infft=new FFT(poolsize);
	outfft=new FFT(out_bufsize*2);
	remained_samples=0.0;
	window_type=w;
};

Stretch::~Stretch(){
	delete [] out_buf;
	delete [] old_out_smp_buf;
	delete [] in_pool;
	delete infft;
	delete outfft;
};

void Stretch::set_rap(REALTYPE newrap){
	if ((rap>=1.0)&&(newrap>=1.0)) rap=newrap;
};

void Stretch::process(REALTYPE *smps,int nsmps){
	if (bypass){
		for (int i=0;i<out_bufsize;i++) out_buf[i]=smps[i];
		//post-process the output
		//	process_output(out_buf,out_bufsize);
		return;
	};
	//add new samples to the pool
	if ((smps!=NULL)&&(nsmps!=0)){
		if (nsmps>poolsize){
			printf("Warning nsmps> inbufsize on Stretch::process() %d>%d\n",nsmps,poolsize);
			nsmps=poolsize;
		};
		int nleft=poolsize-nsmps;

		//move left the samples from the pool to make room for new samples
		for (int i=0;i<nleft;i++) in_pool[i]=in_pool[i+nsmps];

		//add new samples to the pool
		for (int i=0;i<nsmps;i++) in_pool[i+nleft]=smps[i];	
	};

	//get the samples from the pool
	for (int i=0;i<poolsize;i++) infft->smp[i]=in_pool[i];


	infft->applywindow(window_type);
	infft->smp2freq();


	if (out_bufsize==in_bufsize){//output is the same as the input (as usual)
		for (int i=0;i<in_bufsize;i++) outfft->freq[i]=infft->freq[i];
	} else {
		if (out_bufsize>in_bufsize){//output is longer
			REALTYPE rap=(REALTYPE)in_bufsize/(REALTYPE)out_bufsize;
			for (int i=0;i<out_bufsize;i++) {
				REALTYPE pos=i*rap;
				int poshi=(int)floor(pos);
				REALTYPE poslo=pos-floor(pos);
				outfft->freq[i]=infft->freq[poshi]*(1.0-poslo)+infft->freq[poshi+1]*poslo;
			};
		}else{//output is shorter
			for (int i=0;i<out_bufsize;i++) outfft->freq[i]=0.0;
			REALTYPE rap=(REALTYPE)out_bufsize/(REALTYPE)in_bufsize;
			for (int i=0;i<in_bufsize;i++) {
				REALTYPE pos=i*rap;
				int poshi=(int)(floor(pos));
				//		#warning sa folosesc si poslo
				outfft->freq[poshi]+=infft->freq[i];
			};
		};
	};

	process_spectrum(outfft->freq);

	outfft->freq2smp();

	//make the output buffer
	REALTYPE tmp=1.0/(float) out_bufsize*M_PI;
	REALTYPE hinv_sqrt2=0.853553390593;//(1.0+1.0/sqrt(2))*0.5;

	REALTYPE ampfactor=1.0;
	if (rap<1.0) ampfactor=rap*0.707;
	else ampfactor=(out_bufsize/(float)poolsize)*4.0;

	for (int i=0;i<out_bufsize;i++) {
		REALTYPE a=(0.5+0.5*cos(i*tmp));
		REALTYPE out=outfft->smp[i+out_bufsize]*(1.0-a)+old_out_smp_buf[i]*a;
		out_buf[i]=out*(hinv_sqrt2-(1.0-hinv_sqrt2)*cos(i*2.0*tmp))*ampfactor;
	};

	//copy the current output buffer to old buffer
	for (int i=0;i<out_bufsize*2;i++) old_out_smp_buf[i]=outfft->smp[i];

	//post-process the output
	//process_output(out_buf,out_bufsize);
};


int Stretch::get_nsamples(REALTYPE current_pos_percents){
	if (bypass) return out_bufsize;
	if (rap<1.0) return poolsize/2;//pentru shorten


	long double used_rap=rap*get_stretch_multiplier(current_pos_percents);	

	long double r=out_bufsize/used_rap;
	int ri=(int)floor(r);
	long double rf=r-floor(r);

	long double old_remained_samples_test=remained_samples;
	remained_samples+=rf;
	if (remained_samples>=1.0){
		ri+=(int)floor(remained_samples);
		remained_samples=remained_samples-floor(remained_samples);
	};

	long double rf_test=remained_samples-old_remained_samples_test;//this value should be almost like "rf" (for most of the time with the exception of changing the "ri" value) for extremely long stretches (otherwise the shown stretch value is not accurate)
	//for stretch up to 10^18x "long double" must have at least 64 bits in the fraction part (true for gcc compiler on x86 and macosx)

//	long double zzz=1.0;//quick test by adding a "largish" number and substracting it again
//	rf_test+=zzz;
//	rf_test-=zzz;

//	printf("remained_samples=%.20Lg rf=%.20Lg  rf_test=%.20Lg\n",remained_samples,rf,rf_test);
//	printf("rf=%g  rf_test=%g\n",(double)rf,(double)(rf_test));

	if (ri>poolsize){
		ri=poolsize;
	};

	return ri;
};

int Stretch::get_nsamples_for_fill(){
	return poolsize;
};

REALTYPE Stretch::get_stretch_multiplier(REALTYPE pos_percents){
	return 1.0;
};

