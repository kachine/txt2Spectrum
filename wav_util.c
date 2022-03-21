#include	"wav_util.h"

int save_wav(int16_t wave[], unsigned int waveLen, unsigned int samplerate, unsigned char bitdepth, unsigned char channel, const char *filename)
{
	int ret_code = EXIT_SUCCESS;
	
	// Supports only 16bit wavefile
	if(bitdepth != BITDEPTH_16){
		return EXIT_FAILURE;
	}
	
	// Open file in binary write mode
	FILE *fp;
	fp = fopen(filename, "wb");
	if(fp == NULL){
		return EXIT_FAILURE;
	}
	
	// Write wave file
	if(write_riff(fp, waveLen, bitdepth, channel) != EXIT_SUCCESS){
		ret_code = EXIT_FAILURE;
	}
	else if(write_fmt(fp, samplerate, bitdepth, channel) != EXIT_SUCCESS){
		ret_code = EXIT_FAILURE;
	}
	else if(write_data_16bit(fp, wave, waveLen, channel) != EXIT_SUCCESS){
		ret_code = EXIT_FAILURE;
	}
	
	// Close file
	fclose(fp);
	return ret_code;
}

// Write RIFF chunk descripter
int write_riff(FILE *fp, unsigned int waveLen, unsigned char bitdepth, unsigned char channel){
	int ret_code = EXIT_SUCCESS;
	struct _riff RIFF;
	
	strncpy(RIFF.ChunkID, CHUNKID_RIFF, sizeof(RIFF.ChunkID));
	//RIFF.ChunkSize = 36 + waveLen * bitdepth / 8;
	RIFF.ChunkSize = 36 + waveLen * channel * bitdepth / 8;
	strncpy(RIFF.Format, RIFF_FMT, sizeof(RIFF.Format));
	if(fwrite(&RIFF, sizeof(RIFF), 1, fp) < 1){
		ret_code = EXIT_FAILURE;
	}
	
	return ret_code;
}

// Write "fmt" sub-chunk
int write_fmt(FILE *fp, unsigned int samplerate, unsigned char bitdepth, unsigned char channel){
	int ret_code = EXIT_SUCCESS;
	struct _fmt FMT;
	
	strncpy(FMT.SubChunkID, CHUNKID_FMT, sizeof(FMT.SubChunkID));
	FMT.SubChunkSize = 16;
	FMT.AudioFormat = 1;	// 1: Linear PCM
	FMT.NumChannels = channel;
	FMT.SampleRate = samplerate;
	FMT.BitsPerSample = bitdepth;
	FMT.BlockAlign = channel * bitdepth / 8;
	FMT.ByteRate = samplerate * FMT.BlockAlign;
	if(fwrite(&FMT, sizeof(FMT), 1, fp) < 1){
		ret_code = EXIT_FAILURE;
	}
	
	return ret_code;
}

// Write "data" sub-chunk for 16bit resolution wave
int write_data_16bit(FILE *fp, int16_t wave[], unsigned int waveLen, unsigned char channel){
	int ret_code = EXIT_SUCCESS;
	char dataSubChunkID[4] = CHUNKID_DATA;
	int32_t dataSubChunkSize = waveLen * channel * BITDEPTH_16 / 8;
	
	fwrite(dataSubChunkID, 1, 4, fp);
	fwrite(&dataSubChunkSize, 4, 1, fp);
	// Write the wave data itself
	for(int i=0; i < waveLen; i++) {
		if(fwrite(&wave[i], 2, 1, fp) < 1){
			ret_code = EXIT_FAILURE;
			break;
		}
	}
	
	return ret_code;
}
