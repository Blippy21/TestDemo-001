//
//  sndMan.h
//  VulkanTestDemo002
//
//  Created by Ian Minett on 1/5/25.
//

#ifndef _SNDMAN_H_      // The Sentinel
#define _SNDMAN_H_

#include <cstdlib>
#include <vector>

#include <AudioToolbox/AudioToolbox.h>

/* WaveFileInfo */
struct WaveFileInfo
{
    int                 ix;
    int                 channels;
    int                 freq;
    int                 bps;
    bool                isFloat;
    std::vector<char>   data;
};

/* RIFF chunks */
struct RIFFChunk
{
    char        ChunkID[4];
    uint32_t    ChunkSize;
    uint32_t    Format;
};

struct WAVEChunk
{
    char        ChunkID[4];
    uint32_t    ChunkSize;
    uint16_t    WaveFormat;
    uint16_t    WaveChannels;
    uint32_t    WaveFreq;
    uint32_t    WaveBytesPerSec;
    uint16_t    WaveBlockAlign;
    uint16_t    WaveBitsPerSample;
};

struct WAVEDATAChunk
{
    char        ChunkID[4];
    uint32_t    ChunkSize;
};

/* Wave File Layout */
struct WaveFileDesc
{
    RIFFChunk       Header;
    WAVEChunk       WaveHeader;
    WAVEDATAChunk   WaveData;
};

class CCyclicBuffer
{
public:
    CCyclicBuffer( int memSize );
    ~CCyclicBuffer();

public:
    int WriteData       ( char* pBuff, unsigned int numBytes );
    int ReadData        ( char* pBuff, unsigned int numBytes );
    int GetValidBytes   ( void )
        { return muiValidBytes; }

private:
    char*        mpMem;
    unsigned int muiMemSize;

    unsigned int muiValidBytes;

    unsigned int mWC;   // Write
    unsigned int mRC;   // Read
};


/* CSoundManager class */
class CSoundManager
{
public:

    CSoundManager();
    ~CSoundManager();

public:

    int  PlaySound      ( unsigned int ix );

    void  SetGain       ( float fGain );
    float GetGain       ( void );

    void SetFilterParam ( float fParam );

private:

    void LoadWaveFiles( void );

    static void playbackCBStub( void* userData, AudioQueueRef aq,
                                    AudioQueueBufferRef aqb );
    void playbackCB         ( AudioQueueRef aq, AudioQueueBufferRef aqb );
    
    void AllocQueueBuffers  ( AudioQueueRef aq );
    void FillBuffer         ( AudioQueueRef aq, AudioQueueBufferRef aqb );
    
    void InitFilter         ( size_t freq, size_t cutOffFreq );
    void ProcessFilter      ( char* data, int dataLen );

    int  CreateAudioPipe    ( void );
    int  RunAudioPipe       ( char* data, int dataLen );

private:

    std::vector<WaveFileInfo> mWaveFiles;

    float   mfGain;

    AudioQueueBufferRef mAqb[4];
    
    int     mCurrWave;
    size_t  mCursor; // bytes
    bool    mLoop;
    
    float       mfFilterAlpha;
    float       mfFilterAlphaDefault;
    short       mfFilterHistIn;
    short       mfFilterHistOut;

    CCyclicBuffer* mpInputCache;
    CCyclicBuffer* mpOutputCache;
};

#endif /* _SNDMAN_H_ */
