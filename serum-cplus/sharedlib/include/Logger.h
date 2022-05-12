#pragma once

#include <P7_Trace.h>
#include <P7_Telemetry.h>
#include <memory>
#include <string>
#include <boost/format.hpp>

enum LogLevel
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL,
};

#define LOG_TRACE(...) DoTrace(LOG_LEVEL_TRACE, \
                               __LINE__,        \
                               __FILE__,        \
                               __FUNCTION__,    \
                               __VA_ARGS__)

#define LOG_DEBUG(...) DoTrace(LOG_LEVEL_DEBUG, \
                               __LINE__,        \
                               __FILE__,        \
                               __FUNCTION__,    \
                               __VA_ARGS__)

#define LOG_INFO(...) DoTrace(LOG_LEVEL_INFO, \
                              __LINE__,       \
                              __FILE__,       \
                              __FUNCTION__,   \
                              __VA_ARGS__)

#define LOG_WARN(...) DoTrace(LOG_LEVEL_WARNING, \
                              __LINE__,          \
                              __FILE__,          \
                              __FUNCTION__,      \
                              __VA_ARGS__)

#define LOG_ERROR(...) DoTrace(LOG_LEVEL_ERROR, \
                               __LINE__,        \
                               __FILE__,        \
                               __FUNCTION__,    \
                               __VA_ARGS__)

#define LOG_CRITICAL(...) DoTrace(LOG_LEVEL_CRITICAL, \
                                  __LINE__,           \
                                  __FILE__,           \
                                  __FUNCTION__,       \
                                  __VA_ARGS__)

class Logger;
typedef std::shared_ptr<Logger> LoggerPtr;

class Logger
{
public:
    ~Logger()
    {
        if (!m_pTrace)
            return;

        m_pTrace->Unregister_Thread(0);

        m_pTrace->Release();
        m_pTrace = nullptr;

        if (!m_pClient)
            return;

        m_pClient->Release();
        m_pClient = nullptr;
    }

    static LoggerPtr Create(const std::string &channel_name, bool log_to_stdout = true, LogLevel level_to_stdout = LOG_LEVEL_INFO)
    {
        auto pLogger = new Logger(channel_name, log_to_stdout, level_to_stdout);
        return std::shared_ptr<Logger>(pLogger);
    }

    template <typename... Params>
    inline void DoTrace(LogLevel log_level,
                        unsigned short line,
                        const char *file,
                        const char *function,
                        const char *format,
                        Params &&... params)
    {
        if (m_log_to_stdout && log_level>= m_level_to_stdout )
        {
            static const char * logLevelStr[]=
                    {
                            "TRACE",
                            "DEBUG",
                            "INFO",
                            "WARNING",
                            "ERROR",
                            "CRITICAL",
                    };
            char buff[64*8];
            sprintf(buff, format, params...);
            printf("%s | %s | %s \n", timestamp().c_str(), logLevelStr[(int)log_level], buff);
        }

        m_pTrace->Trace(/*i_wTraceID:*/ 0,
                        (eP7Trace_Level)log_level,
                        m_hModule,
                        (tUINT16)line,
                        file,
                        function,
                        format,
                        params...);
    }

private:
    Logger(const std::string &channel_name, bool log_to_stdout = true, LogLevel level_to_stdout = LOG_LEVEL_INFO)
        : m_log_to_stdout(log_to_stdout), m_level_to_stdout(level_to_stdout)
    {
        P7_Set_Crash_Handler();

        // Create a P7 client object
        m_pClient = P7_Create_Client(TM("/P7.Sink=FileTxt /P7.Pool=32768"));
        if (nullptr == m_pClient)
            throw std::runtime_error{"could not create a P7 client"};

        // Create a P7 trace object
        m_pTrace = P7_Create_Trace(m_pClient, channel_name.c_str());
        if (nullptr == m_pTrace)
            throw std::runtime_error{"could not create a P7 trace"};

        m_pTrace->Register_Module(channel_name.c_str(), &m_hModule);
    }

    const bool m_log_to_stdout = true;
    const LogLevel m_level_to_stdout = LOG_LEVEL_DEBUG;
    IP7_Client *m_pClient = nullptr;
    IP7_Trace *m_pTrace = nullptr;
    IP7_Trace::hModule m_hModule = nullptr;

    std::string timestamp() const
    {
        auto now = std::chrono::system_clock::now();

        auto ticks = now.time_since_epoch().count();
        auto total_millisecs = ticks / 1000000;
        auto current_millisecs = total_millisecs % 1000;

        auto tt = std::chrono::system_clock::to_time_t(now);
        auto lt = localtime(&tt);

        auto ts = (boost::format{"%d/%d/%d %02d:%02d:%02d.%03d"}
                        % (lt->tm_year + 1900) % (lt->tm_mon + 1) % lt->tm_mday 
                        % lt->tm_hour % lt->tm_min % lt->tm_sec % current_millisecs)
                  .str();

        return ts;
    }
};
