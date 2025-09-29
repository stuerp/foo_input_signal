
/** $VER: CSound.cpp (2025.09.28) P. Stuer - CSound wrapper **/

#include "pch.h"

#include "csound.h"
#include "Resources.h"

/// <summary>
/// Loads and compiles a CSD file.
/// </summary>
void csound_t::Load(const std::string & content)
{
    _CSound.Reset();

    int Result = _CSound.CompileCsdText(content.c_str());

    if (Result != CSOUND_SUCCESS)
        throw exception_io();

    _CSound.SetOutput("null", "raw", "double");     // Override the output defined in the script.

    _Length = _CSound.GetScoreTime();
    _SampleRate = (uint32_t) _CSound.GetSr();       // Number of audio samples in one second.
    _ControlRate = (uint32_t) _CSound.GetKr();      // Number of samples in one control period (k-rate).
    _ChannelCount =  _CSound.GetNchnls();           // Number of audio output channels.
    _0dBFSLevel = _CSound.Get0dBFS();               // 0dBFS level of the spIn/spOut buffers.

    _FrameSize  = _ControlRate * _ChannelCount;
}

/// <summary>
/// Starts rendering.
/// </summary>
void csound_t::Start() noexcept
{
    _CSound.Start();
}

/// <summary>
/// Stops rendering.
/// </summary>
void csound_t::Stop() noexcept
{
    _CSound.Cleanup();

    _CSound.Stop();
}

/// <summary>
/// Renders an audio chunk.
/// </summary>
bool csound_t::Render(audio_chunk & audioChunk) noexcept
{
    if (_CSound.PerformKsmps() != CSOUND_SUCCESS)
        return false;

    const uint32_t FrameCount = 4096;

    audioChunk.set_data_size((t_size) FrameCount * _ChannelCount);

    audio_sample * FrameData = audioChunk.get_data();

    const size_t FramesToRender = _CSound.GetKsmps(); // Audio frames per control sample.
    const size_t SamplesToRender = FramesToRender * _ChannelCount;

    for (int i = 0; i < SamplesToRender; ++i)
    {
        for (int j = 0; j < (int) _ChannelCount; ++j)
        {
            const MYFLT Sample = _CSound.GetSpoutSample(i, j);

            *FrameData++ = Sample;
        }
    }

    size_t FramesRendered = FramesToRender;

    audioChunk.set_srate(_SampleRate);
    audioChunk.set_channels(_ChannelCount);
    audioChunk.set_sample_count(FramesRendered);

   return true;
}
