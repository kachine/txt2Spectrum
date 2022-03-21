// text spectrum generator
// 
// How to make: 
//   Simply type make
// 
// How to execute:
//   ./txtSpectrumSynth OUTFILE SAMPLING_FREQ[Hz] MIN_FREQ[Hz] MAX_FREQ[Hz] LENGTH[s] INTERVAL[s] TEXT_IN_ASCII
//   example:
//   ./txtSpectrumSynth Message.wav 48000 22000 23000 8 0.1 "Hello world"
// 
// How to mix to your music:
//   Simply, paste the generated wave file into audio track of your DAW application.
//   Or, mix the waveform with your existing audio file.
//     example:
//       ffmpeg -stream_loop 1 -i Message.wav -i YourMusic.wav -filter_complex "amix=inputs=2:duration=longest:dropout_transition=8" Mixed.wav
//       Or
//       sox -m Message.wav YourMusic.wav Mixed.wav
//         If you use sox, you may have to make stereo version of Message.wav before type the command above.
// 
// Notice:
//   If you use compressed audio format like mp3 or aac, inaudible frequency signal will be eliminated.
//   Below is a example of my quick experiment.
//     Text spectrum generation condition: SAMPLING_FREQ=48kHz MIN_FREQ=21kHz MAX_FREQ=23kHz
//     Compressed into mp3         ==> Text spectrum was lost
//                     aac 256kbps ==> lost
//                     aac 384k    ==> decreased but visible
//                     aac 512k    ==> ok
//                     wav, flac   ==> perfect

#include	"stopWar.h"

#define MAX_MSG_LEN			1024
#define ASCII_CHAR_OFFSET	0x20

int getDot(unsigned char charCode, uint8_t x, uint8_t y);
double hanning(double phaseRatio);

int main(int argc, char *argv[]){
	int ret_code = EXIT_SUCCESS;
	char outputfile[PATH_MAX];
	unsigned int fs = DEFAULT_FS;
	double minFreq = DEFAULT_MIN_DRAW_FREQ;
	double maxFreq = DEFAULT_MAX_DRAW_FREQ;
	unsigned int audioLength = DEFAULT_AUDIOLENGTH;
	float charInterval = DEFAULT_CHAR_INTERVAL;
	char message[MAX_MSG_LEN];
	
	// Get args
	// [Notice] All parameter value has almost NO VALIDATION in this implementation.
	if(argc != ARGPOS_FREETEXT + 1){
		fprintf(stderr, "[Usage] %s OUTFILE SAMPLING_FREQ[Hz] MIN_FREQ[Hz] MAX_FREQ[Hz] LENGTH[s] INTERVAL[s] TEXT_IN_ASCII\n", argv[ARGPOS_RUNTIME]);
		return EXIT_FAILURE;
	}
	
	// Get output filename
	sprintf(outputfile, "%s", argv[ARGPOS_OUTPUTFILENAME]);
	
	// Get sampling frequency
	fs = atoi(argv[ARGPOS_FS]);
	fprintf(stdout, "[INFO] Sampling frequency=%d[Hz]\n", fs);
	
	// Get max/min frequency to draw spectrum
	minFreq = atof(argv[ARGPOS_MINFREQ]);
	fprintf(stdout, "[INFO] Min frequency to draw message within spectrogram=%.2lf[Hz]\n", minFreq);
	maxFreq = atof(argv[ARGPOS_MAXFREQ]);
	fprintf(stdout, "[INFO] Max frequency to draw message within spectrogram=%.2lf[Hz]\n", maxFreq);
	if(minFreq>=maxFreq){
		fprintf(stderr, "[ERR] Max frequency(%.2lf[Hz]) should be greater than min frequency(%.2lf[Hz])\n", maxFreq, minFreq);
		return EXIT_FAILURE;
	}
	if(maxFreq > fs/2 || minFreq > fs/2){
		fprintf(stderr, "[ERR] Max frequency(%.2lf[Hz]) or min frequency(%.2lf[Hz]) should be less than the half of sampling frequency(%d[Hz])\n", maxFreq, minFreq, fs);
		return EXIT_FAILURE;
	}
	
	// Get wavefile length
	audioLength = atoi(argv[ARGPOS_AUDIOLENGTH]);
	fprintf(stdout, "[INFO] Length of output wave file=%d[s]\n", audioLength);
	
	// Get interval time between character
	charInterval = atof(argv[ARGPOS_INTERVAL]);
	fprintf(stdout, "[INFO] Interval of each characters within message=%.1f[s]\n", charInterval);
	
	// Get free text message
	sprintf(message, "%s", argv[ARGPOS_FREETEXT]);
	fprintf(stdout, "[INFO] FreeText=%s\n", message);
	
	//short msgLen = sizeof(msg) / (MAX_DOT * MAX_DOT);	// each character is consists of MAX_DOT*MAX_DOT dots
	short msgLen = strlen(message);
	#if DEBUG
	fprintf(stdout, "[INFO] msgLen=%d\n", msgLen);
	#endif
	
	// Variable to store entire output waveform in 16bit depth
	int16_t waveform[audioLength*fs];
	for(int i=0; i<audioLength*fs; i++){
		waveform[i] = 0;
	}
	
	// Determine frequency to draw each dot row
	float freq[MAX_DOT];
	for(short i=0; i<MAX_DOT; i++){
		freq[MAX_DOT-i-1] = (maxFreq - minFreq) / (MAX_DOT - 1) * i + minFreq;
		#if DEBUG
		fprintf(stdout, "[INFO] freq[%d]=%.2f[Hz]\n", i, freq[i]);
		#endif
	}
	
	// Determine duration for each dot column
	int duration = fs * (audioLength - charInterval * (msgLen - 1)) / msgLen / MAX_DOT;
	#if DEBUG
	fprintf(stdout, "[INFO] Duration=%d[samples]\n", duration);
	#endif
	
	// Generate waveform by character-column
	float sample[duration];	// single column tone buffer
	double amp = INT16_MAX / MAX_DOT;	// equivalent to coefficient to normalize waves
	for(short charPos=0; charPos<msgLen; charPos++){
		#if DEBUG
		// Read/print message
		for(uint8_t y=0; y<MAX_DOT; y++){
			for(uint8_t x=0; x<MAX_DOT; x++){
				if(getDot(message[charPos],x,y))	printf("*");
				else	printf(" ");
			}
			printf("\n");
		}
		#endif
		// Generate waveform by column
		for(uint8_t x=0; x<MAX_DOT; x++){
			// Clear the column tone buffer
			for(int i=0; i<duration; i++){
				sample[i] = 0;
			}
			// Synthesize the column tone
			for(uint8_t y=0; y<MAX_DOT; y++){
				if(getDot(message[charPos],x,y)){
					for(int i=0; i<duration; i++){
						// Synthesize waves with fade in/out to anti-click(pop) using half-sine
						//sample[i] += cos(2.0 * M_PI * freq[y] * i / fs) * sin(M_PI * i/(duration-1));
						// Synthesize waves with fade in/out to anti-click(pop) using hanning window
						sample[i] += cos(2.0 * M_PI * freq[y] * i / fs) * hanning((double)i/(duration-1));
					}
				}
			}
			// Writeout column tone buffer to entire output waveform
			unsigned int waveStartPos =
				x * duration					// Offset in current character
				+ charPos * duration * MAX_DOT	// Offset of characters until previous character
				+ charPos * fs * charInterval;	// Offset of character intervals
			for(int i=0; i<duration; i++){
				waveform[waveStartPos+i] = amp * sample[i];
			}
		}
	}
	
	// Output to wavefile
	if(save_wav(waveform, audioLength*fs, fs, BITDEPTH, CHANNELS, outputfile) != EXIT_SUCCESS){
		fprintf(stderr, "[ERR] File output error occured\n");
		ret_code = EXIT_FAILURE;
	}
	else{
		fprintf(stdout, "[INFO] File saved to %s\n", outputfile);
	}
	
	return ret_code;
}

int getDot(unsigned char charCode, uint8_t x, uint8_t y){
	return ( ( ( fontArray[charCode - ASCII_CHAR_OFFSET] >> (y * MAX_DOT) ) & 0xFF) >> x ) & 0x01;
}

double hanning(double phaseRatio){
	return 0.5 - 0.5 * cos(2.0 * M_PI * phaseRatio);
}
