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

#ifndef VORBISOUTPUT_H
#define VORBISOUTPUT_H
#include <string>
#include <stdio.h>
#include <vorbis/vorbisenc.h>
#ifndef REALTYPE
#define REALTYPE float
#endif

class VorbisOutputS{
    public:
	VorbisOutputS();
	~VorbisOutputS();
	
	bool newfile(std::string filename,int samplerate,REALTYPE quality=3.5);//quality 0.0-10.0
	void close();

	void write(int nsmps, REALTYPE *smpsl,REALTYPE *smpr);
	
    private:
	bool opened;

	ogg_stream_state os;
	ogg_page og;
	ogg_packet op;
  
	vorbis_info vi;
	vorbis_comment vc;

	vorbis_dsp_state vd;
	vorbis_block vb;
	
	FILE *outfile;
};
#endif
