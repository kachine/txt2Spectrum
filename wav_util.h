#include	<stdio.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<string.h>

#define	CHUNKID_RIFF	"RIFF"
#define	RIFF_FMT		"WAVE"
#define	CHUNKID_FMT		"fmt "
#define	CHUNKID_DATA	"data"
#define	CHUNKID_SMPL	"smpl"
#define	BITDEPTH_16		16
#define	CH_MONO			1
#define	CH_STEREO		2
#define	LOOP_FWD		0		// Loop forward
#define	LOOP_ALT		1		// Loop alternate
#define	LOOP_BWD		2		// Loop backward

struct _riff {
	char	ChunkID[4];	// "RIFF"
	int32_t	ChunkSize;
	char	Format[4];	// "WAVE"
};

struct _fmt {
	char	SubChunkID[4];	// "fmt "
	int32_t	SubChunkSize;
	int16_t	AudioFormat;	// 1: Liner PCM
	int16_t	NumChannels;
	int32_t	SampleRate;
	int32_t	ByteRate;
	int16_t	BlockAlign;
	int16_t	BitsPerSample;
};

struct _smpl {
	char ChunkID[4];		// "smpl"
	int32_t ChunkDataSize;
	int32_t Manufacture;	// 0
	int32_t Product;		// 0
	int32_t SamplePeriod;	// 1/FS*1000000000
	int32_t MidiUnityNote;
	int32_t MidiPitchFraction;
	int32_t SmpteFormat;	// 0: NO SMPTE offset
	int32_t SmpteOffset;
	int32_t NumSampleLoops;
	int32_t SamplerData;
};

struct _sampleloop {
	int32_t CuePointID;
	int32_t Type;		// 0: Loop forward, 1: Loop alternate(ping-pong), 2: Loop backward(reverse)
	int32_t Start;		// offset of loop start point
	int32_t End;		// offset of loop end point
	int32_t Fraction;	// 0: no fraction
	int32_t PlayCount;	// 0: Sustain Loop
};

int save_wav(int16_t wave[], unsigned int waveLen, unsigned int samplerate, unsigned char bitdepth, unsigned char channel, const char *filename);
int write_riff(FILE *fp, unsigned int waveLen, unsigned char bitdepth, unsigned char channel);
int write_fmt(FILE *fp, unsigned int samplerate, unsigned char bitdepth, unsigned char channel);
int write_data_16bit(FILE *fp, int16_t wave[], unsigned int waveLen, unsigned char channel);
