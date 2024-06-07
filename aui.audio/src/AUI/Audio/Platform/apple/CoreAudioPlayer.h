#pragma once

#include "AUI/Audio/IAudioPlayer.h"

class CoreAudioPlayer : public IAudioPlayer {
public:
    using IAudioPlayer::IAudioPlayer;
private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onLoopSet() override;
    void onVolumeSet() override;
};
