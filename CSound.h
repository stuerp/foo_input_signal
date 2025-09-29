 
/** $VER: CSound.cpp (2025.09.28) P. Stuer - CSound wrapper **/

#pragma once

#include <csound.hpp>

#include <libmsc.h>

class csound_t
{
public:
    csound_t() noexcept : _SampleRate(), _ControlRate(), _ChannelCount(), _FrameSize() { }

    void Load(const std::string & content);

    void Start() noexcept;
    bool Render(audio_chunk & audioChunk) noexcept;
    void Stop() noexcept;

    std::string GetVersion() noexcept
    {
        int Version = _CSound.GetVersion();

        return msc::FormatText("%d.%d.%d", Version / 1000, (Version % 1000) / 10, Version % 10);
    }

public:
    uint32_t _SampleRate;
    uint32_t _ControlRate;
    uint32_t _ChannelCount;
    double _0dBFSLevel;

    uint32_t _FrameSize;

private:
    Csound _CSound;
};
