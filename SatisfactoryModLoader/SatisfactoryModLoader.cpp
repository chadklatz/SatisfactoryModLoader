// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN
#include <stdafx.h>
#include <SatisfactoryModLoader.h>
#include <string>
#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <functional>
#include <chrono>
#include <thread>
#include <util/Utility.h>
#include <util/Reflection.h>
#include <mod/ModHandler.h>
#include <util/Configuration.h>

static ModHandler modHandler;

// Main DLL for loading mod DLLs
void mod_loader_entry() {
	// launch the game's internal console and hook into it
	logFile = std::ofstream("SatisfactoryModLoader.log", std::ios_base::trunc);
	logFile.close();
	logFile = std::ofstream("SatisfactoryModLoader.log", std::ios_base::app);
	AllocConsole();
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	FILE* fp;
	freopen_s(&fp, "CONOIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	logFile.clear();

	info("Attached SatisfactoryModLoader to Satisfactory");

	// load up all of the configuration information
	read_config();

	check_version(targetVersion);

	if (loadConsole) {
		ShowWindow(GetConsoleWindow(), SW_SHOW);
	}
	
	// load mods
	// get path
	char p[MAX_PATH];
	GetModuleFileNameA(NULL, p, MAX_PATH);

	// load mods
	modHandler.load_mods(p);
	modHandler.setup_mods();
	modHandler.check_dependencies();
	modHandler.post_setup_mods();

	// log mod size
	size_t listSize = modHandler.mods.size();
	info("Loaded ", listSize, " mod", (listSize > 1 || listSize == 0 ? "s" : ""));

	//display condensed form of mod information
	std::string modList = "[";
	for (auto&& mod : modHandler.mods) {
		modList.append(mod->info.name + "@" + mod->info.version + ", ");
	}

	if (listSize > 0) {
		info("Loaded mods: ", modList.substr(0, modList.length() - 2), "]");
	}

	info("SatisfactoryModLoader Initialization complete. Launching Satisfactory...");
}

void read_config() {
	info("Finding config file...");
	Configuration loaderConfig("..\\config");
	if (!loaderConfig.exists()) {
		error("No config file found!");
		info("Creating new config file...");
		loaderConfig.set("Console", true);
		loaderConfig.set("Debug", false);
		loaderConfig.set("SupressErrors", false);
		loaderConfig.save();
	}

	loaderConfig.load();

	loadConsole = loaderConfig.get<bool>("Console");
	if (loadConsole == NULL) {
		loadConsole = true;
	}
	debugOutput = loaderConfig.get<bool>("Debug");
	if (debugOutput == NULL) {
		debugOutput = false;
	}
	supressErrors = loaderConfig.get<bool>("SupressErrors");
	if (supressErrors == NULL) {
		supressErrors = false;
	}
}

//cleans up when the program is killed
void cleanup() {
	modHandler.mods.clear();

	info("SML shutting down...");
	logFile.close();
}