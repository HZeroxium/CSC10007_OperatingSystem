#pragma once
#include "System.h"
#include "Console.h"

/// ==========================
/// CONSTRUCTORS & DESTRUCTORS
/// ==========================
System::System() : command(NONE)
{
}

System::~System()
{
}

/// ========
/// CONTROLS
/// ========
bool System::OnCreate()
{
    return true;
}

bool System::OnDestroy()
{
    return true;
}

bool System::OnStart()
{
    if (!OnRunning()) {
        return false;
    }
    return true;
}

bool System::OnFinish()
{
    return true;
}

bool System::OnRunning()
{
    while (bAppRunning = OnLifecycle());
    return true;
}

bool System::OnLifecycle()
{
    bAppRunning = OnCreateEvent();
    while (bAppRunning)
    {
        if (!OnUpdateEvent()) {
            bAppRunning = false;
            break;
        }
        if (!OnRenderEvent()) {
            bAppRunning = false;
            break;
        }
    }
    return !OnDestroyEvent();
}

/// ======
/// EVENTS
/// ======
bool System::OnCreateEvent()
{
    std::vector<std::string> drives;
    int index = 0;
    while (index == 0) {
        console::ClearScreen();
        const std::string sTitle = (command == RESTART ? "Restart" : "Create");
        console::DisplayTitle({ STREAMTEXT("[System] " << sTitle), console::YELLOW});
        drives = console::GetAllDrives();
        index = console::ReadOptions(
            console::Message("What disk do you want to choose ?"),
            console::Message(""),
            0,
            { {"Refresh disk drive"}, drives }
        );
    }
    sDrivePath = "\\\\.\\" + drives[index - 1];
    sDrivePath.pop_back(); // remove the back of C\:
    sDirPath = sDrivePath;
    console::ClearScreen();
    console::DisplayTitle({ STREAMTEXT("[System] Reading Disk " << drives[index - 1]), console::YELLOW });
    if (Vol.Connect(sDrivePath)) {
        console::Display({ STREAMTEXT("The system is successfully created"), console::GREEN }, "\n");
        console::Pause();
        command = HELP;
        return true;
    }
    console::Display({ STREAMTEXT("Failed to read the disk"), console::RED });
    return false;
}

bool System::OnDestroyEvent()
{
    return true;
}

bool System::OnUpdateEvent()
{
    switch (command)
    {
    case Command::HELP:     return OnHelpCommand();
    case Command::CODING:   return OnCodingCommand();
    case Command::RESTART:  return OnRestartCommand();
    case Command::DIR:      return OnDirCommand();
    case Command::OPEN:     return OnOpenCommand();
    case Command::CD:       return OnCdCommand();
    }
    console::Display({ "Unknown Command", console::RED });
    return false;
}

bool System::OnRenderEvent()
{
    return true;
}
/// ========
/// COMMANDS
/// ========
bool System::OnHelpCommand()
{
    console::ClearScreen();
    console::DisplayTitle({ STREAMTEXT("[System] Help"), console::YELLOW });
    Vol.Dir();
    std::vector<std::string> command_param = {
        "</>",
        "-restart",
        "-dir",
        "-open filename.txt",
        "-cd ./path/to/folder"
    };
    std::vector<std::string> command_description = {
        "typing commands by your own",
        "restart the app to choose another drive",
        "to show all files and folders",
        "read the content inside the txt file named \"filename\"",
        "change the current directory into the file named \"folder\" of given path"
    };
    int index = console::ReadOptions(
        console::Message("What do you want to do ?", console::MAGENTA),
        console::Message(""),
        0,
        { command_param },
        { command_description }
    );
    console::ClearScreen();
    switch (index)
    {
    case 0: command = Command::CODING;  return true;
    case 1: command = Command::RESTART; return true;
    case 2: command = Command::DIR;     return true;
    case 3: command = Command::OPEN;    return true;
    case 4: command = Command::CD;      return true;
    }    
    return false;
}

bool System::OnCodingCommand()
{
    std::string input = console::simplify_str(console::ReadLine({ Vol.GetURL() + ">> ", console::YELLOW}));
    input.erase(0, input.find_first_not_of('-'));
    std::cout << std::endl;
    // open filename.txt
    if (input.substr(0, 4) == "open") {
        std::string filenameWithExtension = input.substr(5); // Extract the part after "-open "
        std::size_t lastDot = filenameWithExtension.find_last_of('.');
        if (lastDot != std::string::npos) {
            std::string filename = filenameWithExtension.substr(0, lastDot);
            std::string extension = filenameWithExtension.substr(lastDot + 1);
            console::Display({
                STREAMTEXT("file_name: " << filename << " | file_extension: " << extension),
                console::MAGENTA
            }, "\n");
            Vol.Open(filename + "." + extension);
        }
        else {
            console::Display({ "Error: No extension found.", console::RED }, "\n");
        }
        console::Pause();
        return true;
    }
    // cd path/to/folder
    if (input.substr(0, 2) == "cd") {
        if (input.size() <= 2) {
            console::Display({ "Error: cd command lack of \"path/to/folder\" argument", console::RED }, "\n");
            console::Display({ "Expected: cd path/to/folder", console::BLUE }, "\n");
            return true;
        }
        std::string sPathToFile = input.substr(3);
        console::Display({ STREAMTEXT("Directory path: \"" << sPathToFile << "\""), console::MAGENTA}, "\n");
        if (Vol.Cd(sPathToFile)) {
            console::Display({ STREAMTEXT("Successfully changed directory path"), console::GREEN}, "\n");
        }
        else {
            console::Display({ STREAMTEXT("Failed to changed directory path"), console::RED}, "\n");
        }
        return true;
    }
    // restart
    if (input == "restart") {
        return OnRestartCommand();
    }
    // dir
    if (input == "dir") {
        Vol.Dir();
        return true;
    }
    // easter egg
    if (input == "</>") {
        std::cout << "Typing commands by your own" << std::endl;
        return true;
    }

    console::Display({ "Error: Command not recognized", console::RED }, "\n");
    return true;
}
bool System::OnRestartCommand()
{
    if (!OnCreateEvent()) {
        command = HELP;
        console::DisplayTitle({ STREAMTEXT("[System] Restart"), console::YELLOW });
        console::Display({ "Failed to restart", console::RED });
        console::Pause();
    }
    return true;
}
bool System::OnDirCommand()
{
    console::ClearScreen();
    console::DisplayTitle({ STREAMTEXT("[System] Dir"), console::YELLOW });
    Vol.Dir();
    console::Pause();
    command = HELP;
    return true;
}
bool System::OnOpenCommand()
{
    console::ClearScreen();
    console::DisplayTitle({ STREAMTEXT("[System] Open"), console::YELLOW });
    Vol.Dir();
    std::string sFileFullName = console::simplify_str(console::ReadString({ 
        "Input File Full Name: ", console::CYAN 
    }));
    std::cout << std::endl;
    Vol.Open(sFileFullName);
    console::Pause();
    command = HELP;
    return true;
}
bool System::OnCdCommand()
{
    console::ClearScreen();
    console::DisplayTitle({ STREAMTEXT("[System] Cd"), console::YELLOW });
    Vol.Dir();
    std::string sPathToFile = console::simplify_str(console::ReadString({
        "Input Path/To/File: ", console::CYAN
    }));
    std::cout << std::endl;
    Vol.Cd(sPathToFile);
    Vol.Dir();
    console::Pause();
    command = HELP;
    return true;
}
