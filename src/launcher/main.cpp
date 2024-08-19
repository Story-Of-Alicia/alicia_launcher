#include "alicia.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <windows.h>
#include <winerror.h>

namespace {

  //! Settings.
  struct Settings {
    std::string _webInfoId;
    alicia::WebInfo _webInfoContent;
    std::string _executableProgram;
    std::string _executableArguments;
    bool _launch = true;
  };

  //! Loads settings from file.
  //! @param path Path to settings file.
  //! @param settings Reference to settings.
  void load_settings(const std::string_view& path, Settings& settings)
  {
    std::ifstream settingsFile(path.data());
    if(!settingsFile.is_open()) {
      throw std::runtime_error("The settings file does not exist.");
    }

    try
    {
      const auto json = nlohmann::json::parse(settingsFile);
      settings._webInfoId = json["webInfoId"];

      const nlohmann::json& webInfoContentJson = json["webInfoContent"];
      settings._webInfoContent = {
          .gameId = webInfoContentJson["GameId"],
          .memberNo = webInfoContentJson["MemberNo"],
          .loginId = webInfoContentJson["LoginId"],
          .authKey = webInfoContentJson["AuthKey"],
          .installUrl = webInfoContentJson["InstallUrl"],
          .serverType = webInfoContentJson["ServerType"],
          .serverInfo = webInfoContentJson["ServerInfo"],
          .sex = static_cast<alicia::WebInfo::Sex>(webInfoContentJson["Sex"]),
          .birthday = webInfoContentJson["Birthday"],
          .wardNo = webInfoContentJson["WardNo"],
          .cityCode = webInfoContentJson["CityCode"],
          .zipCode = webInfoContentJson["ZipCode"],
          .pcBangNo = webInfoContentJson["PcBangNo"],
          .closeTime = webInfoContentJson["CloseTime"],
      };

      settings._executableProgram = json["executableProgram"];
      settings._executableArguments = json["executableArguments"];
      settings._launch = json["launch"];
    }
    catch(const nlohmann::json::exception& x)
    {
      throw x;
    }
  }

  //! Registers the alicia launch protocol in the registry.
  //! @param name name of the protocol
  //! @param path absolute path to the protocol command executable
  void register_protocol(const std::string& name, const std::string& path)
  {
    HKEY protocol, command;
    int result;

    result = RegCreateKeyA(HKEY_CLASSES_ROOT, (LPCSTR)name.c_str(), &protocol);
    if(result != ERROR_SUCCESS)
      throw std::runtime_error("failed to create registry key HKEY_CLASSES_ROOT\\a2launch");

    result = RegSetValueEx(protocol, nullptr, 0, REG_SZ, (LPBYTE) "URL:a2launch Protocol", 22);
    if(result != ERROR_SUCCESS)
      throw std::runtime_error("failed to create registry value in 'HKEY_CLASSES_ROOT\\a2launch'");

    result = RegSetValueEx(protocol, "URL Protocol", 0, REG_SZ, nullptr, 0);
    if(result != ERROR_SUCCESS)
      throw std::runtime_error("failed to create registry value in 'HKEY_CLASSES_ROOT\\a2launch'");

    result = RegCreateKeyA(protocol, (LPCSTR) "shell\\open\\command", &command);
    if(result != ERROR_SUCCESS)
      throw std::runtime_error(
          "failed to create registry key 'HKEY_CLASSES_ROOT\\shell\\open\\command'");

    auto path_data = path.c_str();
    result = RegSetValueEx(command, nullptr, 0, REG_SZ, (LPBYTE)path_data, strlen(path_data));
    if(result != ERROR_SUCCESS)
      throw std::runtime_error("failed to create registry value for command in "
                               "'HKEY_CLASSES_ROOT\\a2launch\\shell\\open\\command'");
  }

} // namespace

int main(int argc, char** argv)
{
  Settings settings;
  try
  {
    load_settings("settings.json", settings);
    spdlog::info("Loaded the settings.");
  }
  catch(std::exception& x)
  {
    spdlog::error("Failed to load the settings: {}.", x.what());
    MessageBox(nullptr, "Failed to load settings.", "Launcher", MB_OK);
    return 1;
  }

  alicia::WebInfoHost webInfoHost;
  try
  {
    webInfoHost.host(settings._webInfoId, settings._webInfoContent);
    spdlog::info("Hosted the web info.");
  }
  catch(const std::exception& e)
  {
    spdlog::error("Failed to host web info: {}", e.what());
    MessageBox(nullptr, "Couldn't host the web info.", "Launcher", MB_OK);
    return 0;
  }

  // If launch is not set to true,
  // do not spawn the game.
  if(!settings._launch)
  {
    spdlog::info("Not launching the game. Idling until input from console.");
    int a;
    std::cin >> a;
  }

  STARTUPINFO startupInfo{.cb = sizeof(STARTUPINFO)};
  PROCESS_INFORMATION processInfo{};

  spdlog::info("Launching the game...");
  if(!CreateProcess(
         settings._executableProgram.data(),
         settings._executableArguments.data(),
         nullptr,
         nullptr,
         FALSE,
         0,
         nullptr,
         nullptr,
         &startupInfo,
         &processInfo)) {
    if(GetLastError() == ERROR_ELEVATION_REQUIRED) {
      spdlog::error("Can't launch the game, elevation is required");
      MessageBox(
          nullptr,
          "Couldn't launch the game, run the launcher as an administrator.",
          "Launcher",
          MB_OK);
    }
    else {
      spdlog::error("Can't launch the game, misc error");
      MessageBox(
          nullptr,
          "Couldn't launch the game, is the executable in the working directory of this app?",
          "Launcher",
          MB_OK);
    }
  }
  else {
    spdlog::info("Game launched, idling until the process exits.");
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    spdlog::info("Game exited with code {}.", exitCode);
  }

  return 0;
}
