#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <cstdarg>
#include <string>

class Logger
{
public:
    static int printFormattedOutput(FILE *stream, const char *format, ...);
    static void handleError(const std::string &message);
};

#endif // LOGGER_H
