/*
  Copyright (C) 2011 Nasca Octavian Paul
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


#ifndef PROCESSED_STRETCH_H
#define PROCESSED_STRETCH_H

#include "FreeEdit.h"
#include "Stretch.h"
#include "XMLwrapper.h"


struct ProcessParameters{
	ProcessParameters(){
		pitch_shift.enabled=false;
		pitch_shift.cents=0;

		octave.enabled=false;
		octave.om2=octave.om1=octave.o1=octave.o15=octave.o2=0;
		octave.o0=1.0;

		freq_shift.enabled=false;
		freq_shift.Hz=0;

		compressor.enabled=false;
		compressor.power=0.0;

		filter.enabled=false;
		filter.stop=false;
		filter.low=0.0;
		filter.high=22000.0;
		filter.hdamp=0.0;

		harmonics.enabled=false;
		harmonics.freq=440.0;
		harmonics.bandwidth=25.0;
		harmonics.nharmonics=10;
		harmonics.gauss=false;

		spread.enabled=false;
		spread.bandwidth=0.3;
		
		tonal_vs_noise.enabled=false;
		tonal_vs_noise.preserve=0.5;
		tonal_vs_noise.bandwidth=0.9;
	};
	~ProcessParameters(){
	};
	void add2XML(XMLwrapper *xml);
	void getfromXML(XMLwrapper *xml);
	
	struct{
		bool enabled;
		int cents;
	}pitch_shift;

	struct{
		bool enabled;
		REALTYPE om2,om1,o0,o1,o15,o2;
	}octave;

	struct{
		bool enabled;
		int Hz;
	}freq_shift;

	struct{
		bool enabled;
		REALTYPE power;
	}compressor;

	struct{
		bool enabled;
		REALTYPE low,high;
		REALTYPE hdamp;
		bool stop;
	}filter;    

	struct{
		bool enabled;
		REALTYPE freq;
		REALTYPE bandwidth;
		int nharmonics;
		bool gauss;
	}harmonics;

	struct{
		bool enabled;
		REALTYPE bandwidth;
	}spread;

	struct{
		bool enabled;
		REALTYPE preserve;
		REALTYPE bandwidth;
	}tonal_vs_noise;

	FreeEdit free_filter;
	FreeEdit stretch_multiplier;

};

class ProcessedStretch:public Stretch{
	public:
		//stereo_mode: 0=mono,1=left,2=right
		ProcessedStretch(REALTYPE rap_,int in_bufsize_,FFTWindow w=W_HAMMING,bool bypass_=false,REALTYPE samplerate_=44100,int stereo_mode=0);
		~ProcessedStretch();
		void set_parameters(ProcessParameters *ppar);
	private:
		REALTYPE get_stretch_multiplier(REALTYPE pos_percents);
//		void process_output(REALTYPE *smps,int nsmps);
		void process_spectrum(REALTYPE *freq);
		void do_harmonics(REALTYPE *freq1,REALTYPE *freq2);
		void do_pitch_shift(REALTYPE *freq1,REALTYPE *freq2,REALTYPE rap);
		void do_freq_shift(REALTYPE *freq1,REALTYPE *freq2);
		void do_octave(REALTYPE *freq1,REALTYPE *freq2);
		void do_filter(REALTYPE *freq1,REALTYPE *freq2);
		void do_free_filter(REALTYPE *freq1,REALTYPE *freq2);
		void do_compressor(REALTYPE *freq1,REALTYPE *freq2);
		void do_spread(REALTYPE *freq1,REALTYPE *freq2);
		void do_tonal_vs_noise(REALTYPE *freq1,REALTYPE *freq2);

		void copy(REALTYPE *freq1,REALTYPE *freq2);
		void add(REALTYPE *freq2,REALTYPE *freq1,REALTYPE a=1.0);
		void mul(REALTYPE *freq1,REALTYPE a);
		void zero(REALTYPE *freq1);
		void spread(REALTYPE *freq1,REALTYPE *freq2,REALTYPE spread_bandwidth);

		void update_free_filter();
		int nfreq;

		REALTYPE *free_filter_freqs;
		ProcessParameters pars;

		REALTYPE *infreq,*sumfreq,*tmpfreq1,*tmpfreq2;
		//REALTYPE *fbfreq;
};

#endif


