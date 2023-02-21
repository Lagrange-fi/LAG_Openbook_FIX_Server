//
// Created by Zay on 2/19/2023.
//

#pragma once

#include <sharedlib/include/ILogger.h>

class ConsoleLogger: public ILogger
{
private:

    typedef std::string string;

public:

    void Info(const char *content, ...) override;
    void Debug(const char *content, ...) override;
    void Error(const char *content, ...) override;
    void Critical(const char *content, ...) override;
    void Warn(const char *content, ...) override;
    void Trace(const char *content, ...) override;

    ~ConsoleLogger() = default;
};
