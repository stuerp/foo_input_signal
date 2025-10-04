 
/** $VER: CSound.cpp (2025.10.04) P. Stuer - CSound wrapper **/

#pragma once

#include <csound.hpp>

#include <libmsc.h>

class csound_t
{
public:
    csound_t() noexcept;
    virtual ~csound_t() { }

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

    size_t _FramesPerControlCycle;  // Number of audio frames per control cycle.
    size_t _SamplesPerControlCycle; // Number of samples per control cycle.
    size_t _FramesPerChunk;

private:
    Csound _CSound;

    std::string _Line;
    const MYFLT * _SrcData;
};
