#include <iostream>
#include <cstdarg>
#include <cstdlib>
#include "Logger.h"

int Logger::printFormattedOutput(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

void Logger::handleError(const std::string &message)
{
    std::cerr << message << std::endl;
    // TODO throwing an exception or returning an error code instead of exiting
    exit(1);
}
