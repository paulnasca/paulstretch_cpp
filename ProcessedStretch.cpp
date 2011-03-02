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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "ProcessedStretch.h"

void ProcessParameters::add2XML(XMLwrapper *xml){

	xml->addparbool("pitch_shift.enabled",pitch_shift.enabled);
	xml->addparreal("pitch_shift.cents",pitch_shift.cents);

	xml->addparbool("octave.enabled",octave.enabled);
	xml->addparreal("octave.om2",octave.om2);
	xml->addparreal("octave.om1",octave.om1);
	xml->addparreal("octave.o0",octave.o0);
	xml->addparreal("octave.o1",octave.o1);
	xml->addparreal("octave.o15",octave.o15);
	xml->addparreal("octave.o2",octave.o2);

	xml->addparbool("freq_shift.enabled",freq_shift.enabled);
	xml->addparreal("freq_shift.Hz",freq_shift.Hz);

	xml->addparbool("compressor.enabled",compressor.enabled);
	xml->addparreal("compressor.power",compressor.power);

	xml->addparbool("filter.enabled",filter.enabled);
	xml->addparbool("filter.stop",filter.stop);
	xml->addparreal("filter.low",filter.low);
	xml->addparreal("filter.high",filter.high);
	xml->addparreal("filter.hdamp",filter.hdamp);

	xml->addparbool("harmonics.enabled",harmonics.enabled);
	xml->addparreal("harmonics.freq",harmonics.freq);
	xml->addparreal("harmonics.bandwidth",harmonics.bandwidth);
	xml->addpar("harmonic.nharmonics",harmonics.nharmonics);

	xml->addparbool("harmonics.gauss",harmonics.gauss);

	xml->addparbool("spread.enabled",spread.enabled);
	xml->addparreal("spread.bandwidth",spread.bandwidth);
	
	xml->addparbool("tonal_vs_noise.enabled",tonal_vs_noise.enabled);
	xml->addparreal("tonal_vs_noise.bandwidth",tonal_vs_noise.bandwidth);
	xml->addparreal("tonal_vs_noise.preserve",tonal_vs_noise.preserve);

	xml->beginbranch("FREE_FILTER");
	free_filter.add2XML(xml);
	xml->endbranch();

	xml->beginbranch("STRETCH_MULTIPLIER");
	stretch_multiplier.add2XML(xml);
	xml->endbranch();

};

void ProcessParameters::getfromXML(XMLwrapper *xml){
	pitch_shift.enabled=xml->getparbool("pitch_shift.enabled",pitch_shift.enabled);
	pitch_shift.cents=xml->getparreal("pitch_shift.cents",pitch_shift.cents);

	octave.enabled=xml->getparbool("octave.enabled",octave.enabled);
	octave.om2=xml->getparreal("octave.om2",octave.om2);
	octave.om1=xml->getparreal("octave.om1",octave.om1);
	octave.o0=xml->getparreal("octave.o0",octave.o0);
	octave.o1=xml->getparreal("octave.o1",octave.o1);
	octave.o15=xml->getparreal("octave.o15",octave.o15);
	octave.o2=xml->getparreal("octave.o2",octave.o2);

	freq_shift.enabled=xml->getparbool("freq_shift.enabled",freq_shift.enabled);
	freq_shift.Hz=xml->getparreal("freq_shift.Hz",freq_shift.Hz);

	compressor.enabled=xml->getparbool("compressor.enabled",compressor.enabled);
	compressor.power=xml->getparreal("compressor.power",compressor.power);

	filter.enabled=xml->getparbool("filter.enabled",filter.enabled);
	filter.stop=xml->getparbool("filter.stop",filter.stop);
	filter.low=xml->getparreal("filter.low",filter.low);
	filter.high=xml->getparreal("filter.high",filter.high);
	filter.hdamp=xml->getparreal("filter.hdamp",filter.hdamp);

	harmonics.enabled=xml->getparbool("harmonics.enabled",harmonics.enabled);
	harmonics.freq=xml->getparreal("harmonics.freq",harmonics.freq);
	harmonics.bandwidth=xml->getparreal("harmonics.bandwidth",harmonics.bandwidth);
	harmonics.nharmonics=xml->getpar("harmonic.nharmonics",harmonics.nharmonics,1,100);

	harmonics.gauss=xml->getparbool("harmonics.gauss",harmonics.gauss);

	spread.enabled=xml->getparbool("spread.enabled",spread.enabled);
	spread.bandwidth=xml->getparreal("spread.bandwidth",spread.bandwidth);
	
	tonal_vs_noise.enabled=xml->getparbool("tonal_vs_noise.enabled",tonal_vs_noise.enabled);
	tonal_vs_noise.preserve=xml->getparreal("tonal_vs_noise.preserve",tonal_vs_noise.bandwidth);
	tonal_vs_noise.bandwidth=xml->getparreal("tonal_vs_noise.bandwidth",tonal_vs_noise.bandwidth);


	if (xml->enterbranch("FREE_FILTER")){
		free_filter.getfromXML(xml);
		xml->exitbranch();
	};
	if (xml->enterbranch("STRETCH_MULTIPLIER")){
		stretch_multiplier.getfromXML(xml);
		xml->exitbranch();
	};
};

ProcessedStretch::ProcessedStretch(REALTYPE rap_,int in_bufsize_,FFTWindow w,bool bypass_,REALTYPE samplerate_,int stereo_mode_):Stretch(rap_,in_bufsize_,w,bypass_,samplerate_,stereo_mode_){
	nfreq=bufsize;
	infreq=new REALTYPE[nfreq];
	sumfreq=new REALTYPE[nfreq];
	tmpfreq1=new REALTYPE[nfreq];
	tmpfreq2=new REALTYPE[nfreq];
	//fbfreq=new REALTYPE[nfreq];
	free_filter_freqs=new REALTYPE[nfreq];
	for (int i=0;i<nfreq;i++) {
		free_filter_freqs[i]=1.0;
	//	fbfreq[i]=0.0;
	};
};
ProcessedStretch::~ProcessedStretch(){
	delete [] infreq;
	delete [] sumfreq;
	delete [] tmpfreq1;
	delete [] tmpfreq2;
	delete [] free_filter_freqs;
//	delete [] fbfreq;
};

void ProcessedStretch::set_parameters(ProcessParameters *ppar){
	pars=*ppar;
	update_free_filter();
};


void ProcessedStretch::copy(REALTYPE *freq1,REALTYPE *freq2){
	for (int i=0;i<nfreq;i++) freq2[i]=freq1[i];
};

void ProcessedStretch::add(REALTYPE *freq2,REALTYPE *freq1,REALTYPE a){
	for (int i=0;i<nfreq;i++) freq2[i]+=freq1[i]*a;
};

void ProcessedStretch::mul(REALTYPE *freq1,REALTYPE a){
	for (int i=0;i<nfreq;i++) freq1[i]*=a;
};

void ProcessedStretch::zero(REALTYPE *freq1){
	for (int i=0;i<nfreq;i++) freq1[i]=0.0;
};

REALTYPE ProcessedStretch::get_stretch_multiplier(REALTYPE pos_percents){
	REALTYPE result=1.0;
	if (pars.stretch_multiplier.get_enabled()){
		result*=pars.stretch_multiplier.get_value(pos_percents);
	};
	///REALTYPE transient=pars.get_transient(pos_percents);
	///printf("\n%g\n",transient);
	///REALTYPE threshold=0.05;
	///REALTYPE power=1000.0;
	///transient-=threshold;
	///if (transient>0){
	///	transient*=power*(1.0+power);
	///	result/=(1.0+transient);
	///};
	///printf("tr=%g\n",result);
	return result;
};

void ProcessedStretch::process_spectrum(REALTYPE *freq){
	if (pars.harmonics.enabled) {
		copy(freq,infreq);
		do_harmonics(infreq,freq);
	};

	if (pars.tonal_vs_noise.enabled){
		copy(freq,infreq);
		do_tonal_vs_noise(infreq,freq);
	};

	if (pars.freq_shift.enabled) {
		copy(freq,infreq);
		do_freq_shift(infreq,freq);
	};
	if (pars.pitch_shift.enabled) {
		copy(freq,infreq);
		do_pitch_shift(infreq,freq,pow(2.0,pars.pitch_shift.cents/1200.0));
	};
	if (pars.octave.enabled){
		copy(freq,infreq);
		do_octave(infreq,freq);
	};


	if (pars.spread.enabled){
		copy(freq,infreq);
		do_spread(infreq,freq);
	};


	if (pars.filter.enabled){
		copy(freq,infreq);
		do_filter(infreq,freq);
	};
	
	if (pars.free_filter.get_enabled()){
		copy(freq,infreq);
		do_free_filter(infreq,freq);
	};

	if (pars.compressor.enabled){
		copy(freq,infreq);
		do_compressor(infreq,freq);
	};

};

//void ProcessedStretch::process_output(REALTYPE *smps,int nsmps){
//};


REALTYPE profile(REALTYPE fi, REALTYPE bwi){
	REALTYPE x=fi/bwi;
	x*=x;
	if (x>14.71280603) return 0.0;
	return exp(-x);///bwi;

};

void ProcessedStretch::do_harmonics(REALTYPE *freq1,REALTYPE *freq2){
	REALTYPE freq=pars.harmonics.freq;
	REALTYPE bandwidth=pars.harmonics.bandwidth;
	int nharmonics=pars.harmonics.nharmonics;

	if (freq<10.0) freq=10.0;

	REALTYPE *amp=tmpfreq1;
	for (int i=0;i<nfreq;i++) amp[i]=0.0;

	for (int nh=1;nh<=nharmonics;nh++){//for each harmonic
		REALTYPE bw_Hz;//bandwidth of the current harmonic measured in Hz
		REALTYPE bwi;
		REALTYPE fi;
		REALTYPE f=nh*freq;

		if (f>=samplerate/2) break;

		bw_Hz=(pow(2.0,bandwidth/1200.0)-1.0)*f;
		bwi=bw_Hz/(2.0*samplerate);
		fi=f/samplerate;

		REALTYPE sum=0.0;
		REALTYPE max=0.0;
		for (int i=1;i<nfreq;i++){//todo: optimize here
			REALTYPE hprofile;
			hprofile=profile((i/(REALTYPE)nfreq*0.5)-fi,bwi);
			amp[i]+=hprofile;
			if (max<hprofile) max=hprofile;
			sum+=hprofile;
		};
	};

	REALTYPE max=0.0;
	for (int i=1;i<nfreq;i++){
		if (amp[i]>max) max=amp[i];
	};
	if (max<1e-8) max=1e-8;

	for (int i=1;i<nfreq;i++){
		REALTYPE c,s;
		REALTYPE a=amp[i]/max;
		if (!pars.harmonics.gauss) a=(a<0.368?0.0:1.0);
		freq2[i]=freq1[i]*a;
	};

};


void ProcessedStretch::do_freq_shift(REALTYPE *freq1,REALTYPE *freq2){
	zero(freq2);
	int ifreq=(int)(pars.freq_shift.Hz/(samplerate*0.5)*nfreq);
	for (int i=0;i<nfreq;i++){
		int i2=ifreq+i;
		if ((i2>0)&&(i2<nfreq)) freq2[i2]=freq1[i];
	};
};
void ProcessedStretch::do_pitch_shift(REALTYPE *freq1,REALTYPE *freq2,REALTYPE rap){
	zero(freq2);
	if (rap<1.0){//down
		for (int i=0;i<nfreq;i++){
			int i2=(int)(i*rap);
			if (i2>=nfreq) break;
			freq2[i2]+=freq1[i];
		};
	};
	if (rap>=1.0){//up
		rap=1.0/rap;
		for (int i=0;i<nfreq;i++){
			freq2[i]=freq1[(int)(i*rap)];
		};

	};
};
void ProcessedStretch::do_octave(REALTYPE *freq1,REALTYPE *freq2){
	zero(sumfreq);
	if (pars.octave.om2>1e-3){
		do_pitch_shift(freq1,tmpfreq1,0.25);
		add(sumfreq,tmpfreq1,pars.octave.om2);
	};
	if (pars.octave.om1>1e-3){
		do_pitch_shift(freq1,tmpfreq1,0.5);
		add(sumfreq,tmpfreq1,pars.octave.om1);
	};
	if (pars.octave.o0>1e-3){
		add(sumfreq,freq1,pars.octave.o0);
	};
	if (pars.octave.o1>1e-3){
		do_pitch_shift(freq1,tmpfreq1,2.0);
		add(sumfreq,tmpfreq1,pars.octave.o1);
	};
	if (pars.octave.o15>1e-3){
		do_pitch_shift(freq1,tmpfreq1,3.0);
		add(sumfreq,tmpfreq1,pars.octave.o15);
	};
	if (pars.octave.o2>1e-3){
		do_pitch_shift(freq1,tmpfreq1,4.0);
		add(sumfreq,tmpfreq1,pars.octave.o2);
	};

	REALTYPE sum=0.01+pars.octave.om2+pars.octave.om1+pars.octave.o0+pars.octave.o1+pars.octave.o15+pars.octave.o2;
	if (sum<0.5) sum=0.5;
	for (int i=0;i<nfreq;i++) freq2[i]=sumfreq[i]/sum;    
};

void ProcessedStretch::do_filter(REALTYPE *freq1,REALTYPE *freq2){
	REALTYPE low=0,high=0;
	if (pars.filter.low<pars.filter.high){//sort the low/high freqs
		low=pars.filter.low;
		high=pars.filter.high;
	}else{
		high=pars.filter.low;
		low=pars.filter.high;
	};
	int ilow=(int) (low/samplerate*nfreq*2.0);
	int ihigh=(int) (high/samplerate*nfreq*2.0);
	REALTYPE dmp=1.0;
	REALTYPE dmprap=1.0-pow(pars.filter.hdamp*0.5,4.0);
	for (int i=0;i<nfreq;i++){
		REALTYPE a=0.0;
		if ((i>=ilow)&&(i<ihigh)) a=1.0;
		if (pars.filter.stop) a=1.0-a;
		freq2[i]=freq1[i]*a*dmp;
		dmp*=dmprap+1e-8;
	};
};

void ProcessedStretch::update_free_filter(){
	pars.free_filter.update_curve();
	if (pars.free_filter.get_enabled()) {
		for (int i=0;i<nfreq;i++){
			REALTYPE freq=(REALTYPE)i/(REALTYPE) nfreq*samplerate*0.5;
			free_filter_freqs[i]=pars.free_filter.get_value(freq);
		};
	}else{
		for (int i=0;i<nfreq;i++){
			free_filter_freqs[i]=1.0;
		};
	};
};
void ProcessedStretch::do_free_filter(REALTYPE *freq1,REALTYPE *freq2){
	for (int i=0;i<nfreq;i++){
		freq2[i]=freq1[i]*free_filter_freqs[i];
	};
};

void ProcessedStretch::do_spread(REALTYPE *freq1,REALTYPE *freq2){
	spread(freq1,freq2,pars.spread.bandwidth);
};

void ProcessedStretch::spread(REALTYPE *freq1,REALTYPE *freq2,REALTYPE spread_bandwidth){
	//convert to log spectrum
	REALTYPE minfreq=20.0;
	REALTYPE maxfreq=0.5*samplerate;

	REALTYPE log_minfreq=log(minfreq);
	REALTYPE log_maxfreq=log(maxfreq);
		
	for (int i=0;i<nfreq;i++){
		REALTYPE freqx=i/(REALTYPE) nfreq;
		REALTYPE x=exp(log_minfreq+freqx*(log_maxfreq-log_minfreq))/maxfreq*nfreq;
		REALTYPE y=0.0;
		int x0=(int)floor(x); if (x0>=nfreq) x0=nfreq-1;
		int x1=x0+1; if (x1>=nfreq) x1=nfreq-1;
		REALTYPE xp=x-x0;
		if (x<nfreq){
			y=freq1[x0]*(1.0-xp)+freq1[x1]*xp;
		};
		tmpfreq1[i]=y;
	};

	//increase the bandwidth of each harmonic (by smoothing the log spectrum)
	int n=2;
	REALTYPE bandwidth=spread_bandwidth;
	REALTYPE a=1.0-pow(2.0,-bandwidth*bandwidth*10.0);
	a=pow(a,8192.0/nfreq*n);

	for (int k=0;k<n;k++){                                                  
		tmpfreq1[0]=0.0;
		for (int i=1;i<nfreq;i++){                                       
			tmpfreq1[i]=tmpfreq1[i-1]*a+tmpfreq1[i]*(1.0-a);
		};                                                              
		tmpfreq1[nfreq-1]=0.0;                                               
		for (int i=nfreq-2;i>0;i--){                                     
			tmpfreq1[i]=tmpfreq1[i+1]*a+tmpfreq1[i]*(1.0-a);                    
		};                                                              
	};                                                                      

	freq2[0]=0;
	REALTYPE log_maxfreq_d_minfreq=log(maxfreq/minfreq);
	for (int i=1;i<nfreq;i++){
		REALTYPE freqx=i/(REALTYPE) nfreq;
		REALTYPE x=log((freqx*maxfreq)/minfreq)/log_maxfreq_d_minfreq*nfreq;
		REALTYPE y=0.0;
		if ((x>0.0)&&(x<nfreq)){
			int x0=(int)floor(x); if (x0>=nfreq) x0=nfreq-1;
			int x1=x0+1; if (x1>=nfreq) x1=nfreq-1;
			REALTYPE xp=x-x0;
			y=tmpfreq1[x0]*(1.0-xp)+tmpfreq1[x1]*xp;
		};
		freq2[i]=y;
	};


};


void ProcessedStretch::do_compressor(REALTYPE *freq1,REALTYPE *freq2){
	REALTYPE rms=0.0;
	for (int i=0;i<nfreq;i++) rms+=freq1[i]*freq1[i];
	rms=sqrt(rms/nfreq)*0.1;
	if (rms<1e-3) rms=1e-3;

	REALTYPE rap=pow(rms,-pars.compressor.power);
	for (int i=0;i<nfreq;i++) freq2[i]=freq1[i]*rap;
};

void ProcessedStretch::do_tonal_vs_noise(REALTYPE *freq1,REALTYPE *freq2){
	spread(freq1,tmpfreq1,pars.tonal_vs_noise.bandwidth);

	if (pars.tonal_vs_noise.preserve>=0.0){
		REALTYPE mul=(pow(10.0,pars.tonal_vs_noise.preserve)-1.0);
		for (int i=0;i<nfreq;i++) {
			REALTYPE x=freq1[i];
			REALTYPE smooth_x=tmpfreq1[i]+1e-6;

			REALTYPE result=0.0;
			result=x-smooth_x*mul;
			if (result<0.0) result=0.0;
			freq2[i]=result;
		};
	}else{
		REALTYPE mul=(pow(5.0,1.0+pars.tonal_vs_noise.preserve)-1.0);
		for (int i=0;i<nfreq;i++) {
			REALTYPE x=freq1[i];
			REALTYPE smooth_x=tmpfreq1[i]+1e-6;

			REALTYPE result=0.0;
			result=x-smooth_x*mul+0.1*mul;
			if (result<0.0) result=x;
			else result=0.0;

			freq2[i]=result;
		};
	};

};

