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

    template<typename... Args>
    void Log(const std::string& format_str, Args&&... args) {
        std::string message = std::vformat(format_str, std::make_format_args(args...));

#ifdef _WIN32
        SetConsoleColor(InfoColor);
        std::cout << "[" << CurrentTime() << "] [INFO]  ";
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // reset
        std::cout << message << "\n";
#else
        std::cout << CYAN << "[" << CurrentTime() << "] [INFO]  " << RESET << message << "\n";
#endif
    }

    template<typename... Args>
    void LogWarn(const std::string& format_str, Args&&... args) {
        std::string message = std::vformat(format_str, std::make_format_args(args...));

#ifdef _WIN32
        SetConsoleColor(WarnColor);
        std::cout << "[" << CurrentTime() << "] [WARN] ";
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // reset
        std::cout << message << "\n";
#else
        std::cout << YELLOW << "[" << CurrentTime() << "] [WARN] " << RESET << message << "\n";
#endif
    }

    template<typename... Args>
    void LogError(const std::string& format_str, Args&&... args) {
        std::string message = std::vformat(format_str, std::make_format_args(args...));

#ifdef _WIN32
        SetConsoleColor(ErrorColor);
        std::cerr << "[" << CurrentTime() << "] [ERROR] ";
        SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // reset
        std::cerr << message << "\n";
#else
        std::cerr << RED << "[" << CurrentTime() << "] [ERROR] " << RESET << message << "\n";
#endif
    }

} // namespace Logger

using Logger::Log;
using Logger::LogWarn;
using Logger::LogError;