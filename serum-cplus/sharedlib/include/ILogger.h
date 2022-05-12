#pragma once

#include <stdarg.h>
#include <string>


class ILogger
{
public:
    virtual ~ILogger() {}

    // template <typename... Params>
    // void Info(const std::string &content, Params... params) { Info(content.c_str(), params...); }
    virtual void Info(const char *content, ...) = 0;

    // template <typename... Params>
    // void Warn(const std::string &content, Params... params) { Warn(content.c_str(), params...); }
    virtual void Warn(const char *content, ...) = 0;

    // template <typename... Params>
    // void Error(const std::string &content, Params... params) { Error(content.c_str(), params...); }
    virtual void Error(const char *content, ...) = 0;

    // template <typename... Params>
    // void Critical(const std::string &content, Params... params) { Critical(content.c_str(), params...); }
    virtual void Critical(const char *content, ...) = 0;

    // template <typename... Params>
    // void Trace(const std::string &content, Params... params) { Trace(content.c_str(), params...); }
    virtual void Trace(const char *content, ...) = 0;

    // template <typename... Params>
    // void Debug(const std::string &content, Params... params) { Debug(content.c_str(), params...); }
    virtual void Debug(const char *content, ...) = 0;
};
