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

#ifndef BINAURAL_BEATS
#define BINAURAL_BEATS
#include "FreeEdit.h"

#include "globals.h"

class AP{//allpass
	public:
		AP(REALTYPE a_=0.5){
			in1=in2=out1=out2=0.0;
			set(a_);
		};
		REALTYPE process(REALTYPE in){
			REALTYPE out=a*(in+out2)-in2;
			in2=in1;in1=in;
			out2=out1;out1=out;

			return out;
		};
		REALTYPE set(REALTYPE a_){
			a=a_*a_;
		};
	private:
		REALTYPE in1,in2,out1,out2;
		REALTYPE a;
};



class Hilbert{
	public:
		Hilbert(){
			for (int i=0;i<N;i++){
				apl[i].set(coefl[i]);
				apr[i].set(coefr[i]);
			};
			oldl=0.0;
		};
		void process(REALTYPE in, REALTYPE &out1, REALTYPE &out2){
			out1=oldl;
			out2=in;

			for (int i=0;i<N;i++){
				out1=apl[i].process(out1);
				out2=apr[i].process(out2);
			};


			oldl=in;	
		};

	private:
		static const int N=4;
		static const REALTYPE coefl[];
		static const REALTYPE coefr[];
		AP apl[N],apr[N];
		REALTYPE oldl;
};

enum BB_STEREO_MODE{
	SM_LEFT_RIGHT=0,
	SM_RIGHT_LEFT=1,
	SM_SYMMETRIC=2
};

struct BinauralBeatsParameters{
	BinauralBeatsParameters(){
		stereo_mode=SM_LEFT_RIGHT;
		mono=0.5;
	};

	BB_STEREO_MODE stereo_mode;
	float mono;
	FreeEdit free_edit;
	void add2XML(XMLwrapper *xml);
	void getfromXML(XMLwrapper *xml);
};

class BinauralBeats{
	public:
		BinauralBeats(int samplerate_);
		void process(REALTYPE *smpsl,REALTYPE *smpsr,int nsmps,REALTYPE pos_percents);
		BinauralBeatsParameters pars;
	private:
		int samplerate;
		REALTYPE hilbert_t;
		Hilbert hl,hr;
};

#endif
