#pragma once

#include <iostream>
#include <string>
#include <format>
#include <chrono>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Logger {

#ifdef _WIN32
    inline WORD InfoColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;   // cyan
    inline WORD WarnColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;    // yellow
    inline WORD ErrorColor = FOREGROUND_RED | FOREGROUND_INTENSITY;                      // red
#else
    inline constexpr const char* RESET = "\033[0m";
    inline constexpr const char* RED = "\033[31m";
    inline constexpr const char* YELLOW = "\033[33m";
    inline constexpr const char* CYAN = "\033[36m";
#endif

    inline std::string CurrentTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char buf[20];

#ifdef _WIN32
        std::tm tm_struct;
        localtime_s(&tm_struct, &now_time);
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_struct);
#else
        std::tm* tm_struct = std::localtime(&now_time);
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_struct);
#endif

        return std::string(buf);
    }

#ifdef _WIN32
    inline void SetConsoleColor(WORD color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }
#endif

    inline const char* BaseFileName(const char* path) {
        const char* file = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '/' || *p == '\\') file = p + 1;
        }
        return file;
    }

    template<typename... Args>
    void LogInternal(const char* file, const char* function, int line,
        const std::string& level, const std::string& color,
        const std::string& format_str, Args&&... args)
    {
        std::string message = std::vformat(format_str, std::make_format_args(args...));

#ifdef _WIN32
        WORD colorCode = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (level == "INFO")  colorCode = InfoColor;
        if (level == "WARN")  colorCode = WarnColor;
        if (level == "ERROR") colorCode = ErrorColor;

        SetConsoleColor(colorCode);
        std::ostream& out = (level == "ERROR") ? std::cerr : std::cout;
        out << "[" << CurrentTime() << "] [" << level << "] ";
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // reset
        out << "(" << file << ":" << line << " " << function << ") " << message << "\n";
#else
        std::ostream& out = (level == "ERROR") ? std::cerr : std::cout;
        out << color << "[" << CurrentTime() << "] [" << level << "] " << RESET
            << "(" << file << ":" << line << " " << function << ") " << message << "\n";
#endif
    }

} // namespace Logger

#ifdef _WIN32
#define Log(fmt, ...)      Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "INFO",  "", fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...)  Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "WARN",  "", fmt, ##__VA_ARGS__)
#define LogError(fmt, ...) Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "ERROR", "", fmt, ##__VA_ARGS__)
#else
#define Log(fmt, ...)      Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "INFO",  Logger::CYAN,   fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...)  Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "WARN",  Logger::YELLOW, fmt, ##__VA_ARGS__)
#define LogError(fmt, ...) Logger::LogInternal(Logger::BaseFileName(__FILE__), __func__, __LINE__, "ERROR", Logger::RED,   fmt, ##__VA_ARGS__)
#endif
