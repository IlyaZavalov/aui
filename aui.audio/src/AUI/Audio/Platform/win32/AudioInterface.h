//
// Created by Alex2772 on 2/9/2022.
//

#pragma once

#include "AUI/Platform/AProgramModule.h"
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm" )

#include <utility>
#include <dsound.h>
#include "AUI/Traits/memory.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Audio/AAuidoPlayer.h"
#include "AUI/Audio/Sound/ISoundStream.h"

#define ASSERT_OK AssertOkHelper{} +
struct AssertOkHelper {
    void operator+(HRESULT r) const {
        assert(r == DS_OK);
    }
};

class DirectSound {
public:
    static IDirectSound* instance() {
        static DirectSound ds;
        return ds.mDirectSound;
    }

private:
    DirectSound() {
        ASSERT_OK DirectSoundCreate8(nullptr, &mDirectSound, nullptr);
        auto w = dynamic_cast<AWindow*>(AWindow::current());
        auto handle = w->nativeHandle();
        ASSERT_OK mDirectSound->SetCooperativeLevel(handle, DSSCL_PRIORITY);
    }

    ~DirectSound() {
        AUI_NULLSAFE(mDirectSound)->Release();
    }

    IDirectSound8* mDirectSound;
};



class AudioInterface {
public:
    AudioInterface(_<ISoundStream> sound) : mSound(std::move(sound)){
        setupSecondaryBuffer();
    }

    bool release() {
        mSoundBufferInterface->Release();
        mNotifyInterface->Release();
        return true;
    }

    bool play() {
        if (mIsPlaying) {
            stop();
        }
        setupReachPointEvents();
        setupBufferThread();
        uploadNextBlock(BUFFER_DURATION_SEC);
        resume();
        mIsPlaying = true;
        return true;
    }

    bool stop() {
        ASSERT_OK mSoundBufferInterface->Stop();

        while (mThreadIsActive) {
            for (int i = 0; i < BUFFER_DURATION_SEC; i++)
                ResetEvent(mEvents[i]);
            SetEvent(mEvents[BUFFER_DURATION_SEC]);
        }

        for (auto& mEvent : mEvents)
            CloseHandle(mEvent);
        CloseHandle(mThread);

        clearBuffer();
        rewind();
        return true;
    }

    bool pause() {
        ASSERT_OK mSoundBufferInterface->Stop();
        mIsPlaying = false;
        return true;
    }

    bool resume() {
        ASSERT_OK mSoundBufferInterface->Play(0, 0, DSBPLAY_LOOPING);
        mIsPlaying = true;
        return true;
    }

    bool rewind() {
        ASSERT_OK mSoundBufferInterface->SetCurrentPosition(0);
        mSound->rewind();
        return true;
    }

private:
    static constexpr int BUFFER_DURATION_SEC = 2;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");

    HANDLE mEvents[BUFFER_DURATION_SEC + 1];
    DSBPOSITIONNOTIFY mNotifyPositions[BUFFER_DURATION_SEC];
    HANDLE mThread;
    bool mThreadIsActive = false;

    IDirectSoundBuffer8* mSoundBufferInterface;
    IDirectSoundNotify8* mNotifyInterface;
    _<ISoundStream> mSound;

    bool mIsPlaying = false;
    int mBytesPerSecond;

    void uploadNextBlock(DWORD reachedPointIndex) {
        LPVOID buffer;
        DWORD bufferSize = mBytesPerSecond;
        DWORD offset = ((reachedPointIndex + 1) % BUFFER_DURATION_SEC) * mBytesPerSecond;

        HRESULT result = mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        if (result == DSERR_BUFFERLOST) {
            mSoundBufferInterface->Restore();
            ASSERT_OK mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        }

        bufferSize = mSound->read(static_cast<char*>(buffer), bufferSize);
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
        if (bufferSize == 0) {
            std::memset(buffer, 0, mBytesPerSecond - bufferSize);
            stop();
        } else if (bufferSize < mBytesPerSecond) {
            stop();
        }
    }

    void clearBuffer() {
        LPVOID buffer;
        DWORD bufferSize = BUFFER_DURATION_SEC * mBytesPerSecond;
        ASSERT_OK mSoundBufferInterface->Lock(0, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        std::memset(buffer, 0, bufferSize);
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    }

    void setupBufferThread() {
        DWORD threadId;
        mThread = CreateThread(nullptr,
                               0,
                               (LPTHREAD_START_ROUTINE) bufferThread,
                               (void *) this,
                               0,
                               &threadId);
    }

    [[noreturn]] static DWORD WINAPI bufferThread(void *lpParameter) {
        auto audio = reinterpret_cast<AudioInterface*>(lpParameter);
        while(true) {
            audio->onAudioReachCallbackPoint();
        }
    }

    void onAudioReachCallbackPoint() {
        DWORD waitResult;
        waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
        while(waitResult != WAIT_FAILED) {
            DWORD eventIndex = waitResult - WAIT_OBJECT_0;
            if (eventIndex != BUFFER_DURATION_SEC) {
                uploadNextBlock(eventIndex);
                waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
                continue;
            }

            mIsPlaying = false;
            mThreadIsActive = false;
            ExitThread(0);
        }
    }

    void setupReachPointEvents() {
        ASSERT_OK mSoundBufferInterface->QueryInterface(IID_IDirectSoundNotify8, (void**) &mNotifyInterface);
        mEvents[BUFFER_DURATION_SEC] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        for (int i = 0; i < BUFFER_DURATION_SEC; i++) {
            mEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            mNotifyPositions[i].hEventNotify = mEvents[i];
            mNotifyPositions[i].dwOffset = i * mBytesPerSecond + mBytesPerSecond / 2;
        }

        ASSERT_OK (mNotifyInterface->SetNotificationPositions(BUFFER_DURATION_SEC, mNotifyPositions));
    }

    void setupSecondaryBuffer() {
        DSBUFFERDESC format;
        WAVEFORMATEX waveFormat;
        IDirectSoundBuffer* buffer;

        auto info = mSound->info();
        waveFormat.cbSize = sizeof(waveFormat);
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.wBitsPerSample = info.bitsPerSample;
        waveFormat.nChannels = info.channelCount;
        waveFormat.nSamplesPerSec = info.sampleRate;
        waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);

        mBytesPerSecond = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.nAvgBytesPerSec = mBytesPerSecond;

        aui::zero(format);
        format.dwSize = sizeof(format);
        format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
        format.dwBufferBytes = BUFFER_DURATION_SEC * mBytesPerSecond;
        format.lpwfxFormat = &waveFormat;

        ASSERT_OK DirectSound::instance()->CreateSoundBuffer(&format, &buffer, nullptr);
        ASSERT_OK buffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&mSoundBufferInterface));
        buffer->Release();
    }
};