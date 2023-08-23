#pragma once

#include "AUI/IO/AFileInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "ISoundStream.h"

class WavSoundStream: public ISoundStream {
public:
    explicit WavSoundStream(_<AFileInputStream> is);

    AAudioFormat info() override;

    void rewind() override;

    size_t read(char* dst, size_t size) override;

    static _<WavSoundStream> load(_<AFileInputStream> is);

private:
    struct WavFileHeader {
        char    chunkID[4];
        int32_t chunkSize;
        char    format[4];

        char    subchunk1ID[4];
        int32_t subchunk1Size;
        int16_t audioFormat;
        int16_t numChannels;
        int32_t sampleRate;
        int32_t byteRate;
        int16_t blockAlign;
        int16_t bitsPerSample;

        char    subchunk2ID[4];
        int32_t subchunk2Size;
    };

    _<AFileInputStream> mFis;
    WavFileHeader mHeader;
    size_t mChunkReadPos = 0; // до mHeader.ChunkSize
};

//TODO replace AFileInputStream with abstract stream with seek() and tell() methods
