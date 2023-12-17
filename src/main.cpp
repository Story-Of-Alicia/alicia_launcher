#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <windows.h>

namespace
{

  HANDLE webInfoMappingHandle = nullptr;

  //! Hosts shared memory file containing web info for Alicia.
  //! @param webInfoHandle Shared memory file name.
  //! @param webInfo Contents of the shared memory file.
  //! @throws std::runtime_error on runtime error.
  void create_webinfo(
    const std::string_view& webInfoHandle,
    const std::string_view webInfo)
  {
    // create the file
    DWORD error = 0;
    const auto file = CreateFile(
      webInfoHandle.data(),
      GENERIC_READ | GENERIC_WRITE,
      0, nullptr,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
      nullptr);
    if (file == INVALID_HANDLE_VALUE)
    {
      error = GetLastError();
      if (error) {
        printf("Couldn't create file: (%ld)\n", error);
        throw std::runtime_error("Couldn't create file.");
      }
    }

    // write the file
    WriteFile(file, webInfo.data(), webInfo.size(), nullptr, nullptr);
    FlushFileBuffers(file);

    // map the file
    webInfoMappingHandle = CreateFileMapping(
      file, nullptr, PAGE_READWRITE, 0, 0, webInfoHandle.data());
    error = GetLastError();
    if (error) {
      printf("Couldn't create file mapping: (%ld)\n", error);
      throw std::runtime_error("Couldn't create file mapping.");
    }
  }

  //! Releases shared memory file.
  //! @param webInfoHandle Shared memory file name.
  void destroy_webinfo(const std::string_view& webInfoHandle)
  {
    if (webInfoMappingHandle)
      UnmapViewOfFile(webInfoMappingHandle);
  }

  //! Settings.
  struct Settings
  {
    std::string webInfoHandle;
    std::string webInfoContent;
    std::string executableProgram;
    std::string executableArguments;
    bool noLaunch;
    bool tui;
  };

  //! Loads settings from file.
  //! @param path Path to settings file.
  //! @param settings Reference to settings.
  void load_settings(const std::string_view& path, Settings& settings) {
    std::ifstream settingsFile(path.data());
    if (!settingsFile.is_open())
      throw std::runtime_error("The settings file does not exist.");

    try
    {
      auto json = nlohmann::json::parse(settingsFile);
      settings.webInfoHandle = json["webInfoHandle"];
      settings.webInfoContent = json["webInfoContent"];
      settings.executableProgram = json["executableProgram"];
      settings.executableArguments = json["executableArguments"];
      settings.noLaunch = json["noLaunch"];
      settings.tui = json["tui"];
    } catch (const nlohmann::json::exception& x)
    {
      throw x;
    }
  }

} // anon namespace

int main(int argc, char** argv) {

  Settings settings;
  try
  {
    load_settings("settings.json", settings);
    spdlog::info("Loaded settings");
  } catch (std::exception& x)
  {
    spdlog::error("Failed to load settings: {}", x.what());
    MessageBox(nullptr, "Failed to load settings.", "Launcher", MB_OK);
    return 1;
  }

  try
  {
    create_webinfo(settings.webInfoHandle, settings.webInfoContent);
    spdlog::info("Created web info");
  } catch (const std::exception& e)
  {
    spdlog::error("Failed to create web info: {}", e.what());
    MessageBox(nullptr, "Couldn't create the web info.", "Launcher", MB_OK);
    return 0;
  }

  if (settings.noLaunch)
  {
    spdlog::info("Not launching the game. Idling until input from console.");
    int a;
    std::cin >> a;
  }

  STARTUPINFO startupInfo{
    .cb = sizeof(STARTUPINFO)
  };
  PROCESS_INFORMATION processInfo{
  };

  spdlog::info("Launching the game");
  if (!CreateProcess(
        settings.executableProgram.data(),
        settings.executableArguments.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo))
  {
    if (GetLastError() == ERROR_ELEVATION_REQUIRED)
    {
      spdlog::error("Can't launch the game, elevation is required");
      MessageBox(nullptr, "Couldn't launch the game, run the launcher as an administrator.", "Launcher", MB_OK);
    } else {
      spdlog::error("Can't launch the game, misc error");
      MessageBox(nullptr, "Couldn't launch the game, is the executable in the working directory of this app?", "Launcher", MB_OK);
    }
  }
  else
  {
    spdlog::info("Game launched, idling until the process exits");
    WaitForSingleObject( processInfo.hProcess, INFINITE );

    DWORD exitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    spdlog::info("Game exited with code {}", exitCode);
  }

  destroy_webinfo(settings.webInfoHandle);
  spdlog::info("Released web info");

  return 0;
}
