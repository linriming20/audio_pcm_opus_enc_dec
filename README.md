#### 1、opus库移植步骤

源码下载地址：[https://opus-codec.org/](https://opus-codec.org/) 

```bash
tar xzf opus-1.5.2.tar.gz 
cd opus-1.5.2/
./configure --prefix=$PWD/_install
make -j8
make install
strip --strip-unneeded _install/lib/
```

```bash
tree _install/
_install/
├── [4.0K]  include
│   └── [4.0K]  opus
│       ├── [ 36K]  opus_defines.h
│       ├── [ 53K]  opus.h
│       ├── [ 33K]  opus_multistream.h
│       ├── [ 28K]  opus_projection.h
│       └── [5.0K]  opus_types.h
├── [4.0K]  lib
│   ├── [727K]  libopus.a
│   ├── [1006]  libopus.la
│   ├── [  17]  libopus.so -> libopus.so.0.10.1
│   ├── [  17]  libopus.so.0 -> libopus.so.0.10.1
│   ├── [374K]  libopus.so.0.10.1
│   └── [4.0K]  pkgconfig
│       └── [ 418]  opus.pc
└── [4.0K]  share
    └── [4.0K]  aclocal
        └── [4.0K]  opus.m4

6 directories, 12 files
```

#### 2、说明

##### a. demo现状

- `main_xxx2yyy_gpt.c`的文件是chatgpt生成的demo，没有实际验证效果，<font color=red>仅做参考</font>；

- `main_pcm2opus.c`编码：程序可以正常运行，但是编码出来没有播放器可以验证，<font color=red>不好确定设置的码率是否生效</font>，不过可以通过下面自己再解码成PCM即可验证是否正常；
- `main_opus2pcm.c`解码：在得到编码数据时，先写4个字节标识一下这段数据的大小，方便解码得到这一帧的数据长度。~~<font color=red>有问题，定位中。</font>另外，比较疑惑的是`opus_decode`函数`len`入参和输出buf的`frame_size`入参。~~ 
  - ~~我拿到一个opus数据，我怎么知道`len`入参应该是多少呢？是不是我随便往接口里塞数据就可以给我缓存没有解码完的非完全一帧的数据呢？~~ 
  - ~~按照头文件注释说`frame_size`入参是单个通道的采样数，但是解码demo里面的又不对，还需要继续深入学习。。。~~ 

##### b. 关于opus格式的说明

- 一个有损音频压缩的数字音频编码格式，由Xiph.Org基金会开发；
- 目标是希望用单一格式包含声音和语音，取代Speex和Vorbis，且适用于网络上低延迟的即时声音传输；
- 是一个开放格式，使用上没有任何专利或限制；
- Opus集成了两种声音编码的技术：以语音编码为导向的SILK和低延迟的CELT；
- 可以无缝调节高低比特率；
- Opus具有非常低的算法延迟（默认为22.5ms），非常适合用于低延迟语音通话的编码，也可以透过降低编码比特率，达成更低的算法延迟，最低可以到5 ms；
- Opus都比MP3、 AAC、 HE-AAC等常见格式，有更低的延迟和更好的声音压缩率；
- 6 kb/秒到510 kb/秒的比特率；单一频道最高256 kb/秒；
- 采样率从8 kHz（窄带）到48 kHz（全频）；只支持8KHz、12KHz、16KHz、24KHz、48KHz。
- 帧大小从2.5毫秒到60毫秒；
- 支持单声道和立体声；支持多达255个音轨（多数据流的帧）；

#### 3、demo使用方法

##### a. 编译

```bash
$ make clean && make
rm -rf pcm2opus opus2pcm pcm2opus_gpt opus2pcm_gpt out*
gcc main_pcm2opus.c -I./include -lopus -L./lib -lm -o pcm2opus
gcc main_opus2pcm.c -I./include -lopus -L./lib -lm -o opus2pcm
gcc main_pcm2opus_gpt.c -I./include -lopus -L./lib -lm -o pcm2opus_gpt
gcc main_opus2pcm_gpt.c -I./include -lopus -L./lib -lm -o opus2pcm_gpt
```

##### b. 编码

```bash
$ ./pcm2opus
Usage: 
         ./pcm2opus <in-pcm-file> <sample-rate> <bits per sample> <channels> <out-opus-file> <target bps>
examples: 
         ./pcm2opus ./audio/test_8000_16_1.pcm 8000 16 1 out1.opus 40000
         ./pcm2opus ./audio/test_16000_16_2.pcm 16000 16 2 out2.opus 90000
```

##### c. 解码

```bash
$ ./opus2pcm
Usage: 
         ./opus2pcm <in-opus-file> <sample-rate> <bits per sample> <channels> <per decode size> <out-pcm-file>
examples: 
         ./opus2pcm out1.opus 8000 16 1 199 out_8000_16_1.pcm # 199是编码时打印的opus数据大小
         ./opus2pcm out2.opus 16000 16 2 898 out_16000_16_2.pcm
```

#### 4、参考文章

- [opus编解码的特色和优点 - 虚生 - 博客园](https://www.cnblogs.com/dylancao/p/16168725.html) 
- [libopus 实现pcm 编码到opus-CSDN博客](https://blog.csdn.net/sinat_27720649/article/details/126530085) 
- [Opus从入门到精通(二)\_编解码器使用\_android\_轻口味\_InfoQ写作社区](https://xie.infoq.cn/article/d959ccfa217b89d6e7ed9ccde) 
- [【音视频 _ opus】opus编解码库(opus-1.4)详细介绍以及使用——附带解码示例代码 - 技术栈](https://jishuzhan.net/article/1722162296723083266) 
- [Opus 音频编码格式 · 陈亮的个人博客](https://chenliang.org/2020/03/15/opus-format/) 

#### 5、demo目录架构

```bash
$ tree -h
.
├── [4.0K]  audio
│   ├── [1.2M]  test_16000_16_2.pcm
│   └── [313K]  test_8000_16_1.pcm
├── [4.0K]  docs
│   ├── [3.2M]  libopus 实现pcm 编码到opus-CSDN博客.mhtml
│   ├── [888K]  Opus从入门到精通(二)_编解码器使用_android_轻口味_InfoQ写作社区.mhtml
│   ├── [1006K]  opus编解码的特色和优点 - 虚生 - 博客园.pdf
│   ├── [901K]  Opus 音频编码格式 · 陈亮的个人博客.pdf
│   └── [893K]  【音视频 _ opus】opus编解码库(opus-1.4)详细介绍以及使用——附带解码示例代码 - 技术栈.mhtml
├── [4.0K]  include
│   └── [4.0K]  opus
│       ├── [ 36K]  opus_defines.h
│       ├── [ 53K]  opus.h
│       ├── [ 33K]  opus_multistream.h
│       ├── [ 28K]  opus_projection.h
│       └── [5.0K]  opus_types.h
├── [4.0K]  lib
│   └── [727K]  libopus.a
├── [3.7K]  main_opus2pcm.c
├── [1.6K]  main_opus2pcm_gpt.c
├── [4.6K]  main_pcm2opus.c
├── [1.7K]  main_pcm2opus_gpt.c
├── [ 504]  Makefile
├── [4.0K]  opensource
│   └── [7.5M]  opus-1.5.2.tar.gz
└── [1.8K]  README.md

6 directories, 20 files
```
