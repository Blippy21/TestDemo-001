//
//  sndMan.cpp
//  VulkanTestDemo002
//
//  Created by Ian Minett on 1/5/25.
//

#include "sndMan.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

#define ENABLE_RAW_OUT_LOAD     (0)
#define ENABLE_RAW_OUT_STREAM   (0)

std::vector<const char*> waveAssets =
{
    "musicA_1_s16_44.wav"
};

CSoundManager::CSoundManager() :
    mCurrWave( -1 )
{
#if ENABLE_RAW_OUT_STREAM
    FILE* f = fopen("TAP_AUDIO_WAVESTREAM.raw", "wb" );
                
    if( f )
    {
        fclose( f );
    }
#endif

    LoadWaveFiles();

    PlaySound(0);
}

CSoundManager::~CSoundManager()
{

}


int CSoundManager::PlaySound( unsigned int ix )
{
    if( mWaveFiles.size() > ix )
    {
        WaveFileInfo wfi = mWaveFiles[ix];

        AudioStreamBasicDescription strDesc{};
        
        strDesc.mSampleRate         = wfi.freq;
        strDesc.mChannelsPerFrame   = wfi.channels;
        strDesc.mBitsPerChannel     = wfi.bps;
        strDesc.mFramesPerPacket    = 1;
        strDesc.mBytesPerFrame      = (wfi.channels * wfi.bps) >> 3;
        strDesc.mBytesPerPacket     = strDesc.mBytesPerFrame *
                                        strDesc.mFramesPerPacket;
        strDesc.mFormatID           = kAudioFormatLinearPCM;
        strDesc.mFormatFlags        = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;

        AudioQueueRef aq = 0;
        OSStatus err = AudioQueueNewOutput( &strDesc,
                                            CSoundManager::playbackCBStub,
                                            this,
                                            0, 0, 0,
                                            &aq );

        mCurrWave   = ix;
        mCursor     = 0;
        mLoop       = true;
        
#if ENABLE_RAW_OUT_STREAM
        FILE* f = fopen("TAP_AUDIO_WAVESTREAMBULK.raw", "wb" );

        if( f )
        {
            fwrite( mData, 1, mDataSize, f );

            fclose( f );
        }
#endif

        AllocQueueBuffers( aq );

        InitFilter( wfi.freq, 100 );

        int numBuffs = sizeof(mAqb)/sizeof(mAqb[0]);

        for( int i = 0; i < numBuffs; ++i )
        {
            FillBuffer( aq, mAqb[i] );
        }

        AudioQueueStart( aq, 0 );
    }

    return 0;
}


void CSoundManager::LoadWaveFiles( void )
{
    int i = 0;

    for( auto wave : waveAssets )
    {
        WaveFileDesc fIn{};
        WaveFileInfo wStore{};

        std::ifstream file( wave, std::ios::binary );

        if( !file.is_open() )
        {
            throw std::runtime_error( "Failed to open file!" );
        }

        file.read( (char*)&fIn, sizeof(fIn) );

        // data
        if( fIn.WaveData.ChunkSize > 0 )
        {
            wStore.data.resize(fIn.WaveData.ChunkSize);

            file.read( wStore.data.data(), fIn.WaveData.ChunkSize );
        }

        if( wStore.data.size() > 0 )
        {
            wStore.ix       = i++;
            wStore.channels = fIn.WaveHeader.WaveChannels;
            wStore.freq     = fIn.WaveHeader.WaveFreq;
            wStore.bps      = fIn.WaveHeader.WaveBitsPerSample;
            wStore.isFloat  = fIn.WaveHeader.WaveFormat == 0x0003;

            mWaveFiles.push_back(wStore);

#if ENABLE_RAW_OUT_LOAD
            FILE* f = fopen("TAP_AUDIO_WAVELOAD.raw", "wb" );
            
            if( f )
            {
                fwrite( wStore.data.data(), 1, wStore.data.size(), f );
                
                fclose( f );
            }
#endif
        }

        file.close();
    }
}


void CSoundManager::playbackCBStub( void* userData, AudioQueueRef aq, AudioQueueBufferRef aqb )
{
    CSoundManager* pObj = static_cast<CSoundManager*>(userData);

    if( pObj )
    {
        pObj->playbackCB( aq, aqb );
    }

    return;
}

void CSoundManager::playbackCB( AudioQueueRef aq, AudioQueueBufferRef aqb )
{
    FillBuffer( aq, aqb );

    return;
}

void CSoundManager::AllocQueueBuffers( AudioQueueRef aq )
{
    unsigned int buffSize = (unsigned int)(44100 * 100) / 1000;
//  unsigned int buffSize = (unsigned int)(44100 * 2);
    OSStatus err = 0;

    int numBuffs = sizeof(mAqb)/sizeof(mAqb[0]);

    for( int i = 0; i < numBuffs; ++i )
    {
        if( mAqb[i] )
        {
            AudioQueueFreeBuffer( aq, mAqb[0] );
            mAqb[0] = nullptr;
        }

        err = AudioQueueAllocateBuffer( aq, buffSize, &mAqb[i] );
        if( err )
        {
            throw std::runtime_error( "Failed to create AudioQueueBuffer!" );
        }
    }
}

void CSoundManager::FillBuffer( AudioQueueRef aq, AudioQueueBufferRef aqb )
{
    OSStatus err = 0;

    if( !aq || !aqb )
        return;

    char* inData = (char*)mWaveFiles[mCurrWave].data.data();
    size_t inDataSize = mWaveFiles[mCurrWave].data.size();
    
    char* psSrcData = inData + mCursor;
    char* psDstData = (char*)aqb->mAudioData;

    unsigned int buffSize = aqb->mAudioDataBytesCapacity;
       
    size_t bytesRem = inDataSize - mCursor;
    size_t bytesToCopy = bytesRem > buffSize ? buffSize : bytesRem;

    if( bytesToCopy && psSrcData && psDstData )
    {
        memset( psDstData, 0, buffSize );

        memcpy( psDstData, psSrcData, bytesToCopy );

        ProcessFilter( psDstData, bytesToCopy );

        mCursor += bytesToCopy;

#if ENABLE_RAW_OUT_STREAM
        FILE* f = fopen("TAP_AUDIO_WAVESTREAM.raw", "ab" );

        if( f )
        {
            fwrite( psSrcData, 1, bytesToCopy, f );

            fclose( f );
        }
#endif

        aqb->mAudioDataByteSize = buffSize;

        err = AudioQueueEnqueueBuffer( aq, aqb, 0, 0 );

        if( err )
        {
            std::cout << "Received error from Enqueue: " << err << std::endl;
        }
    }

    if( mCursor >= inDataSize )
    {
        if( mLoop )
        {
            mCursor = 0;
        }
        else
        {
            AudioQueueStop( aq, 0 );
        }
    
    }
}

void CSoundManager::InitFilter( size_t freq, size_t cutOffFreq )
{
    float rc = 1.0f / (2.0f * M_PI * cutOffFreq);
    float dt = 1.0f / freq;

    mfFilterAlpha   = dt / (rc + dt);
    mfFilterHistIn  = 0;
    mfFilterHistOut = 0;

}

void CSoundManager::ProcessFilter ( char* data, int dataLen )
{
    // y[n] = y[n−1] + α(x[n]−y[n−1])

    short* psData = (short*)data;
    int numSamples = dataLen / sizeof(short);

    if( numSamples )
    {
        psData[0] = mfFilterHistOut + (mfFilterAlpha * (psData[0] - mfFilterHistOut));

        for( int i = 1; i < numSamples; ++i )
        {
            psData[i] = psData[i-1] + (mfFilterAlpha * (psData[i] - psData[i-1]));
        }

        mfFilterHistOut = psData[numSamples-1];
    }
}
