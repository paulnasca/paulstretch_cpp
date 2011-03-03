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

#ifndef INPUTS_H
#define INPUTS_H
#include "../globals.h"

class InputS{
	public:
		InputS(){
			skipbufsize=1024;
			skipbuf=new short int[skipbufsize*4];	
		};

		virtual ~InputS(){
			delete [] skipbuf;
		};
		virtual bool open(std::string filename)=0;
		virtual void close()=0;

		virtual int read(int nsmps,short int *smps)=0;
		void skip(int nsmps){
			while ((nsmps>0)&&(!eof)){
				int readsize=(nsmps<skipbufsize)?nsmps:skipbufsize;
				read(readsize,skipbuf);
				nsmps-=readsize;
			};
		};
		virtual void seek(double pos)=0;//0=start,1.0=end

		struct {
			int nsamples;
			int nchannels;
			int samplerate;
			int currentsample;
		} info;
		bool eof;	
	private:
		int skipbufsize;
		short int *skipbuf;

};

#endif


