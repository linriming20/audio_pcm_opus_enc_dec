#include <stdio.h>
#include <opus/opus.h>

#define FRAME_SIZE 960 // Opus帧大小
#define SAMPLE_RATE 48000 // 采样率
#define CHANNELS 2 // 声道数

int main() {
    OpusDecoder *decoder;
    int error;

    // 创建Opus解码器
    decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(error));
        return 1;
    }

    // 输入Opus数据（假设从文件读取）
    FILE *opus_file = fopen("input.opus", "rb");
    if (!opus_file) {
        fprintf(stderr, "Failed to open input Opus file\n");
        return 1;
    }

    // 输出PCM数据（假设写入到文件）
    FILE *pcm_file = fopen("output.pcm", "wb");
    if (!pcm_file) {
        fprintf(stderr, "Failed to open output PCM file\n");
        fclose(opus_file);
        return 1;
    }

    unsigned char opus_data[4096];
    short pcm_data[FRAME_SIZE * CHANNELS];

    while (!feof(opus_file)) {
        // 从Opus文件读取数据
        int opus_bytes = fread(opus_data, sizeof(unsigned char), sizeof(opus_data), opus_file);

        // 解码Opus数据为PCM数据
        int pcm_samples = opus_decode(decoder, opus_data, opus_bytes, pcm_data, FRAME_SIZE, 0);

        // 将解码后的PCM数据写入文件
        fwrite(pcm_data, sizeof(short), pcm_samples * CHANNELS, pcm_file);
    }

    // 关闭文件和解码器
    fclose(opus_file);
    fclose(pcm_file);
    opus_decoder_destroy(decoder);

    printf("Decoding Opus to PCM completed successfully\n");

    return 0;
}