all: pcm2opus opus2pcm pcm2opus_gpt opus2pcm_gpt

# our static lib is compile by x86_64 gcc
CC := gcc
CFLAG := -I./include
LDFLAGS := -lopus -L./lib -lm

pcm2opus: main_pcm2opus.c
	$(CC) $^ $(CFLAG) $(LDFLAGS) -o $@

opus2pcm: main_opus2pcm.c
	$(CC) $^ $(CFLAG) $(LDFLAGS) -o $@

pcm2opus_gpt: main_pcm2opus_gpt.c
	$(CC) $^ $(CFLAG) $(LDFLAGS) -o $@

opus2pcm_gpt: main_opus2pcm_gpt.c
	$(CC) $^ $(CFLAG) $(LDFLAGS) -o $@

clean :
	rm -rf pcm2opus opus2pcm pcm2opus_gpt opus2pcm_gpt out*
.PHONY := clean

