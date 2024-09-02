#pragma once
#include "Console.h"
#include <locale>
#include <codecvt>

// remove rarely used windows headers
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/// C-lib
#include <stdio.h>
#include <conio.h>
/// STL
#include <cassert>
#include <sstream>
#include <algorithm>
#include <iostream>

/// =========
/// UTILITIES
/// =========
namespace console
{
    void SetTextColor(console::Color textColor)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, textColor);
    }

    void SetBackgroundColor(console::Color bgColor)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
        WORD attributes = (consoleInfo.wAttributes & 0xFF0F) | (bgColor << 4);
        SetConsoleTextAttribute(hConsole, attributes);
    }
    void ResetTextColor()
    {
        SetTextColor(console::WHITE);
    }
    void ResetBackgroundColor()
    {
        SetBackgroundColor(console::BLACK);
    }

    void ResetConsoleColors()
    {
        ResetTextColor();
        ResetBackgroundColor();
    }

    void Display(const console::Message& msg, const std::string& end)
    {
        if (msg.empty()) {
            return;
        }
        SetBackgroundColor(msg.bgColor);
        SetTextColor(msg.textColor);
        std::cout << msg.data << end;
        std::cout.flush();
        ResetConsoleColors();
    }

    void DisplayTitle(const console::Message& msg)
    {
        if (msg.empty()) {
            return;
        }
        SetBackgroundColor(msg.bgColor);
        SetTextColor(msg.textColor);
        std::cout << std::string(msg.size(), '=') << std::endl;
        std::cout << msg.data << std::endl;
        std::cout << std::string(msg.size(), '=') << std::endl;
        ResetConsoleColors();
    }

    void DisplayOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> options
    ) {
        Display(sFrontMsg, "\n");
        int index = nStartIndex;
        for (const auto& option_list : options) {
            for (const auto& option : option_list) {
                Display({ STREAMTEXT("  [" << index++ << "]: "), console::CYAN });
                Display({ option, console::GREEN }, "\n");
            }
        }
        Display(sBackMsg, "\n");
    }
    void DisplayOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> param_options,
        std::initializer_list<std::vector<std::string>> value_options
    ) {
        Display(sFrontMsg, "\n");
        std::vector<std::string> all_param_options;
        for (const auto& vec : param_options) {
            all_param_options.insert(all_param_options.end(), vec.begin(), vec.end());
        }
        std::vector<std::string> all_value_options;
        for (const auto& vec : value_options) {
            all_value_options.insert(all_value_options.end(), vec.begin(), vec.end());
        }
        size_t max_left_size = 0;
        for (const std::string& s : all_param_options) {
            max_left_size = max(max_left_size, s.size());
        }
        for (int i = 0, index = nStartIndex; i < all_param_options.size(); ++i, ++index) {
            const auto& param = all_param_options[i];
            const auto& value = all_value_options[i];
            const std::string sPadding = std::string(4 + max_left_size - param.size(), ' ');
            Display({ STREAMTEXT("  [" << index << "]: "), console::CYAN });
            Display({ param, console::BLUE }, sPadding);
            Display({ value, console::GREEN }, "\n");
        }
        Display(sBackMsg, "\n");
    }


    void ClearScreen()
    {
#if defined _WIN32
        system("cls");
#elif
        system("clear");
#endif
    }

    void Pause(const console::Message& msg, bool bFlush)
    {
        ReadChar(msg);
        if (bFlush) {
            std::cout.flush();
        }
    }
}

/// =======
/// READERS
/// =======
namespace console
{
    char ReadToken()
    {
        while (!_kbhit());
        char input = _getch();
        if (input == 27) {
            return ' ';
        }
        return input;
    }

    char ReadChar(const console::Message& msg)
    {
        Display(msg);
        char input = ReadToken();
        return input;
    }

    std::string ReadString(const console::Message& msg, const std::string& terminators)
    {
        Display(msg);
        std::string input;
        for (char token; (token = ReadToken()) != EOF; ) {
            bool isTerminator = false;
            for (char terminator : terminators) {
                if (token == terminator) {
                    isTerminator = true;
                    break;
                }
            }
            if (isTerminator) {
                break;
            }
            if (token == '\b') {
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";
                }
            }
            else if (token >= 32 && token <= 126) {
                std::cout << token;
                input += token;
            }
        }
        return input;
    }

    std::string ReadLine(const console::Message& msg)
    {
        return ReadString(msg, "\n\r");
    }

    int ReadInt(const console::Message& msg)
    {
        try {
            int value = std::stoi(ReadString(msg));
        }
        catch (const std::invalid_argument& e) {
            Display({ "Invalid input, expected a valid integer", console::RED }, "\n");
            return -1;
        }
    }

    int ReadInt(int lower, int upper, const console::Message& msg)
    {
        assert((lower <= upper) && "Lower bound is greater than upper bound.");
        while (true) {
            try {
                int value = std::stoi(ReadString(msg));
                if (lower <= value && value <= upper) {
                    return value;
                }
                else {
                    std::cout << std::endl;
                    Display({
                        STREAMTEXT("Integer must in [" << lower << ".." << upper << "]"),
                        console::RED
                    }, "\n");
                }
            }
            catch (const std::invalid_argument& e) {
                Display({ "Invalid input, expected a valid integer", console::RED }, "\n");
            }
        }
    }

    int ReadOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> options
    ) {
        int nOptionCount = 0;
        for (const std::vector<std::string>& element : options) {
            nOptionCount += static_cast<int>(element.size());
        }
        DisplayOptions(sFrontMsg, sBackMsg, nStartIndex, options);
        return ReadInt(nStartIndex, nStartIndex + nOptionCount - 1);
    }
    int ReadOptions(
        const console::Message& sFrontMsg,
        const console::Message& sBackMsg,
        int32_t nStartIndex,
        std::initializer_list<std::vector<std::string>> param_options,
        std::initializer_list<std::vector<std::string>> value_options
    ) {
        int nOptionParamCount = 0;
        for (const std::vector<std::string>& element : param_options) {
            nOptionParamCount += static_cast<int>(element.size());
        }
        int nOptionValueCount = 0;
        for (const std::vector<std::string>& element : value_options) {
            nOptionValueCount += static_cast<int>(element.size());
        }
        assert(nOptionParamCount == nOptionValueCount && "Mismatch size of param and value for read options");
        DisplayOptions(sFrontMsg, sBackMsg, nStartIndex, param_options, value_options);
        return ReadInt(nStartIndex, nStartIndex + nOptionParamCount - 1);
    }
}

namespace console
{
    std::string wchar_to_str(const wchar_t* wstr) {
        size_t wstr_len = wcslen(wstr);
        char* mbstr = new char[wstr_len + 1];
        size_t converted_size;
        wcstombs_s(&converted_size, mbstr, wstr_len + 1, wstr, _TRUNCATE);
        std::string result(mbstr);
        delete[] mbstr;
        return result;
    }

    std::wstring str_to_wstr(const std::string& utf8String)
    {
        const int wideStringSize =
            MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, nullptr, 0);
        if (wideStringSize == 0) {
            return L"";
        }
        const std::unique_ptr<wchar_t[]> wideBuffer(new wchar_t[wideStringSize]);
        if (MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), -1, wideBuffer.get(), wideStringSize) == 0) {
            return L"";
        }
        return std::wstring(wideBuffer.get());
    }

    std::string wstr_to_str(const std::wstring& wideString)
    {
        const int utf8StringSize = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8StringSize == 0) {
            return "";
        }
        const std::unique_ptr<char[]> utf8Buffer(new char[utf8StringSize]);
        if (WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, utf8Buffer.get(), utf8StringSize, nullptr, nullptr) == 0) {
            return "";
        }
        return std::string(utf8Buffer.get());
    }

    std::string simplify_str(std::string input)
    {
        std::replace(input.begin(), input.end(), '\t', ' ');
        input.erase(input.begin(), std::find_if(input.begin(), input.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
        input.erase(std::find_if(input.rbegin(), input.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), input.end());
        return input;
    }

    std::vector<std::string> split_str(const std::string& str, const std::string& sDelimiter)
    {
        std::vector<std::string> res;
        std::string current;
        for (char c : str) {
            if (sDelimiter.find(c) != std::string::npos) {
                if (current.size()) {
                    res.push_back(current);
                    current = "";
                }
            }
            else {
                current += c;
            }
        }
        if (current.size()) {
            res.push_back(current);
        }
        return res;
    }

    bool case_sensitive_eq(const std::string& str1, const std::string& str2)
    {
        if (str1.length() != str2.length()) {
            return false;
        }
        std::string str1Upper = str1;
        std::string str2Upper = str2;
        std::transform(str1Upper.begin(), str1Upper.end(), str1Upper.begin(), ::toupper);
        std::transform(str2Upper.begin(), str2Upper.end(), str2Upper.begin(), ::toupper);
        return str1Upper == str2Upper;
    }

    /// only take alpha numeric characters (lower latin, upper latin, numbers)
    std::string get_alnum(const std::string& str)
    {
        std::string res;
        std::copy_if(str.begin(), str.end(), std::back_inserter(res), [](unsigned char c) { 
            return std::isalnum(c); 
        });
        return res;
    }

    std::string get_file_extension(const std::string& sFileName)
    {
        size_t lastDotIndex = sFileName.find_last_of(".");
        if (lastDotIndex != std::string::npos) {
            return sFileName.substr(lastDotIndex + 1);
        }
        return "";
    }

    std::string get_drive_name(const std::string& sDrivePath)
    {
        size_t pos = sDrivePath.find(':');
        if (pos != std::string::npos && pos > 0) {
            return get_alnum(sDrivePath.substr(0, pos));
        }
        return "";
    }
}

namespace console
{
    std::vector<std::string> GetAllDrives()
    {
        std::vector<std::string> drives;
        DWORD dwSize = MAX_PATH;
        wchar_t szLogicalDrives[MAX_PATH] = { 0 };
        DWORD dwResult = GetLogicalDriveStringsW(dwSize, szLogicalDrives);
        if (dwResult > 0 && dwResult <= MAX_PATH) {
            wchar_t* szSingleDrive = szLogicalDrives;
            while (*szSingleDrive) {
                drives.push_back(wchar_to_str(szSingleDrive));
                szSingleDrive += wcslen(szSingleDrive) + 1;
            }
        }
        return drives;
    }
    
    std::string GetLastErrorAsString() 
    {
        DWORD errorMessageID = GetLastError();
        if (errorMessageID == 0) {
            return std::string(); // No error message has been recorded
        }
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorMessageID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&messageBuffer),
            0,
            NULL
        );
        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }
}