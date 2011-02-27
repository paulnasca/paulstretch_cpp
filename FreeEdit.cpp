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

#include <stdio.h>
#include <math.h>
#include "FreeEdit.h"

FreeEdit::FreeEdit(){			
			enabled=false;
			smooth=0.0;
			interp_mode=LINEAR;
			npos=FREE_EDIT_MAX_POINTS;
			pos=new FreeEditPos[npos];
			for (int i=0;i<npos;i++){
				pos[i].x=pos[i].y=0;
				pos[i].enabled=false;
			};
			pos[0].x=0;
			pos[0].y=0.5;
			pos[0].enabled=true;
			pos[1].x=1;
			pos[1].y=0.5;
			pos[1].enabled=true;
			
			curve.data=NULL;
			curve.size=0;
};

void FreeEdit::deep_copy_from(const FreeEdit &other){
	enabled=other.enabled;
	smooth=other.smooth;
	interp_mode=other.interp_mode;
	npos=other.npos;
	pos=new FreeEditPos[npos];
	for (int i=0;i<npos;i++){
		pos[i].x=other.pos[i].x;
		pos[i].y=other.pos[i].y;
		pos[i].enabled=other.pos[i].enabled;		
	};
	curve.size=other.curve.size;
	if (other.curve.data&&other.curve.size){
		curve.data=new REALTYPE[curve.size];
		for (int i=0;i<curve.size;i++) curve.data[i]=other.curve.data[i];
	}else curve.data=NULL;
	extreme_x=other.extreme_x;
	extreme_y=other.extreme_y;
};
FreeEdit::FreeEdit (const FreeEdit &other){
	deep_copy_from(other);
};
const FreeEdit &FreeEdit::operator=(const FreeEdit &other){
	if (this == &other) return *this;
	deep_copy_from(other);
	return *this;
};
void FreeEdit::get_curve(int datasize,REALTYPE *data,bool real_values){
	int npos_used=0;
	for (int i=0;i<npos;i++) if (is_enabled(i)) npos_used++;
	if (!npos_used){
		for (int i=0;i<datasize;i++) data[i]=(real_values?extreme_y.get_min():0.0); 
		return;
	};

	//get the enabled points
	REALTYPE posx[npos_used],posy[npos_used];
	int k=0;
	for (int i=0;i<npos;i++){
		if (is_enabled(i)){
			posx[k]=get_posx(i);
			posy[k]=get_posy(i);
			k++;
		};
	};

	//sort the points
	for (int j=0;j<npos_used-1;j++){
		for (int i=j+1;i<npos_used;i++){
			if (posx[i]<posx[j]){
				swap(posx[i],posx[j]);
				swap(posy[i],posy[j]);
			};
		};
	};

	//generate the curve
	int p1=0,p2=1;
	for (int i=0;i<datasize;i++){
		REALTYPE x=(REALTYPE)i/(REALTYPE)datasize;
		while ((x>posx[p2])&&(p2<npos_used)){
			p1=p2;
			p2++;
		};
		REALTYPE px1=posx[p1];
		REALTYPE px2=posx[p2];
		REALTYPE diffx=px2-px1;
		REALTYPE x0=0;		
		if (diffx>1e-5) x0=(x-px1)/diffx;
		if (interp_mode==COSINE) x0=(1.0-cos(x0*M_PI))*0.5;
		REALTYPE y=y=posy[p1]*(1.0-x0)+posy[p2]*x0;
		data[i]=y;
	};


	//smooth the curve
	if (smooth>0.01){
		const int max_times=4;
		REALTYPE a=exp(log(0.25)/(smooth*smooth*datasize*0.25));
		if ((a<=0.0)||(a>=1.0)) return;
		a=pow(a,max_times);
		for (k=0;k<max_times;k++){
			for (int i=1;i<datasize;i++) data[i]=data[i]*(1.0-a)+data[i-1]*a;
			for (int i=datasize-2;i>=0;i--) data[i]=data[i]*(1.0-a)+data[i+1]*a;
		};
	};

	if (real_values){
		for (int i=0;i<datasize;i++) data[i]=extreme_y.coord_to_real_value(data[i]);
		if (extreme_y.get_scale()==FE_DB){
			for (int i=0;i<datasize;i++) data[i]=dB2rap(data[i]);
		};
	};	

};

void FreeEdit::update_curve(int size){
	if (curve.data) delete []curve.data;
	if (size<2) size=2;
	curve.size=size;
	curve.data=new REALTYPE[size];

	get_curve(curve.size,curve.data,true);

//	for(int i=0;i<size;i++) printf("_%d %g\n",i,curve.data[i]);
};

REALTYPE FreeEdit::get_value(REALTYPE x){
	if (!curve.data) {
		return 0.0;// update_curve();
	};
	if (extreme_x.get_scale()==FE_LOG){
		if (x<=0.0) x=1e-9;
	};
	
//	printf("%g\n",curve.data[1]);
	
	x=extreme_x.real_value_to_coord(x);
	if (x<0) x=0.0;
	else if (x>1.0) x=1.0;
	REALTYPE rx=x*curve.size;
	REALTYPE rxh=floor(rx);
	int k=(int)rxh;
	REALTYPE rxl=rx-rxh;
	if (k<0) k=0;
	if (k>(curve.size-1)) k=curve.size-1;
	int k1=k+1; if (k1>(curve.size-1)) k1=curve.size-1;
	return curve.data[k]*(1.0-rxl)+curve.data[k1]*rxl;
};

void FreeEdit::add2XML(XMLwrapper *xml){
	xml->addparbool("enabled",enabled);
	xml->addparreal("smooth",smooth);
	xml->addpar("interp_mode",interp_mode);
	xml->beginbranch("POINTS");
		for (int i=0;i<FREE_EDIT_MAX_POINTS;i++){
			if (!pos[i].enabled) continue;
			xml->beginbranch("POINT",i);
				xml->addparbool("enabled",pos[i].enabled);
				xml->addparreal("x",pos[i].x);
				xml->addparreal("y",pos[i].y);
	
			xml->endbranch();
		};
	xml->endbranch();

	xml->beginbranch("EXTREME_X");
		extreme_x.add2XML(xml);
	xml->endbranch();

	xml->beginbranch("EXTREME_Y");
		extreme_y.add2XML(xml);
	xml->endbranch();
};


void FreeEdit::getfromXML(XMLwrapper *xml){
	enabled=xml->getparbool("enabled",enabled);
	smooth=xml->getparreal("smooth",smooth);
	interp_mode=(INTERP_MODE)xml->getpar("interp_mode",interp_mode,0,1);
	
	
	
	if (xml->enterbranch("POINTS")){
		for (int i=0;i<FREE_EDIT_MAX_POINTS;i++){
			if (xml->enterbranch("POINT",i)){
				pos[i].enabled=xml->getparbool("enabled",pos[i].enabled);
				pos[i].x=xml->getparreal("x",pos[i].x);
				pos[i].y=xml->getparreal("y",pos[i].y);

				xml->exitbranch();
			};
		};
		xml->exitbranch();
	};

	if (xml->enterbranch("EXTREME_X")){
		extreme_x.getfromXML(xml);
		xml->exitbranch();
	};

	if (xml->enterbranch("EXTREME_Y")){
		extreme_y.getfromXML(xml);
		xml->exitbranch();
	};

	update_curve();
};


