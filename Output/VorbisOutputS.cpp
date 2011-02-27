/*
  Copyright (C) 2006-2009 Nasca Octavian Paul and the Vorbis authors
  Author: Nasca Octavian Paul and Vorbis authors (XIPHOPHORUS Company)
  (some lines of code took from encoder_example.c from vorbis library)

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

#include <stdlib.h>
#include "VorbisOutputS.h"
using namespace std;

VorbisOutputS::VorbisOutputS(){
    outfile=NULL;
    opened=false;
};

VorbisOutputS::~VorbisOutputS(){
    close();
};

bool VorbisOutputS::newfile(string filename,int samplerate,REALTYPE quality){
    close();//inchide un posibil fisier existent


    outfile=fopen(filename.c_str(),"wb"); 
    if (!outfile) return false;

    vorbis_info_init(&vi);
    int ret=vorbis_encode_init_vbr(&vi,2,samplerate,quality/10.0);
    if (ret) return false;
    
    //adaug comentariu
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc,"program","PaulStretch by Nasca Octavian PAUL");

    //setari analiza
    vorbis_analysis_init(&vd,&vi);
    vorbis_block_init(&vd,&vb);
    
    ogg_stream_init(&os,0x3FB771E2);

    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
    ogg_stream_packetin(&os,&header);
    ogg_stream_packetin(&os,&header_comm);
    ogg_stream_packetin(&os,&header_code);

    int eos=0;
    while(!eos){
    	int result=ogg_stream_flush(&os,&og);
    	if(result==0)break;
	int tmp=0;
    	tmp=fwrite(og.header,1,og.header_len,outfile);
    	tmp=fwrite(og.body,1,og.body_len,outfile);
    };


    opened=true;
    return(true);
};

void VorbisOutputS::close(){
    if (!opened) return;
    write(0,NULL,NULL);//scriu un pachet de EOS
    
    fclose(outfile);
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    
    opened=false;
    
};

void VorbisOutputS::write(int nsmps,REALTYPE *smpsl,REALTYPE *smpsr){
    if (!opened) return;

    if (nsmps!=0){
	float **buffer=vorbis_analysis_buffer(&vd,nsmps);
	int i=0;
	for (i=0;i<nsmps;i++){
	    buffer[0][i]=smpsl[i];
    	    buffer[1][i]=smpsr[i];
	};
    };
    vorbis_analysis_wrote(&vd,nsmps);

    while(vorbis_analysis_blockout(&vd,&vb)==1){

        vorbis_analysis(&vb,NULL);
        vorbis_bitrate_addblock(&vb);

        while(vorbis_bitrate_flushpacket(&vd,&op)){
	    ogg_stream_packetin(&os,&op);
	    int eos=0;
	    while (!eos){
	        int result=ogg_stream_pageout(&os,&og);
		if(result==0)break;
		int tmp;
		tmp=fwrite(og.header,1,og.header_len,outfile);
		tmp=fwrite(og.body,1,og.body_len,outfile);
	  
		if(ogg_page_eos(&og))eos=1;
	    };
	};
    };

};

