#include <stdio.h>
#include <stdlib.h>

#include "opus/opus.h"

int main(int argc, char *argv[])
{
    /* 输入PCM文件相关信息 */
    char *in_pcm_file_name = NULL;
    FILE *fp_in_pcm = NULL;
    unsigned int sample_rate = 0;
    unsigned int bits_per_sample = 0;
    unsigned int channels = 0;
    unsigned char *buffer = NULL;
    /* 输出OPUS文件相关信息 */
    char *out_opus_file_name = NULL;
    FILE *fp_out_opus = NULL;
    int error = 0;
    OpusEncoder *encoder = NULL;
    unsigned int target_bps = 0;

    /* 检查参数 */
    if(argc != 7)
    {
        printf("Usage: \n"
                "\t %s <in-pcm-file> <sample-rate> <bits per sample> <channels> <out-opus-file> <target bps>\n"
                "examples: \n"
                "\t %s ./audio/test_8000_16_1.pcm 8000 16 1 out1.opus 40000\n"
                "\t %s ./audio/test_16000_16_2.pcm 16000 16 2 out2.opus 90000\n"
                , argv[0], argv[0], argv[0]);
        return -1;
    }
    in_pcm_file_name = argv[1];
    sample_rate = atoi(argv[2]);
    bits_per_sample = atoi(argv[3]);
    channels = atoi(argv[4]);
    out_opus_file_name = argv[5];
    target_bps = atoi(argv[6]);

    switch(sample_rate)
    {
        case 8000:
        case 12000:
        case 16000:
        case 24000:
        case 48000:
            break;
        default:
            fprintf(stderr, "OPUS format only support sample rate: 8000, 12000, 16000, 24000, or 48000.!\n");
            return -1;
            break;
    }
    switch(channels)
    {
        case 1:
        case 2:
            break;
        default:
            fprintf(stderr, "OPUS format only support channels: 1 or 2.!\n");
            return -1;
            break;
    }

    // step1: 创建编码器，也可以按照 opus_encoder_get_size + malloc + opus_encoder_init 方法配置
    int application = OPUS_APPLICATION_AUDIO;
    encoder = opus_encoder_create(sample_rate, channels, application, &error);
    if(encoder == NULL || error != OPUS_OK )
    {
        fprintf(stderr, "opus_encoder_create failed!\n");
        return -1;
    }

    // step2: 配置编码器
    //int signal_type = OPUS_SIGNAL_VOICE;
    int signal_type = OPUS_SIGNAL_MUSIC;
    opus_encoder_ctl(encoder, OPUS_SET_VBR(1)); // 0:CBR, 1:VBR
    opus_encoder_ctl(encoder, OPUS_SET_VBR_CONSTRAINT(1));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(target_bps));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(8)); // 8 0~10
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(signal_type));
    opus_encoder_ctl(encoder, OPUS_SET_LSB_DEPTH(bits_per_sample)); // 每个采样点位数
    opus_encoder_ctl(encoder, OPUS_SET_DTX(0));
    opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));


    /* 打开pcm/opus文件 */
    fp_in_pcm = fopen(in_pcm_file_name, "rb");
    if(fp_in_pcm == NULL)
    {
        perror("open pcm file failed!");
        return -1;
    }
    fp_out_opus = fopen(out_opus_file_name, "wb");
    if(fp_out_opus == NULL)
    {
        perror("create opus file failed!");
        return -1;
    }

    unsigned int encode_per_samples = sample_rate / 50; // 20ms, 只能是 2.5, 5, 10, 20, 40 或 60 毫秒
    unsigned int encode_per_size = encode_per_samples * bits_per_sample/8 * channels;
    unsigned char *read_buff = calloc(encode_per_size, 1);
    unsigned char *encoded_buff = calloc(encode_per_size, 1); // 最大也就跟没压缩一样大小
    printf("It will encode pcm file. (sample_rate: %d, channels: %d, encode_per_samples: %d, encode_per_size: %d)\n",
        sample_rate, channels, encode_per_samples, encode_per_size);
    while(1)
    {
        int len = 0;
        len = fread(read_buff, 1, encode_per_size, fp_in_pcm);
        printf("--- read src pcm data len: %d\n", len);
        if(len <= 0)
        {
            // file end
            printf("file end or error with read return value(%d).\n", len);
            break;
        }

        // step3: 编码
        len = opus_encode(encoder, (const opus_int16 *)read_buff, encode_per_samples, encoded_buff, encode_per_size);
        printf("+++ encoder output opus data len: %d\n", len);
        if(len > 0)
        {
            /* 写入帧大小 */
            /* LRM: 这里写入接下来编码的数据大小，这里记录方便后面解码知道要读多少，不是标准的opus文件格式 */
            fwrite(&len, 1, sizeof(len), fp_out_opus);
            /* 写入编码数据 */
            /* LRM: 这里写入编码后的opus数据 */
            fwrite(encoded_buff, 1, len, fp_out_opus);
        }
        else
        {
            perror("opus_encode failed!");
            break;
        }
    }

    // step4: 销毁编码器
    opus_encoder_destroy(encoder);

    /* 关闭pcm/opus文件 */
    fclose(fp_in_pcm);
    fclose(fp_out_opus);
    free(read_buff);
    free(encoded_buff);

    return 0;
}
