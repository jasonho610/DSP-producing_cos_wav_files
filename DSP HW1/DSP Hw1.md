# 1. Wave File Format

![wave file format](http://soundfile.sapp.org/doc/WaveFormat/wav-sound-format.gif)

## RIFF

The wave format starts with the **RIFF** chunk:  

| Pos<br />(bytes) | Name      | Description                                                  |
| :--------------- | --------- | :----------------------------------------------------------- |
| 0 ~ 3            | ChunkID   | Contains the letters "RIFF" in ASCII form.                          (0x52494646 big-endian form) |
| 4 ~ 7            | ChunkSize | 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)<br />This is the size of the rest of the chunk following this number. |
| 8 ~ 11           | Format    | Contains the letters "WAVE".                         (0x57415645 big-endian form) |

(RIFF stands for Resource Interchange File Format.)

After **RIFF**, the WAVE format consists of two subchunks: **fmt** and **data**:

## fmt

The **fmt** subchunk describes the sound data's format:

| Pos<br />(bytes) | Name          | Description                                                  |
| ---------------- | ------------- | ------------------------------------------------------------ |
| 12 ~ 15          | Subchunk1ID   | Contains the letters "fmt "                                (0x666d7420 big-endian form). |
| 16 ~ 19          | Subchunk1Size | 16 for PCM. <br />This is the size of the rest of the Subchunk which follows this number. |
| 20 ~ 21          | AudioFormat   | PCM = 1.<br />Values other than 1 indicate some form of compression. |
| 22 ~ 23          | NumChannels   | Mono = 1, Stereo = 2.                                        |
| 24 ~ 27          | SampleRate    | 8000, 44100, etc.                                            |
| 28 ~ 31          | ByteRate      | == SampleRate * NumChannels * BitsPerSample/8                |
| 32 ~ 33          | BlockAlign    | == NumChannels * BitsPerSample/8<br />The number of bytes for one sample including all channels. |
| 34 ~ 35          | BitsPerSample | 8 bits = 8, 16 bits = 16, etc.                               |

The **data** subchunk contains the size of the data and the actual sound:

| Pos<br />(bytes) | Name          | Description                                                  |
| ---------------- | ------------- | ------------------------------------------------------------ |
| 36 ~ 39          | Subchunk2ID   | Contains the letters "data"                                (0x64617461 big-endian form). |
| 40 ~ 43          | Subchunk2Size | == NumSamples * NumChannels * BitsPerSample/8<br />This is the number of bytes in the data. |
| 44 ~             | Data          | The actual sound data.                                       |

# 2. C++ Code Demonstration

- The default byte ordering assumed for wave data files is little-endian. 

## Definition

```c++
#define fs 16000       // sample freq = sample rate (Hz)
#define MP 2           // music period
#define NoS fs*MP      // number of samples

#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
using namespace std;
```

- **fs** : sample freq = sample rate (Hz)
- **MP** : music period (sec)
- **NoS** : number of samples
- **fstream** : file library in c++

## Structure

```c++
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
```

- Wave is a binary file.
- Building a data structure for wave, according to wave file format.
- It has to be precise with the size of every element.
  - 0 ~ FFFF (HEX) = 0 ~ 65535 (DEC) = unsigned short
  - 0 ~ FFFF,FFFF (HEX) = 0 ~ 4294967295 (DEC) = unsigned int = unsigned long

## Constructor

```c++
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
```

- This is a constructor of wave header.

## Data

- Spec

  1. Sampling Rate: 16000 Hz
  2. Bit per Sample: linear 16 bits
  3. Number of Channels: 1
  4. File format: windows PCM (*.wav)

  ```c++
  WaveHeader myWH = make_WaveHeader(1,fs,16);
  ```

  5. Continuous-time Signal:

     - $x(t)=0.1*cos(2\pi kft)*w(t)$

     - $f=110 Hz$
     - $k=0,1,2,4,8,16,32,64$
     - $w(t)=\begin{cases} 1,t<0.5\\0, otherwise\end{cases}$   (Step Function)

  ```c++
  bool step(float time)
  {
      if(time<0.5)
          return 1;
      else
          return 0;
  }
  ```

  ```c++
  for(int i=0;i<NoS;i++)
    	t[i]=(1.0/fs)*i;
      
  for(int j=0;j<8;j++)
  {
    	for(int i=0;i<NoS;i++)
        	x[j][i] = 0.1 * cos(2 * PI * k[j] * f * t[i]) * step(t[i]) * pow(2,15);
  }
  ```

## Write File

```c++
for(int i=0;i<8;i++)
{
  	filename = "wave_" + to_string(k[i]*f) + "Hz.wav";
  	cout << filename << endl;
  	wavefile.open(filename, ofstream::binary|ofstream::out);
  	wavefile.write((char*)&myWH, sizeof(myWH));
  	wavefile.write((char*)&x[i], sizeof(x[i]));
  	wavefile.close();
}
```

- Instead of regular I/O, the file must be written in binary mode, and stored in byte type (char*).
- There will be 8 wave file: wave_0Hz.wav, wave_110Hz.wav, wave_220Hz.wav…wave_7040Hz.wav.

## Result

#### WaveForm

#### ![](/Users/jason/Desktop/DSP/DSP HW1/waveform.png)

- $k = 0, 1, 2, 4, ..., 64$ indicates the times of fundamental frequency, which is $f=110Hz$.

#### WaveFile

![image-20201007173940185](/Users/jason/Library/Application Support/typora-user-images/image-20201007173940185.png)

- 8 wave files named: wave_XXXHz.wav.

#### HexFile

![image-20201008113522759](/Users/jason/Library/Application Support/typora-user-images/image-20201008113522759.png)

![image-20201008113601355](/Users/jason/Library/Application Support/typora-user-images/image-20201008113601355.png)

![image-20201008113624768](/Users/jason/Library/Application Support/typora-user-images/image-20201008113624768.png)

- Checking the correctness of file format, including RIFF header, fmt subchunk, data subchunk.

# 3. Further Discuss

1. 當 $w(t)$ 的判斷到達 $t>=0.5s$ 的 criteria 時，照理來說，當下的sample應該要正好是第16000次。個人猜測大概是因為電腦是離散機器，所以數值只能求最逼近。而由於每台電腦的運算速度、方法的不同，趨近結果野可能不同。例如：其他電腦16046，我的Mac卻只需要到16044就停止。不過，測試過很多次，發現同台電腦都還是會一樣，也證明它的穩定性。

   ![image-20201008113320859](/Users/jason/Library/Application Support/typora-user-images/image-20201008113320859.png)

2. Endian 問題

   大部分的Intel CPU都是little-endian，所以Mac跟一般Windows PC其實不太需要擔心這問題。不過，當有桌機是以big-endian運算位址時，wav檔案就必須自行改寫。

   以下是測試電腦為何種endian的程式碼。

```c++
#include <iostream>
using namespace std;
typedef unsigned char BYTE;

int main()
{
	unsigned int num = 0;
	unsigned int* p  = &num;

	*(BYTE *)p = 0xff;
	if(num == 0xff)
	   cout<<"The endian of cpu is little"<<endl;
	else //num == 0xff000000
	   cout<<"The endian of cpu is big"<<endl;

	return 0;
}
```

3. Wave Structure裡的字母Header ("RIFF", "fmt ", "data")，不能用**string**只能用**char[4]**因為會多了'\0'，如此一來File Format就錯誤了。

