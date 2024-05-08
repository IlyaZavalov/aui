//
// Created by dervisdev on 2/9/2023.
//

#pragma once

#include "AUI/IO/AFileInputStream.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

class ISeekableInputStream;
class AUrl;

class OggVorbisFileImpl;


/**
 * @brief Sound stream for OGG format
 * @ingroup audio
 */
class API_AUI_AUDIO AOggSoundStream: public ISoundInputStream {
public:
    static constexpr ASampleFormat SAMPLE_FORMAT = ASampleFormat::I16;

    explicit AOggSoundStream(AUrl url);

    explicit AOggSoundStream(_<IInputStream> stream);

    ~AOggSoundStream() override = default;

    AAudioFormat info() override;

    size_t read(char* dst, size_t size) override;

    void rewind() override;

    static _<AOggSoundStream> fromUrl(AUrl url);

private:
    AOptional<AUrl> mUrl;
    AOptional<aui::fast_pimpl<OggVorbisFileImpl, 960>> mVorbisFile;
};
