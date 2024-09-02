#ifndef CONSOLE_H
#define CONSOLE_H
#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <initializer_list>
#define STREAMTEXT(msg) \
    ([&]() -> std::string { \
        std::ostringstream msgStream; \
        msgStream << msg; \
        return msgStream.str(); \
    })()

namespace console
{
    enum Color
    {
        BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE,
    };

    struct Message
    {
        Color textColor, bgColor;
        std::string data;
        Message(std::string data = "", Color text = WHITE, Color background = BLACK)
            : data(data), textColor(text), bgColor(background) {}
        Message(const char* data, Color text = WHITE, Color background = BLACK)
            : data(data), textColor(text), bgColor(background) {}
        inline bool empty() const { return data.empty(); }
        inline size_t size() const { return data.size(); }
    };
}

namespace console
{
    void SetTextColor(console::Color textColor);
    void SetBackgroundColor(console::Color bgColor);
    void ResetTextColor();
    void ResetBackgroundColor();
    void ResetConsoleColors();
    void Display(const console::Message &msg, const std::string& end = "");
    void DisplayTitle(const console::Message &msg);
    void DisplayOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> options
    );
    void DisplayOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> param_options,
        std::initializer_list<std::vector<std::string>> value_options
    );
    void ClearScreen();
    void Pause(
        const console::Message& msg = { "Press any key to continue\n", console::RED},
        bool bFlush = true
    );
};

namespace console
{
    char ReadToken();
    char ReadChar(const console::Message &msg = console::Message());
    std::string ReadString(
        const console::Message &msg = console::Message(),
        const std::string& terminators = " \n\t\r"
    );
    std::string ReadLine(const console::Message& msg = console::Message());
    int ReadInt(const console::Message& msg = console::Message());
    int ReadInt(int lower, int upper, const console::Message& msg = console::Message());
    int ReadOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> options
    );
    int ReadOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> param_options,
        std::initializer_list<std::vector<std::string>> value_options
    );
}

namespace console // string utils
{
    std::string wchar_to_str(const wchar_t* wstr);
    std::wstring str_to_wstr(const std::string& str);
    std::string wstr_to_str(const std::wstring& wstr);
    std::string simplify_str(std::string input);
    std::vector<std::string> split_str(const std::string& str, const std::string& sDelimiters = " \t");
    bool case_sensitive_eq(const std::string& str1, const std::string& str2);
    std::string get_alnum(const std::string& str);
    std::string get_file_extension(const std::string& sFileName);
    std::string get_drive_name(const std::string& sDrivePath);
}

namespace console // window stuff
{
    std::vector<std::string> GetAllDrives();
    std::string GetLastErrorAsString();
}
#endif // CONSOLE_H
