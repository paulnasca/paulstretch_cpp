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

#ifndef CONTROL_H
#define CONTROL_H

#include "globals.h"
#include "Input/AInputS.h"
#include "Input/VorbisInputS.h"
#include "Input/MP3InputS.h"
#include "Output/AOutputS.h"
#include "Output/VorbisOutputS.h"
#include "ProcessedStretch.h"
#include "Player.h"
#include "JAaudiooutput.h"
#include "PAaudiooutput.h"
#include "BinauralBeats.h"

class Control{
	public:
		Control();
		~Control();

		void UpdateControlInfo();
		void startplay(bool bypass);
		void stopplay();
		void pauseplay();
		void freezeplay();

		void set_volume(REALTYPE vol);

		void set_seek_pos(REALTYPE x);
		REALTYPE get_seek_pos();

		bool save_parameters(const char *filename);
		bool load_parameters(const char *filename);

		bool playing(){
			return player->info.playing;
		};
		bool playing_eof(){
			return player->info.eof;
		};

		bool set_input_filename(std::string filename,FILE_TYPE intype);//return false if the file cannot be opened
		std::string get_input_filename();
		std::string get_input_filename_and_info();
		std::string get_stretch_info();
		std::string get_fftsize_info();
		std::string get_fftresolution_info();
		double get_stretch(){
			return process.stretch;
		};
		double get_onset_detection_sensitivity(){
			return process.onset_detection_sensitivity;
		};
		

		bool is_freeze(){
			return player->is_freeze();
		};

		void set_stretch_controls(double stretch_s,int mode,double fftsize_s,double onset_detection_sensitivity);//*_s sunt de la 0.0 la 1.0
		double get_stretch_control(double stretch,int mode);
		void update_player_stretch();

		void set_window_type(FFTWindow window);
		///	void pre_analyse_whole_audio(InputS *ai);

		std::string Render(std::string inaudio,std::string outaudio,FILE_TYPE outtype,FILE_TYPE intype,
				REALTYPE pos1,REALTYPE pos2);//returneaza o eroare sau un string gol (pos1,pos2 are from 0.0 to 1.0)
		struct {
			REALTYPE render_percent;
			bool cancel_render;
		}info;

		ProcessParameters ppar;
		BinauralBeatsParameters	bbpar;
		bool wav32bit;
		void update_process_parameters();//pt. player
		struct{
			double fftsize_s,stretch_s;
			int mode_s;
		}gui_sliders;	   
		FFTWindow window_type;

	private:
		REALTYPE volume;

		int get_optimized_updown(int n,bool up);
		int optimizebufsize(int bufsize);
		std::string getfftsizestr(int fftsize);

		struct {
			int bufsize;
			double stretch;
			double onset_detection_sensitivity;
		}process;

		struct {
			int samplerate;
			int nsamples;
			std::string filename;
			FILE_TYPE intype;
		}wavinfo;//input
		REALTYPE seek_pos;

		Player *player;
};

#endif

