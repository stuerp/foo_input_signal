
/** $VER: CSound.cpp (2025.09.29) P. Stuer - CSound wrapper **/

#include "pch.h"

#include "CSound.h"

#include "Resources.h"
#include "Log.h"

/// <summary>
/// Initializes this instance.
/// </summary>
csound_t::csound_t() noexcept : _SampleRate(), _ControlRate(), _ChannelCount(), _FrameSize()
{
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
    int Result = _CSound.CompileCsdText(content.c_str());

    if (Result != CSOUND_SUCCESS)
        throw exception_io("Failed to compile Csound Document");

    _CSound.SetOutput("null", "raw", "double");     // Override the output defined in the script.

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
