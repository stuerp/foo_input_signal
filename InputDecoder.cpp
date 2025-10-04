 
/** $VER: InputDecoder.cpp (2025.09.29) P. Stuer **/

#include "pch.h"

#include <CppCoreCheck/Warnings.h>

#pragma warning(disable: 4625 4626 4710 4711 5045 ALL_CPPCORECHECK_WARNINGS)

#include <sdk/input_impl.h>
#include <sdk/input_file_type.h>
#include <sdk/file_info_impl.h>
#include <sdk/tag_processor.h>

#include "Resources.h"
#include "Log.h"

#include "csound.h"

#pragma hdrstop

/// <summary>
/// Implements an input decoder.
/// </summary>
#pragma warning(disable: 4820) // x bytes padding added after last data member
class InputDecoder : public input_stubs
{
public:
    InputDecoder() noexcept : _File(), _FilePath(), _FileStats()
    {
    }

    InputDecoder(const InputDecoder &) = delete;
    InputDecoder(InputDecoder &&) = delete;
    InputDecoder& operator=(const InputDecoder &) = delete;
    InputDecoder& operator=(InputDecoder &&) = delete;

    virtual ~InputDecoder() noexcept
    {
    }

public:
    #pragma region input_impl

    /// <summary>
    /// Opens the specified file and parses it.
    /// </summary>
    void open(service_ptr_t<file> file, const char * filePath, t_input_open_reason reason, abort_callback & abortHandler)
    {
        if (reason == input_open_info_write)
            throw exception_tagging_unsupported(); // Decoder does not support retagging.

        _File = file;
        _FilePath = filePath;

        input_open_file_helper(_File, filePath, reason, abortHandler);

        {
            _FileStats = _File->get_stats(abortHandler);

            if (_FileStats.m_size == 0)
                throw exception_io_unsupported_format("Invalid file size");
        }

        {
            pfc::array_t<char> Data;

            Data.resize((size_t) _FileStats.m_size);

            _File->read_object(Data.get_ptr(), Data.get_size(), abortHandler);

            _Script = Data.get_ptr();

            _CSound.Load(_Script);
        }

        Log.AtInfo().Write(STR_COMPONENT_NAME " is using Csound %s.", _CSound.GetVersion().c_str());
    }

    static bool g_is_our_content_type(const char * contentType)
    {
        return ::stricmp_utf8(contentType, "audio/csd") == 0;
    }

    static bool g_is_our_path(const char *, const char * extension)
    {
        return (::stricmp_utf8(extension, "csd") == 0);
    }

    static GUID g_get_guid()
    {
        return { 0xd52b21c5, 0xef04, 0x43d5, { 0xb7, 0xe7, 0xfe, 0x17, 0x36, 0xc0, 0x9d, 0x5d } };
    }

    static const char * g_get_name()
    {
        return STR_COMPONENT_NAME;
    }

    static GUID g_get_preferences_guid()
    {
        return { 0xa2a5b8e0, 0x6dcc, 0x4bcb, { 0x82, 0xcc, 0x98, 0x77, 0x42, 0x50, 0xb9, 0x2 } };;
    }

    static bool g_is_low_merit()
    {
        return false;
    }

    #pragma endregion

    #pragma region input_info_reader

    unsigned get_subsong_count()
    {
        return 1;
    }

    t_uint32 get_subsong(unsigned subSongIndex)
    {
        return subSongIndex;
    }

    /// <summary>
    /// Retrieves information about specified subsong.
    /// </summary>
    void get_info(t_uint32, file_info & fileInfo, abort_callback &)
    {
        fileInfo.set_length(0.); // Sets audio duration, in seconds (infinite)

        // General info tags
        fileInfo.info_set("encoding", "Synthesized");

        fileInfo.info_set_int("fis_control_rate", _CSound._ControlRate);
        fileInfo.info_set_int("fis_channel_count", _CSound._ChannelCount);
        fileInfo.info_set_int("fis_0dbfs_level", (int64_t) _CSound._0dBFSLevel);
/*
        // Meta data tags
        fileInfo.meta_add("title", _Decoder->GetTitle());
        fileInfo.meta_add("artist", _Decoder->GetArranger());
        fileInfo.meta_add("composer", _Decoder->GetComposer());
        fileInfo.meta_add("memo", _Decoder->GetMemo());
*/
    }

    #pragma endregion

    #pragma region input_info_reader_v2

    t_filestats2 get_stats2(uint32_t stats, abort_callback & abortHandler)
    {
        return _File->get_stats2_(stats, abortHandler);
    }

    t_filestats get_file_stats(abort_callback &)
    {
        return _FileStats;
    }

    #pragma endregion

    #pragma region input_info_writer

    /// <summary>
    /// Set the tags for the specified file.
    /// </summary>
    void retag_set_info(t_uint32, const file_info &, abort_callback &)
    {
        throw exception_tagging_unsupported();
    }

    void retag_commit(abort_callback &)
    {
        throw exception_tagging_unsupported();
    }

    void remove_tags(abort_callback &)
    {
        throw exception_tagging_unsupported();
    }

    #pragma endregion

    #pragma region input_decoder

    /// <summary>
    /// Initializes the decoder before playing the specified subsong. Resets playback position to the beginning of specified subsong.
    /// </summary>
    void decode_initialize(unsigned, unsigned, abort_callback & abortHandler)
    {
        abortHandler.check();

        _File->reopen(abortHandler); // Equivalent to seek to zero, except it also works on nonseekable streams

        _CSound.Start();
    }

    /// <summary>
    /// Reads/decodes one chunk of audio data.
    /// </summary>
    bool decode_run(audio_chunk & audioChunk, abort_callback & abortHandler)
    {
        abortHandler.check();

        return _CSound.Render(audioChunk);
    }

    /// <summary>
    /// Seeks to the specified time offset.
    /// </summary>
    void decode_seek(double timeInSeconds, abort_callback & abortHandler)
    {
        abortHandler.check();
    }

    /// <summary>
    /// Returns true if the input decoder supports seeking.
    /// </summary>
    bool decode_can_seek() noexcept
    {
        return true;
    }

    /// <summary>
    /// Returns dynamic VBR bitrate etc...
    /// </summary>
    bool decode_get_dynamic_info(file_info & fileInfo, double &) noexcept
    {
        bool IsDynamicInfoUpdated = false;

        if (!_IsDynamicInfoSet)
        {
            fileInfo.info_set_int("sample_rate", _CSound._SampleRate);

//          fileInfo.info_set_bitrate(((t_int64) _Decoder->GetBitsPerSample() * _Decoder->GetChannelCount() * _SynthesisRate + 500 /* rounding for bps to kbps*/) / 1000 /* bps to kbps */);

            _IsDynamicInfoSet = true;

            IsDynamicInfoUpdated = true;
        }

        return IsDynamicInfoUpdated;
    }

    /// <summary>
    /// Deals with dynamic information such as track changes in live streams.
    /// </summary>
    bool decode_get_dynamic_info_track(file_info &, double &) noexcept
    {
        return false;
    }

    void decode_on_idle(abort_callback & abortHandler)
    {
        _File->on_idle(abortHandler);
    }

    #pragma endregion

private:
    service_ptr_t<file> _File;
    pfc::string8 _FilePath;
    t_filestats _FileStats;

    csound_t _CSound;
    std::string _Script;
    uint32_t _SynthesisRate;

    // Dynamic track info
    uint32_t _LoopNumber;

    bool _IsDynamicInfoSet;
};
#pragma warning(default: 4820) // x bytes padding added after last data member

// Declare the supported file types to make it show in "open file" dialog etc.
DECLARE_FILE_TYPE("Csound Documents (CSD)", "*.csd");

static input_factory_t<InputDecoder> _InputDecoderFactory;
