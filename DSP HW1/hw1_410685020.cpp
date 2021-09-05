//
//  main.cpp
//  DSP HW1
//
//  Created by 何冠勳 on 2020/10/5.
//  Copyright © 2020 何冠勳. All rights reserved.
//
#define fs 16000       // sample freq = sample rate (Hz)
#define MP 2           // music period
#define NoS fs*MP      // number of samples

#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
using namespace std;

typedef struct WaveHeader
{
    // riff wave header
    char ChunkID[4] = {'R','I','F','F'};
    unsigned int ChunkSize;        // 0 ~ FFFF,FFFF
    char Format[4] = {'W','A','V','E'};
    
    // fmt subchunk
    char SubChunk1ID[4] = {'f','m','t',' '};
    unsigned int SubChunk1Size;    // 0 ~ FFFF,FFFF
    unsigned short AudioFormat;    // 0 ~ FFFF
    unsigned short NumChannels;    // 0 ~ FFFF
    unsigned int SampleRate;       // 0 ~ FFFF,FFFF
    unsigned int ByteRate;         // 0 ~ FFFF,FFFF
    unsigned short BlockAlign;     // 0 ~ FFFF
    unsigned short BitsPerSample;  // 0 ~ FFFF
    
    // data subchunk
    char SubChunk2ID[4] = {'d','a','t','a'};
    unsigned int SubChunk2Size;    // 0 ~ FFFF,FFFF
} WaveHeader;

WaveHeader make_WaveHeader(unsigned short const NumChannels, unsigned int const SampleRate, unsigned short const BitsPerSample)
{
    WaveHeader myWH;
    
    myWH.AudioFormat = 1;                  // 1 for PCM...
    myWH.SampleRate = SampleRate;
    myWH.NumChannels = NumChannels;        // 1 for Mono, 2 for Stereo
    myWH.BitsPerSample = BitsPerSample;
    myWH.ByteRate = (myWH.SampleRate * myWH.NumChannels * myWH.BitsPerSample)/8;
    myWH.BlockAlign = (myWH.NumChannels * myWH.BitsPerSample)/8;
    myWH.SubChunk2Size = (NoS * myWH.NumChannels * myWH.BitsPerSample)/8;
    myWH.SubChunk1Size = 16;               // 16 for PCM
    myWH.ChunkSize = 4+(8+myWH.SubChunk1Size)+(8+myWH.SubChunk2Size);
    
    return myWH;
}

bool step(float time)
{
    if(time<0.5)
        return 1;
    else
        return 0;
}

int main()
{
    short int x[8][NoS] = {0};
    float t[NoS] = {0};
    float const PI = acos(-1);
    int k[8] = {0,1,2,4,8,16,32,64};
    int f = 110;
    WaveHeader myWH = make_WaveHeader(1,fs,16);
    string filename;
    
    ofstream wavefile ;
    
    for(int i=0;i<NoS;i++)
        t[i]=(1.0/fs)*i;
    
    for(int j=0;j<8;j++)
    {
        for(int i=0;i<NoS;i++)
        {
            x[j][i] = 0.1 * cos(2 * PI * k[j] * f * t[i]) * step(t[i]) * pow(2,15);
            /*
             x[j][i] = floor(0.1 * cos(2 * PI * k[j] * f * t[i] * step(t[i])) * pow(2,15) + 0.5);
             rounding method
            */
        }
    }
    
    for(int i=0;i<8;i++)
    {
        filename = "wave_" + to_string(k[i]*f) + "Hz.wav";
        cout << filename << endl;
        wavefile.open(filename, ofstream::binary|ofstream::out);
        wavefile.write((char*)&myWH, sizeof(myWH));
        wavefile.write((char*)&x[i], sizeof(x[i]));
        wavefile.close();
    }
    
    return 0;
}
