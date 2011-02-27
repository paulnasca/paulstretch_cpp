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
#ifndef STRETCH_H
#define STRETCH_H
#include "globals.h"

#ifdef KISSFFT
#include <kiss_fftr.h>
#else
#include <fftw3.h>
#endif


enum FFTWindow{W_RECTANGULAR,W_HAMMING,W_HANN,W_BLACKMAN,W_BLACKMAN_HARRIS};

class FFT{//FFT class that considers phases as random
	public:
		FFT(int nsamples_);//samples must be even
		~FFT();
		void smp2freq();//input is smp, output is freq (phases are discarded)
		void freq2smp();//input is freq,output is smp (phases are random)
		void applywindow(FFTWindow type);
		REALTYPE *smp;//size of samples/2
		REALTYPE *freq;//size of samples
		int nsamples;
	private:
#ifdef KISSFFT
		kiss_fftr_cfg  plankfft,plankifft;	
		kiss_fft_scalar *datar;
		kiss_fft_cpx *datac;
#else
		fftwf_plan planfftw,planifftw;
		REALTYPE *data;
#endif
		struct{
			REALTYPE *data;
			FFTWindow type;
		}window;

		unsigned int rand_seed;
		static unsigned int start_rand_seed;
};

class Stretch{
	public:
		Stretch(REALTYPE rap_,int in_bufsize_,FFTWindow w=W_HAMMING,bool bypass_=false,REALTYPE samplerate_=44100,int stereo_mode_=0);
		//in_bufsize is also a half of a FFT buffer (in samples)
		virtual ~Stretch();

		void process(REALTYPE *smps,int nsmps);
		//		virtual void process_output(REALTYPE *smps,int nsmps){};

		int in_bufsize;
		int poolsize;//how many samples are inside the input_pool size (need to know how many samples to fill when seeking)

		int out_bufsize;
		REALTYPE *out_buf;//pot sa pun o variabila "max_out_bufsize" si asta sa fie marimea lui out_buf si pe out_bufsize sa il folosesc ca marime adaptiva

		int get_nsamples(REALTYPE current_pos_percents);//how many samples are required to be added in the pool next time
		int get_nsamples_for_fill();//how many samples are required to be added for a complete buffer refill (at start of the song or after seek)

		void set_rap(REALTYPE newrap);//set the current stretch value

		FFTWindow window_type;
	protected:
		virtual void process_spectrum(REALTYPE *freq){};
		virtual REALTYPE get_stretch_multiplier(REALTYPE pos_percents);
		REALTYPE samplerate;
		int stereo_mode;//0=mono,1=left,2=right
	private:
		REALTYPE *in_pool;//de marimea in_bufsize
		REALTYPE rap;
		REALTYPE *old_out_smp_buf;

		FFT *infft,*outfft;
		long double remained_samples;//how many fraction of samples has remained (0..1)
		bool bypass;
};


#endif

