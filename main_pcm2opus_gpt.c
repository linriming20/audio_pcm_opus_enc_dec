#include <stdio.h>
#include <opus/opus.h>

#define FRAME_SIZE 960 // 20ms 帧大小
#define SAMPLE_RATE 48000 // 采样率
#define CHANNELS 2 // 声道数

int main() {
    OpusEncoder *encoder;
    int error;

    // 创建Opus编码器
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK) {
        fprintf(stderr, "Failed to create encoder: %s\n", opus_strerror(error));
        return 1;
    }

    // 设置编码器的比特率
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));

    // 输入PCM数据（假设从文件读取）
    FILE *pcm_file = fopen("input.pcm", "rb");
    if (!pcm_file) {
        fprintf(stderr, "Failed to open input PCM file\n");
        return 1;
    }

    // 输出Opus数据（假设写入到文件）
    FILE *opus_file = fopen("output.opus", "wb");
    if (!opus_file) {
        fprintf(stderr, "Failed to open output Opus file\n");
        fclose(pcm_file);
        return 1;
    }

    short pcm_data[FRAME_SIZE * CHANNELS];
    unsigned char opus_data[4096]; // 用于存储编码后的Opus数据

    while (!feof(pcm_file)) {
        // 从PCM文件读取数据
        fread(pcm_data, sizeof(short), FRAME_SIZE * CHANNELS, pcm_file);

        // 编码PCM数据为Opus数据
        int opus_size = opus_encode(encoder, pcm_data, FRAME_SIZE, opus_data, sizeof(opus_data));

        // 写入编码后的Opus数据到文件
        fwrite(opus_data, sizeof(unsigned char), opus_size, opus_file);
    }

    // 关闭文件和编码器
    fclose(pcm_file);
    fclose(opus_file);
    opus_encoder_destroy(encoder);

    printf("Encoding PCM to Opus completed successfully\n");

    return 0;
}