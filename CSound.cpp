
/** $VER: CSound.cpp (2025.10.04) P. Stuer - CSound wrapper **/

#include "pch.h"

#include "CSound.h"

#include "Resources.h"
#include "Log.h"

/// <summary>
/// Initializes this instance.
/// </summary>
csound_t::csound_t() noexcept : _SampleRate(), _ControlRate(), _ChannelCount(), _SrcData()
{
    static_assert(sizeof(audio_sample) == sizeof(MYFLT), "sizeof(audio_sample) != sizeof(MYFLT)");

    _CSound.SetHostData(this);

    _CSound.SetMessageLevel(0); // All messages

    _CSound.SetMessageCallback([](CSOUND * csound, int attr, const char * format, va_list args)
    {
        auto This = (csound_t *) ::csoundGetHostData(csound);

        char Part[1024] = { };

        (void) ::vsnprintf(Part, sizeof(Part) - 1, format, args);

        for (const char c : Part)
        {
            if (c == '\0')
                break;

            if (c == '\n')
            {
                Log.AtInfo().Write("Csound: %s", This->_Line.c_str());
                This->_Line.clear();
            }
            else
                This->_Line += c;
        }
    });
}

/// <summary>
/// Loads and compiles a CSD file.
/// </summary>
void csound_t::Load(const std::string & content)
{
    int Result = _CSound.SetOption("-o null");      // Override the output option in the CSD.

    if (Result != CSOUND_SUCCESS)
        throw exception_io("Failed to set Csound option");

    Result = _CSound.CompileCSD(content.c_str(), 1, 0);

    if (Result != CSOUND_SUCCESS)
        throw exception_io("Failed to compile Csound Document");

    _SampleRate = (uint32_t) _CSound.GetSr();           // Number of audio samples in one second.
    _ControlRate = (uint32_t) _CSound.GetKr();          // Number of samples in one control period (k-rate).
    _ChannelCount = (uint32_t) _CSound.GetChannels(0);  // Number of audio output channels.
    _0dBFSLevel = _CSound.Get0dBFS();                   // 0dBFS level of the spIn/spOut buffers.

    _FramesPerControlCycle  = (size_t) _CSound.GetKsmps();
    _SamplesPerControlCycle = _FramesPerControlCycle * _ChannelCount;

    // Make sure the audio chunk is large enough to hold all samples of a control cycle.
    _FramesPerChunk = ((512 + _FramesPerControlCycle - 1) / _FramesPerControlCycle) * _FramesPerControlCycle;
}

/// <summary>
/// Starts rendering.
/// </summary>
void csound_t::Start() noexcept
{
    _CSound.Start();

    _SrcData = _CSound.GetSpout();
}

/// <summary>
/// Stops rendering.
/// </summary>
void csound_t::Stop() noexcept
{
    _SrcData = nullptr;

    _CSound.Reset();

    if (!_Line.empty())
    {
        Log.AtInfo().Write("Csound: %s", _Line.c_str());
        _Line.clear();
    }
}

/// <summary>
/// Renders an audio chunk.
/// </summary>
bool csound_t::Render(audio_chunk & audioChunk) noexcept
{
    if (_SrcData == nullptr)
        return false;

    bool KeepRendering = true;

    audioChunk.set_data_size((t_size) (_FramesPerChunk + 1) * _ChannelCount); // Set the number of samples in the audio chunk. Add room for 1 extra frame of silence.

    audio_sample * DstData = audioChunk.get_data();

    size_t FramesRendered = 0;

    while (FramesRendered < _FramesPerChunk)
    {
        auto Result = _CSound.PerformKsmps();

        ::memcpy(DstData,_SrcData, _SamplesPerControlCycle * sizeof(*DstData));

        DstData        += _SamplesPerControlCycle;
        FramesRendered += _FramesPerControlCycle;

        if (Result != CSOUND_SUCCESS)
        {
            // Add one frame of silence.
            ::memset(DstData, 0, _ChannelCount * sizeof(*DstData));
            ++FramesRendered;

            KeepRendering = false;
            break;
        }
    }

    audioChunk.set_srate(_SampleRate);
    audioChunk.set_channels(_ChannelCount);         // Set the number of channels in the audio chunk.
    audioChunk.set_sample_count(FramesRendered);    // Set the number of samples per channel in the audio chunk (= number of frames).

   return KeepRendering;
}
