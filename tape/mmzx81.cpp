/***************************************************************************
    mmzx81.cpp - ZX-81 File Convertor

    begin                : September 2, 2004
    copyright            : (C) 2004 by Michael Minn
    email                : see michaelminn.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of version 2 of the GNU General Public License as 
    published by the Free Software Foundation (see COPYING or www.gnu.org).

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

class mmwav_file
{
	public:
	char riff_title[4]; // RIFF
	unsigned int header_chunk_length;
	char wave_title[4]; // WAVE

	char format_chunk_title[4]; // fmt
	unsigned int format_chunk_length; // always 16
	unsigned short format_type; // always 1?
	unsigned short channel_numbers;
	unsigned int sample_rate;
	unsigned int bytes_per_second;
	unsigned short bytes_per_sample;
	unsigned short bits_per_sample; // resolution
	char data_chunk_title[4];
	unsigned int data_chunk_length;
	unsigned char data[];

	mmwav_file() { memset(this, 0, 44); }
	int band_pass_filter(int pitch_hz, int bandwidth_hz);
	int check_validity();
	int compress(float max_gain, int release_microseconds);
	int convert_to_8_bit_mono();
	int convert_to_binary(int peak, int carrier_hz);
	int low_pass_filter(int pitch_hz);
	int normalize();
	int write_file(int file_descriptor);
};

int mmwav_file::band_pass_filter(int pitch_hz, int bandwidth_hz)
{
	if ((bits_per_sample != 8) || (channel_numbers != 1) || !data_chunk_length || (sample_rate <= 0))
		return -EINVAL;
		
	double frequency = pitch_hz / (double) sample_rate;
	double bandwidth = bandwidth_hz / (double) sample_rate;
	if (bandwidth > frequency)
		bandwidth = frequency;
		
	double frequency_cosine = cos(2.0 * M_PI * frequency);
	double r = 1 - (3.0 * bandwidth);
	double k = (1 - (2.0 * r * frequency_cosine) + (r * r)) / (2.0 - (2.0 * frequency_cosine));
	double a0 = 1 - k;
	double a1 = 2.0 * (k - r) * frequency_cosine;
	double a2 = (r * r) - k;
	double b1 = 2.0 * r * frequency_cosine;
	double b2 = -r * r;
	
	double inbuffer[8] = { 0,0,0,0,0,0,0,0 };
	double outbuffer[8] = { 0,0,0,0,0,0,0,0 };

	for (int scan = 0, index = 0; scan < (int) data_chunk_length; ++scan, index = (index + 1) & 7)
	{
		inbuffer[index] = ((float)data[scan]) - 128;
		
		outbuffer[index] = (a0 * inbuffer[index])
				 + (a1 * inbuffer[(index - 1) & 0x07])
				 + (a2 * inbuffer[(index - 2) & 0x07])
				 + (b1 * outbuffer[(index - 1) & 0x07])
				 + (b2 * outbuffer[(index - 2) & 0x07]);

		double outvalue = outbuffer[index] + 128;
		if (outvalue < 0)
			outvalue = 0;
		else if (outvalue > 255)
			outvalue = 255;

		data[scan] = (unsigned char) outvalue;
	}

	return data_chunk_length;
}

int mmwav_file::check_validity()
{
	if (strncmp(riff_title, "RIFF", 4))
		fprintf(stderr, "No RIFF in .wav file header\n");

	else if (strncmp(wave_title, "WAVE", 4))
		fprintf(stderr, "No WAV in .wav file header\n");

	else if ((channel_numbers <= 0) || (channel_numbers > 2))
		fprintf(stderr, "Invalid number of channels: %d\n", channel_numbers);

	else if ((bits_per_sample != 8) && (bits_per_sample != 16))
		fprintf(stderr, "Invalid bit resolution %d in .wav header\n", bits_per_sample);

	else
	{
		fprintf(stderr, "WAV file: %d channels, %d samples/second, %d bit resolution\n",
			channel_numbers, sample_rate, bits_per_sample);
		return data_chunk_length;
	}

	return -EINVAL;
}

int mmwav_file::compress(float max_gain, int release_microseconds)
{
	if ((bits_per_sample != 8) || (channel_numbers != 1) || !data_chunk_length)
		return -EINVAL;

	int release_samples = sample_rate * release_microseconds / 1000000;
	float gain = max_gain;
	float gain_increment = (max_gain - 1) / release_samples;
	
	for (int scan = 0; scan < (int) data_chunk_length; ++scan)
	{
		float value = data[scan] - 128;

		if (fabs(value * gain) > 127)
			gain = 127.0 / fabs(value);
			
		else if ((gain < max_gain) && (fabs(value * (gain + gain_increment)) < 127))
			gain += gain_increment;
			
		value *= gain;
		data[scan] = (unsigned char)(value + 128);
	}

	return data_chunk_length;
}

int mmwav_file::convert_to_8_bit_mono()
{
	unsigned char *source = data + (bits_per_sample / 8) - 1; // move to MSB of sample
	//unsigned char *destination = data;
	unsigned char *end = &data[data_chunk_length];
	int bytes_per_sample = (bits_per_sample / 8) * channel_numbers;
	
	for (unsigned char *destination = data; source < end; source += bytes_per_sample, ++destination)
	{
		*destination = *source;
		if (bits_per_sample > 8)
			(*destination) += 128;
	}

	data_chunk_length /= (channel_numbers * bits_per_sample / 8);
	header_chunk_length = data_chunk_length + 36;
	bits_per_sample = 8;
	channel_numbers = 1;
	bytes_per_sample = 1;
	bytes_per_second = sample_rate * bits_per_sample * channel_numbers / 8;

	return data_chunk_length + 44;	
}

int mmwav_file::convert_to_binary(int threshold, int carrier_hz)
{
	if ((bits_per_sample != 8) || (channel_numbers != 1) || !data_chunk_length)
		return -EINVAL;

	int hi_threshold = 128 + threshold;
	int lo_threshold = 128 - threshold;
	int window_samples = 2 * sample_rate / carrier_hz;
	
	for (int scan = 0; scan < (int) data_chunk_length; )
	{
		// Skip and silence spaces before pulses
		while ((scan < (int) data_chunk_length) && (data[scan] < hi_threshold) && (data[scan] > lo_threshold))
		{
			data[scan] = 0;
			++scan;
		}	

		// Find width of pulse
		int pulse_start = scan;
		int pulse_end = scan;
		for (int pulse = scan; (pulse < (int) data_chunk_length) && (pulse < (pulse_end + window_samples)); ++pulse)
			if ((data[pulse] >= hi_threshold) || (data[pulse] <= lo_threshold))
				pulse_end = pulse + 1;

		// fprintf(stderr, "Pulse %d %d - %d (%d)\n", scan, pulse_start, pulse_end, window_samples);
		
		// If the pulse is less than window length, it's a transient that should be silenced
		if (pulse_end < (pulse_start + window_samples))
			for (int pulse = pulse_start; pulse < pulse_end; ++pulse)
				data[pulse] = 0;
				
		// Otherwise, fill the pulse
		else
			for (int pulse = pulse_start; pulse < pulse_end; ++pulse)
				data[pulse] = 255;
			
		// Start looking for the next pulse
		scan = pulse_end;
	}
		
	return data_chunk_length;
}

int mmwav_file::low_pass_filter(int pitch_hz)
{
	if ((bits_per_sample != 8) || (channel_numbers != 1) || !data_chunk_length)
		return -EINVAL;

	double decay = pow(M_E, -2.0 * M_PI * pitch_hz / sample_rate);
	double a0 = (1 + decay) / 2.0;
	double a1 = -(1.0 + decay) / 2.0;
	double b1 = decay;
	double in0 = 0;
	double in1 = 0;
	double out0 = 0;
	double out1 = 0;

	for (int scan = 0; scan < (int) data_chunk_length; ++scan)
	{
		in0 = ((float)data[scan]) - 128;
		out0 = (a0 * in0) + (a1 * in1) + (b1 * out1);
		if (out0 < -127)
			out0 = -127;
		else if (out0 > 127)
			out0 = 127;

		data[scan] = (unsigned char)(out0 + 128);
		in1 = in0;
		out1 = out0;
	}

	return data_chunk_length;
}

int mmwav_file::normalize()
{
	if ((bits_per_sample != 8) || (channel_numbers != 1) || !data_chunk_length)
		return -EINVAL;

	int peak = 128;
	for (int scan = 0; scan < (int) data_chunk_length; ++scan)
		if (peak < data[scan])
			peak = data[scan];

	if (peak <= 128)
		return data_chunk_length;

	float ratio = 127 / (peak - 128.0);

	for (int scan = 0; scan < (int) data_chunk_length; ++scan)
	{
		int value = (int) data[scan] - 128;
		value = (int)(value * ratio);
		data[scan] = (unsigned char)(value + 128);
	}

	return data_chunk_length;
}

int mmwav_file::write_file(int file_descriptor)
{
	int status = 0;
	header_chunk_length = data_chunk_length + 36;

	if (lseek(file_descriptor, 0, SEEK_SET) == -1)
	{
		status = -errno;
		perror("Failure seeking start of output file for header write");
	}

	else if ((status = write(file_descriptor, (void*) this, data_chunk_length + 44)) < 0)
	{
		status = -errno;
		perror("Failure writing output file header");
	}

	return status;
}

/***************************************************************************
***************************************************************************/

const char *character_set[256] =
{
	" ",	"(01)",	"(02)",	"(03)",
	"(04)",	"(05)",	"(06)",	"(07)",
	"(08)",	"(09)",	"(0a)",	"\"",
	// 0x12
	"&",	"$",	":",	"?",
	"(",	")",	">",	"<",
	"=",	"+",	"-",	"*",
	"/",	";",	",",	".",
	"0",	"1",	"2",	"3",
	"4",	"5",	"6",	"7",
	"8",	"9",	"A",	"B",
	"C",	"D",	"E",	"F",
	"G",	"H",	"I",	"J",
	"K",	"L",	"M",	"N",
	"O",	"P",	"Q",	"R",
	"S",	"T",	"U",	"V",
	"W",	"X",	"Y",	"Z",
	// 0x40
	"RND",	"INKEY$","PI",	"(43)",
	"(44)",	"(45)",	"(46)",	"(47)",	
	"(48)",	"(49)",	"(4a)",	"(4b)",	
	"(4c)",	"(4d)",	"(4e)",	"(4f)",	
	"(50)",	"(51)",	"(52)",	"(53)",	
	"(54)",	"(55)",	"(56)",	"(57)",	
	"(58)",	"(59)",	"(5a)",	"(5b)",	
	"(5c)",	"(5d)",	"(5e)",	"(5f)",	
	"(60)",	"(61)",	"(62)",	"(63)",	
	"(64)",	"(65)",	"(66)",	"(67)",	
	"(68)",	"(69)",	"(6a)",	"(6b)",	
	"(6c)",	"(6d)",	"(6e)",	"(6f)",	
	"(70)",	"(71)",	"(72)",	"(73)",	
	"(74)",	"(75)",	"(EOL)","(77)",	
	"(78)",	"(79)",	"(7a)",	"(7b)",	
	"(7c)",	"(7d)",	"(7e)",	"(7f)",	
	// 0x80
	"#",	"|1|",	"|2|",	"|3|",
	"|4|",	"|5|",	"|6|",	"|7|",
	"|8|",	"|9|",	"|10|",	"\"",
	"&",	"$",	":",	"?",
	"(",	")",	">",	"<",
	"=",	"+",	"-",	"*",
	"/",	";",	",",	".",
	"0",	"1",	"2",	"3",
	"4",	"5",	"6",	"7",
	"8",	"9",	"a",	"b",
	"c",	"d",	"e",	"f",
	"g",	"h",	"i",	"j",
	"k",	"l",	"m",	"n",
	"o",	"p",	"q",	"r",
	"s",	"t",	"u",	"v",
	"w",	"x",	"y",	"z",
	// 0xc0
	"'",	"AT",	"TAB",	"(c3)",
	"CODE",	"VAL",	"LEN",	"SIN",
	"COS",	"TAN",	"ASN",	"ACS",
	"ATN",	"LN",	"EXP",	"INT",
	"SQR",	"SGN",	"ABS",	"PEEK",
	"USR",	"STR$",	"CHR$",	"NOT",
	"**",	"OR",	"AND",	"<=",
	">=",	"<>",	"THEN",	"TO",
	"STEP",	"LPRINT","LLIST","STOP",
	"SLOW",	"FAST",	"NEW",	"SCROLL",
	"CONT",	"DIM",	"REM",	"FOR",
	"GOTO",	"GOSUB","INPUT","LOAD",
	"LIST",	"LET",	"PAUSE","NEXT",
	"POKE",	"PRINT","PLOT",	"RUN",
	"SAVE",	"RAND",	"IF",	"CLS",
	"UNPLOT","CLEAR","RETURN","COPY"
};

int list_zx81_code_file(unsigned char *data, int length)
{
	if (length < 116)
	{
		fprintf(stderr, "File is too short for ZX-81 program\n");
		return -EINVAL;
	}

	unsigned char version = data[0];
	int current_line = data[1] + (((int)data[2]) << 8);
	int display_area = data[3] + (((int)data[4]) << 8);
	int print_position = data[5] + (((int)data[6]) << 8);
	int variable_area = data[7] + (((int)data[8]) << 8);
	int current_line_work_area = data[11] + (((int)data[12]) << 8);
	int calculator_stack  = data[17] + (((int)data[18]) << 8);
	int spare_area = data[19] + (((int)data[20]) << 8);
	int calculator_memory = data[22] + (((int)data[23]) << 8);

	printf("------------------ ZX-81 Program Listing ------------------\n\n");
	printf("File version: %02x (VERSN: 4009)\n", version);
	printf("Current line: %d (E_PPC: 400A)\n", current_line);
	printf("Print position: %d (DF_CC: 400E)\n\n", print_position);
	printf("RAM Layout ------------------\n");
	printf("    [4000] System variables (start of RAM)\n");
	printf("    [4009] Start of area loaded from tape\n");
	printf("    [407D] Program area\n");
	printf("    [%04x] Display area (D_FILE: 400C)\n", display_area);
	printf("    [%04x] Variable area (VARS: 4010)\n", variable_area);
	printf("    [%04x] Current line work area (E_LINE: 4012)\n", current_line_work_area);
	printf("    [%04x] Calculator stack (STKBOT: 401a)\n", calculator_stack);
	printf("    [%04x] Spare area (STKEND: 401c)\n", spare_area);
	printf("    [----] Machine stack (sp)\n");
	printf("    [----] GOSUB stack (ERR_SP)\n");
	printf("    [----] USR routines (RAMTOP)\n");
	printf("    [%04x] Calculator memory (MEM: 401F)\n", calculator_memory);

	int max_address = 0x4009 + length;
	if (length < (display_area - 0x4009))
		fprintf(stderr, "Display area %04x beyond end of file %04x: file is probably corrupt\n",
			display_area, max_address);

	if ((display_area < 0x407d) || 
	    (variable_area < display_area) || 
	    (current_line_work_area < variable_area))
	{
		fprintf(stderr, "Corrupt file header (D_FILE %x, VARS %x, E_LINE %x)\n",
			display_area, variable_area, current_line_work_area);
		return -EINVAL;
	}

	printf("\nProgram ------------------\n");
	for (int address = 0x407d, line_length = 0; 
		(address < max_address) && (address < display_area); 
		address += (line_length + 4))
	{
		int line_number = (((int) data[address - 0x4009]) << 8) + data[address + 1 - 0x4009];
		line_length = data[address + 2 - 0x4009] + (((int) data[address + 3 - 0x4009]) << 8);

		if ((address + line_length) >= display_area)
		{
			fprintf(stderr, "Bad line length %d at %04x: file is probably corrupt\n", 
				line_length, address);
			return -EINVAL;
		}

		int command = data[address + 4 - 0x4009];
		printf("    [%04x] %.4d %s ", address, line_number, character_set[command]);

		for (int text = address + 5 - 0x4009, end = text + line_length - 1; text < end; ++text)
			if (data[text] == 0x7e) // Numeric constant - 5 bytes follow
			{
				printf(" ");
				text += 5;
			}
			else if (((data[text] >= 0x12) && (data[text] < 0x40)) ||
				 ((data[text] >= 0x80) && (data[text] < 0xc0)))
				printf("%s", character_set[data[text]]);

			else if (command == 0xea) // REM comments can contain binary data
				printf("(%02x)", data[text]);

			else if (data[text] == 0x76) // Don't print EOL
				continue;

			else if (data[text] >= 0xc0)
				printf("%s ", character_set[data[text]]);

			else
				printf("%s", character_set[data[text]]);

		printf("\n");
	}

	/**
	printf("\nDisplay ------------------\n");
	for (int address = display_area, line_length = 0; 
		(address < max_address) && (address < variable_area); ++address)
		if (data[address - 0x4009] == 0x76)
			printf("\n");
		else
			printf("%s", character_set[data[address - 0x4009]]);
	**/

	printf("\nVariables ------------------\n");
	for (int address = variable_area; 
		(address < max_address) && (address < current_line_work_area) && 
		(data[address - 0x4009] !=0x80);)
	{
		int end = 0;
		printf("    [%04x] ", address);

		switch (data[address - 0x4009] & 0xe0)
		{
			case 0x40: // String
				printf("%s$=\"", character_set[(data[address - 0x4009] & 0x3f) + 0x20]);
				end = address + 3 + data[address + 1 - 0x4009] 
						+ (((int) data[address + 2 - 0x4009]) << 8);
				if (end > current_line_work_area)
					end = current_line_work_area;

				for (address += 3; address < end; ++address)
					printf("%s", character_set[data[address - 0x4009] & 0x3f]);
				printf("\" (string)\n");
				break;

			case 0x60: // Single letter numeric variable
				printf("%s (number)\n", character_set[data[address - 0x4009] & 0x3f]);
				address += 6;
				break;

			case 0x80: // Array of numbers
				printf("%s[", character_set[(data[address - 0x4009] & 0x3f) + 0x20]);
				end = address + 3 + data[address + 1 - 0x4009] 
						+ (((int) data[address + 2 - 0x4009]) << 8);
				for (int dim = 0; (dim < data[address + 3 - 0x4009]) && (dim < 8); ++dim)
					if (dim == (data[address + 3 - 0x4009] - 1))
						printf("%d] (array)\n", data[address + 4 + (dim * 2) - 0x4009] +
							(((int) data[address + 5 + (dim * 2) - 0x4009]) << 8));
					else
						printf("%d,", data[address + 4 + (dim * 2) - 0x4009] +
							(((int) data[address + 5 + (dim * 2) - 0x4009]) << 8));
				address = end;
				break;

			case 0xa0: // Multiple letter numeric variable
				for (int start = address; ((address < (start + 2))
						|| !(data[address - 0x4009 - 1] & 0x80))
						&& (address < current_line_work_area); ++address)
					printf("%s", character_set[data[address - 0x4009] & 0x3f]);
				printf(" (number)\n");
				address += 5;
				break;

			case 0xc0: // Array of characters
				printf("%s$ (character array)", character_set[data[address - 0x4009] & 0x3f]);
				end = address + 3 + data[address + 1 - 0x4009] 
						+ (((int) data[address + 2 - 0x4009]) << 8);
				address = end;
				break;

			case 0xe0: // Control variable
				printf("%s (control variable)\n", character_set[data[address - 0x4009] & 0x3f]);
				address += 18;
				break;
		}
	}

	printf("\n----------------------- %d bytes listed --------------------\n\n", length);

	return length;
}

int list_zx81_data(unsigned char *file_data, int length)
{
	for (int scan = 0; scan < length; scan += 8)
	{
		printf("%0.4x ", scan);

		for (int character = 0; character < 8; ++character) 
			if ((scan + character) < length)
				printf("%02x ", file_data[scan + character]);
			else
				printf("   ");

		for (int character = 0; character < 8; ++character) 
			if ((scan + character) < length)
				printf("%6s ", character_set[file_data[scan + character]]);
			else
				printf("       ");

		printf("\n");
	}

	return length;
}


/***************************************************************************
***************************************************************************/

int check_file_validity(unsigned char *data, int length)
{
	if (length < 116)
		return -EINVAL;

	int display_area = data[3] + (((int)data[4]) << 8);
	int variable_area = data[7] + (((int)data[8]) << 8);
	int current_line_work_area = data[11] + (((int)data[12]) << 8);
	int max_address = 0x4009 + length;

	//if (max_address < display_area)
	//	return -EINVAL;

	if ((display_area < 0x407d) || 
	    (variable_area < display_area) || 
	    (current_line_work_area < variable_area))
	{
		fprintf(stderr, "Invalid header addresses %04x %04x %04x\n", 
			display_area, variable_area, current_line_work_area);
		return -EINVAL;
	}

	for (int address = 0x407d, line_length = 0; 
		(address < max_address) && (address < display_area); 
		address += (line_length + 4))
	{
		line_length = data[address + 2 - 0x4009] + (((int) data[address + 3 - 0x4009]) << 8);
		if ((address + line_length) >= display_area)
		{
			fprintf(stderr, "[%04x] Invalid line length 0x%x extends past display area %04x\n", 
				address, line_length, display_area);
			return -EINVAL;
		}
			
		if (((address + 4) < max_address) && (data[address + 4 - 0x4009] < 0xc0))
		{
			fprintf(stderr, "[%04x] Invalid command code at start of line\n", address);
			return -EINVAL;
		}
	}

	return length;
}


int convert_to_zx81_code_file(mmwav_file *wav_file, unsigned char *zx81_data)
{
	int status = 0;

	if ((status = wav_file->check_validity()) < 0)
		return status;

	else if ((status = wav_file->convert_to_8_bit_mono()) < 0)
	{
		fprintf(stderr, "Failure converting to mono file\n");
		return status;
	}

	else if ((status = wav_file->band_pass_filter(3266, 1000)) < 0)
	{
		fprintf(stderr, "Failure filtering .wav file");
		return status;
	}
	
	else if ((status = wav_file->normalize()) < 0)
	{
		fprintf(stderr, "Failure normalizing .wav file");
		return status;
	}
	
	else if ((status = wav_file->compress(3.0, 300)) < 0)
	{
		fprintf(stderr, "Failure compressing.wav file");
		return status;
	}
	
	//wav_file->write_file(1);
	//if (1)
	//return 0;

	else if ((status = wav_file->convert_to_binary(64, 3000)) < 0)
	{
		fprintf(stderr, "Failure converting .wav file to binary");
		return status;
	}
	
	//wav_file->write_file(1);
	//return 0;


	fprintf(stderr, "Converting %d byte .wav file to ZX-81 .p code file\n",
		wav_file->data_chunk_length);

	int threshold_microseconds = 2400;
	int threshold_samples = wav_file->sample_rate * threshold_microseconds / 1000000;

	int length = 0;
	int bit_buffer = 0;
	int current_bit = 0;
	
	unsigned char *scan = wav_file->data;
	unsigned char *end = &wav_file->data[wav_file->data_chunk_length];

	while ((scan < end) && (length < 65536))
	{
		// Find start of pulse
		while ((scan < end) && (*scan == 0))
			++scan;

		// List start of byte
		if ((current_bit == 0) && ((length % 8) == 0))
			fprintf(stderr, "%06x @ sample %07d: ",
				length, scan - wav_file->data);
		
		// Find end of pulse
		unsigned char *bit_start = scan;
		while ((scan < end) && (*scan > 0))
			++scan;

		// Find if bit is 1 or 0 based on length of pulse
		int bit_length = scan - bit_start;
		if (scan < end)
			if (bit_length > threshold_samples)
				bit_buffer = (bit_buffer << 1) | 1;
			else
				bit_buffer = (bit_buffer << 1);

		// Copy completed byte into data buffer
		++current_bit;
		if (current_bit == 8)
		{
			zx81_data[length] = (unsigned char) bit_buffer;
			++length;
			bit_buffer = 0;
			current_bit = 0;

			if ((length % 8) == 0)
			{
				for (int bytescan = 0; bytescan < 8; ++bytescan)
					printf("%02x ", zx81_data[length - 8 + bytescan]);
					
				for (int bytescan = 0; bytescan < 8; ++bytescan)
					printf("%s ", character_set[zx81_data[length - 8 + bytescan]]);

				printf("\n");
				
				if ((length > 116) && (check_file_validity(&zx81_data[1], length - 1) < 0))
				{
					fprintf(stderr, ".wav file error somewhere around sample %d\n", 
						scan - wav_file->data);
					return -EINVAL;
				}
			}
		}
	}

	fprintf(stderr, "Converted to %d bytes of ZX-81 data\n", length);

	return length;
}

/***************************************************************************
***************************************************************************/

int print_syntax()
{
	fprintf(stderr, "SYNTAX: mmzx81 <input file>\n");
	fprintf(stderr, "    If input is xxx.wav, output ZX-81 xxx.p code file\n");
	fprintf(stderr, "    If input is .p (ZX-81 code file), output is text list of BASIC code\n");
	return -1;
}

int main(int argc, char **argv)
{
	int descriptor = 0;
	unsigned char *file_data = NULL;
	struct stat file_status;

	fprintf(stderr, "MMZX81 - ZX-81 File Convertor\n"
		"v2004-09-07 (c) 2004 by Michael Minn\n");

	if (argc != 2)
		return print_syntax();

	else if (stat(argv[1], &file_status) < 0)
		perror(argv[1]);

	else if ((file_data = new unsigned char[file_status.st_size]) == NULL)
		perror("Failure allocating file buffer");

	else if ((descriptor = open(argv[1], O_RDONLY)) < 0)
		perror(argv[1]);

	else if (read(descriptor, file_data, file_status.st_size) < 0)
		perror(argv[1]);

	else if (close(descriptor) < 0)
		perror(argv[1]);
		
	else if (strstr(argv[1], ".wav"))
	{
		unsigned char zx81_data[0x10000]; // 64K memory limit
		int length = convert_to_zx81_code_file((mmwav_file*) file_data, zx81_data);
		if (length < 0)
			return length;

		list_zx81_code_file(&zx81_data[1], length - 1);

		char *output_name = strcpy(new char[strlen(argv[1]) + 1], argv[1]);
		strcpy(strstr(output_name, ".wav"), ".p");
		int output_file = open(output_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (output_file < 0)
			perror(output_name);

		else if (write(output_file, &zx81_data[1], length - 1) < 0)
			perror(output_name);

		else
			close(output_file);
	}	

	else if (strstr(argv[1], ".p"))
		list_zx81_code_file(file_data, file_status.st_size);

	else
		return print_syntax();

	return 0;
}

