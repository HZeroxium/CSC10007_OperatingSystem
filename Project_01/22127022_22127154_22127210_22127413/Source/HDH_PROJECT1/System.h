#ifndef SYSTEM_H
#define SYSTEM_H
#pragma once
#include"Volume.h"
#include <atomic>
#include <vector>
#include <string>
class System {
private: // Core
	std::atomic<bool> bAppRunning;
private: // Data
	Volume Vol;
	std::string sDrivePath;
	std::string sDirPath;
private: // Navigator
	enum Command
	{
		NONE,
		HELP,
		CODING,
		RESTART,
		DIR,
		OPEN,
		CD,
	};
	Command command;
public: // Constructors & Destructors
	System();
	~System();
public: // Controls
	bool OnCreate();
	bool OnDestroy();
	bool OnStart();
	bool OnFinish();
	bool OnRunning();
	bool OnLifecycle();
public: // Events
	bool OnCreateEvent();
	bool OnDestroyEvent();
	bool OnUpdateEvent();
	bool OnRenderEvent();
public: // Commands
	bool OnHelpCommand();
	bool OnCodingCommand();
	bool OnRestartCommand();
	bool OnDirCommand();
	bool OnOpenCommand();
	bool OnCdCommand();
};

#endif // SYSTEM_H