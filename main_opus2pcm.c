#include <stdio.h>
#include <stdlib.h>

#include "opus/opus.h"

int main(int argc, char *argv[])
{
    /* 输出PCM文件相关信息 */
    char *out_pcm_file_name = NULL;
    FILE *fp_out_pcm = NULL;
    unsigned int sample_rate = 0;
    unsigned int bits_per_sample = 0;
    unsigned int channels = 0;
    unsigned int decode_per_size = 0;
    unsigned char *buffer = NULL;
    /* 输入OPUS文件相关信息 */
    char *in_opus_file_name = NULL;
    FILE *fp_in_opus = NULL;
    int error = 0;
    OpusDecoder *decoder = NULL;


    /* 检查参数 */
    if(argc != 7)
    {
        printf("Usage: \n"
                "\t %s <in-opus-file> <sample-rate> <bits per sample> <channels> <per decode size> <out-pcm-file>\n"
                "examples: \n"
                "\t %s out1.opus 8000 16 1 199 out_8000_16_1.pcm\n"
                "\t %s out2.opus 16000 16 2 898 out_16000_16_2.pcm\n"
                , argv[0], argv[0], argv[0]);
        return -1;
    }
    in_opus_file_name = argv[1];
    sample_rate = atoi(argv[2]);
    bits_per_sample = atoi(argv[3]);
    channels = atoi(argv[4]);
    decode_per_size = atoi(argv[5]);
    out_pcm_file_name = argv[6];

    switch(sample_rate)
    {
        case 8000:
        case 12000:
        case 16000:
        case 24000:
        case 48000:
            break;
        default:
            fprintf(stderr, "%d sample rate not support for OPUS!\n", sample_rate);
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

    // step1: 创建编码器，也可以按照 opus_decoder_get_size + malloc + opus_decoder_init 方法配置
    decoder = opus_decoder_create(sample_rate, channels, &error);
    if(decoder == NULL || error != OPUS_OK )
    {
        fprintf(stderr, "opus_encoder_create failed!\n");
        return -1;
    }

    // step2: 配置编码器
    opus_decoder_ctl(decoder, OPUS_SET_LSB_DEPTH(bits_per_sample));

    /* 打开pcm/opus文件 */
    fp_in_opus = fopen(in_opus_file_name, "rb");
    if(fp_in_opus == NULL)
    {
        perror("open opus file failed!");
        return -1;
    }
    fp_out_pcm = fopen(out_pcm_file_name, "wb");
    if(fp_out_pcm == NULL)
    {
        perror("create pcm file failed!");
        return -1;
    }

    unsigned int decode_per_samples = sample_rate / 50; // 20ms, 只能是 2.5, 5, 10, 20, 40 或 60 毫秒
    unsigned int decode_out_per_size = decode_per_samples * bits_per_sample/8 * channels;
    unsigned char *read_buff = calloc(decode_per_size, 1);
    unsigned char *decoded_buff = calloc(decode_out_per_size, 1); // 1s/50=20ms, 最大也就跟没压缩一样大小
    printf("It will decode opus file. (sample_rate: %d, channels: %d, decode_per_size: %d, decode out buf size: %d)\n",
        sample_rate, channels, decode_per_size, decode_out_per_size);
    while(1)
    {
        int len = 0;
        len = fread(read_buff, 1, decode_per_size, fp_in_opus);
        printf("--- read src opus data len: %d\n", len);
        if(len <= 0)
        {
            // file end
            printf("file end or error with read return value(%d).\n", len);
            break;
        }

        // step3: 编码
        //len = opus_decode(decoder, (const unsigned char *)read_buff, len, (opus_int16 *)decoded_buff, decode_per_samples, 0);
        //len = opus_decode(decoder, (const unsigned char *)read_buff, len, (opus_int16 *)decoded_buff, decode_per_samples * bits_per_sample/8, 0);
        len = opus_decode(decoder, (const unsigned char *)read_buff, len, (opus_int16 *)decoded_buff, decode_per_samples * bits_per_sample/8 * channels, 0);
        printf("+++ decode output opus data len: %d\n", len);
        if(len > 0)
        {
            fwrite(decoded_buff, len, 1, fp_out_pcm);
        }
        else
        {
            perror("opus_decode failed!");
            break;
        }
    }

    // step4: 销毁编码器
    opus_decoder_destroy(decoder);

    /* 关闭pcm/opus文件 */
    fclose(fp_in_opus);
    fclose(fp_out_pcm);
    free(read_buff);
    free(decoded_buff);

    return 0;
}
