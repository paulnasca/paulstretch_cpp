/*
  Copyright (C) 2008-2011 Nasca Octavian Paul
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
#include <stdio.h>
#include "BinauralBeats.h"

void BinauralBeatsParameters::add2XML(XMLwrapper *xml){
	xml->addpar("stereo_mode",(int) stereo_mode);
	xml->addparreal("mono",mono);

	xml->beginbranch("FREE_EDIT");
	free_edit.add2XML(xml);
	xml->endbranch();
};
void BinauralBeatsParameters::getfromXML(XMLwrapper *xml){
	stereo_mode=(BB_STEREO_MODE)xml->getpar("stereo_mode",(int) stereo_mode,0,2);
	mono=xml->getparreal("mono",mono);

	if (xml->enterbranch("FREE_EDIT")){
		free_edit.getfromXML(xml);
		xml->exitbranch();
	};
};

//coefficients of allpass filters are found by Olli Niemitalo

const REALTYPE Hilbert::coefl[]={0.6923877778065, 0.9360654322959, 0.9882295226860, 0.9987488452737};
const REALTYPE Hilbert::coefr[]={0.4021921162426, 0.8561710882420, 0.9722909545651, 0.9952884791278};

BinauralBeats::BinauralBeats(int samplerate_){
    samplerate=samplerate_;
    hilbert_t=0.0;
};


void BinauralBeats::process(REALTYPE *smpsl,REALTYPE *smpsr,int nsmps,REALTYPE pos_percents){
//	for (int i=0;i<nsmps;i++) smpsl[i]=smpsr[i]=sin(30.0*M_PI*i*2.0/nsmps)*0.4;
	
	//printf(" enabled %d\n",pars.free_edit.get_enabled());
	if (pars.free_edit.get_enabled()){
		float mono=pars.mono*0.5;
		for (int i=0;i<nsmps;i++){
			REALTYPE inl=smpsl[i];
			REALTYPE inr=smpsr[i];
			REALTYPE outl=inl*(1.0-mono)+inr*mono;
			REALTYPE outr=inr*(1.0-mono)+inl*mono;

			smpsl[i]=outl;
			smpsr[i]=outr;
		};

		REALTYPE freq=pars.free_edit.get_value(pos_percents);


//		freq=300;//test

		freq*=0.5;//half down for left, half up for right
		for (int i=0;i<nsmps;i++){
			hilbert_t=fmod(hilbert_t+freq/samplerate,1.0);
			REALTYPE h1=0,h2=0;
			hl.process(smpsl[i],h1,h2);

			REALTYPE x=hilbert_t*2.0*M_PI;
			REALTYPE m1=h1*cos(x);
			REALTYPE m2=h2*sin(x);
			REALTYPE outl1=m1+m2;
			REALTYPE outl2=m1-m2;


			h1=0,h2=0;
			hr.process(smpsr[i],h1,h2);

			m1=h1*cos(x);
			m2=h2*sin(x);
			REALTYPE outr1=m1-m2;
			REALTYPE outr2=m1+m2;

			switch(pars.stereo_mode){
				case SM_LEFT_RIGHT:
					smpsl[i]=outl2;
					smpsr[i]=outr2;
					break;
				case SM_RIGHT_LEFT:
					smpsl[i]=outl1;
					smpsr[i]=outr1;
					break;
				case SM_SYMMETRIC:
					smpsl[i]=(outl1+outr1)*0.5;
					smpsr[i]=(outl2+outr2)*0.5;
					break;
			};	


	
	
		};

	};
};

