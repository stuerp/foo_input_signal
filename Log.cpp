
/** $VER: Log.cpp (2025.10.08) P. Stuer - Another logger implementation **/

#include "pch.h"

#include "Log.h"

class null_log_t : public ilog_t
{
public:
    null_log_t() noexcept { };

    null_log_t(const null_log_t &) = delete;
    null_log_t(const null_log_t &&) = delete;
    null_log_t & operator=(const null_log_t &) = delete;
    null_log_t & operator=(null_log_t &&) = delete;

    virtual ~null_log_t() final { };

    ilog_t & SetLevel(LogLevel level) noexcept override final { return *this; }
    LogLevel GetLevel() const noexcept override final { return LogLevel::Never; }

    ilog_t & AtFatal() noexcept override final { return *this; }
    ilog_t & AtError() noexcept override final { return *this; }
    ilog_t & AtWarn() noexcept override final { return *this; }
    ilog_t & AtInfo() noexcept override final { return *this; }
    ilog_t & AtDebug() noexcept override final { return *this; }
    ilog_t & AtTrace() noexcept override final { return *this; }
    ilog_t & Write(const char * format, ... ) noexcept override final { return *this; }
    ilog_t & Write(const char * format, va_list args) noexcept override final { return *this; }
};

static null_log_t _Null;
ilog_t & Null = _Null;

class log_impl_t : public ilog_t
{
public:
#ifdef _DEBUG
    log_impl_t() noexcept { SetLevel(LogLevel::Debug); }
#else
    LogImpl() noexcept { SetLevel(LogLevel::Info); }
#endif

    log_impl_t(const log_impl_t &) = delete;
    log_impl_t(const log_impl_t &&) = delete;
    log_impl_t & operator=(const log_impl_t &) = delete;
    log_impl_t & operator=(log_impl_t &&) = delete;

    virtual ~log_impl_t() noexcept { };

    ilog_t & SetLevel(LogLevel level) noexcept override final
    {
        _Level = level;

        return *this;
    }

    LogLevel GetLevel() const noexcept override final
    {
        return _Level;
    }

    ilog_t & AtFatal() noexcept override final
    {
        return (_Level >= LogLevel::Fatal) ? *this : Null;
    }

    ilog_t & AtError() noexcept override final
    {
        return (_Level >= LogLevel::Error) ? *this : Null;
    }

    ilog_t & AtWarn() noexcept override final
    {
        return (_Level >= LogLevel::Warn) ? *this : Null;
    }

    ilog_t & AtInfo() noexcept override final
    {
        return (_Level >= LogLevel::Info) ? *this : Null;
    }

    ilog_t & AtDebug() noexcept override final
    {
        return (_Level >= LogLevel::Debug) ? *this : Null;
    }

    ilog_t & AtTrace() noexcept override final
    {
        return (_Level >= LogLevel::Trace) ? *this : Null;
    }

    ilog_t & Write(const char * format, ... ) noexcept override final
    {
        char Line[1024] = { };

        va_list args;

        va_start(args, format);

        (void) ::vsnprintf(Line, sizeof(Line) - 1, format, args);
        console::print(Line);

        return *this;
    }

    ilog_t & Write(const char * format, va_list args) noexcept override final
    {
        char Line[1024] = { };

        (void) ::vsnprintf(Line, sizeof(Line) - 1, format, args);
        console::print(Line);

        return *this;
    }

private:
    LogLevel _Level;
};

static log_impl_t _LogImpl;
ilog_t & Log = _LogImpl;

cfg_var_modern::cfg_int CfgLogLevel({ 0x1ead47d9, 0xc4aa, 0x4079, { 0x95, 0xfa, 0xaf, 0x51, 0x6a, 0xaf, 0xd8, 0xb6 } }, LogLevel::Debug);
