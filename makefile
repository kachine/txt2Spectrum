# gcc stopWar.c wav_util.c -lm -O2 -o stopWar
PROGRAM = txtSpectrumSynth
PROGRAM_OLD = stopWar
OBJ = wav_util.o
CFLAGS = -Wall -O2
CC = gcc

all:	$(PROGRAM) $(PROGRAM_OLD)

$(PROGRAM):	$(PROGRAM).c $(OBJ) 
	$(CC) $(CFLAGS) $^ -lm -O2 -Wno-unknown-pragmas -o $(PROGRAM)

$(PROGRAM_OLD):	$(PROGRAM_OLD).c $(OBJ) 
	$(CC) $(CFLAGS) $^ -lm -O2 -Wno-unknown-pragmas -o $(PROGRAM_OLD)

wav_util.o:	wav_util.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(OBJ) $(PROGRAM_OLD) $(PROGRAM)
