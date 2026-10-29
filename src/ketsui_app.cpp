// ketsui - ReXGlue Recompiled Project

#include "ketsui_app.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_dialog.h>
#include <toml++/toml.hpp>

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/runtime.h>

namespace {

// $XDG_CONFIG_HOME/ketsui/ketsui.toml, or ~/.config/ketsui/ketsui.toml.
std::filesystem::path UserConfigPath() {
  std::filesystem::path base;
  if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg) {
    base = xdg;
  } else if (const char* home = std::getenv("HOME"); home && *home) {
    base = std::filesystem::path(home) / ".config";
  } else {
    base = rex::filesystem::GetUserFolder();
  }
  return base / "ketsui" / "ketsui.toml";
}

bool IsGameFolder(const std::filesystem::path& folder) {
  std::error_code ec;
  return !folder.empty() && std::filesystem::is_regular_file(folder / "default.xex", ec);
}

// Writes only game_data_root. rex::cvar::SaveConfig would also persist options
// given on the command line (log_file, gpu_plugin...). Other keys are kept.
void SaveGameFolder(const std::filesystem::path& config_path, const std::filesystem::path& folder) {
  std::error_code ec;
  std::filesystem::create_directories(config_path.parent_path(), ec);

  toml::table table;
  if (std::filesystem::exists(config_path)) {
    try {
      table = toml::parse_file(config_path.string());
    } catch (const toml::parse_error&) {
      table = toml::table{};
    }
  }
  table.insert_or_assign("game_data_root", folder.string());

  std::ofstream out(config_path);
  out << "# Auto-generated cvar configuration\n" << table;
}

void ShowInfo(const char* message, SDL_MessageBoxFlags flags) {
  static const SDL_MessageBoxButtonData kOk[] = {
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK"},
  };
  const SDL_MessageBoxData data = {flags, nullptr, "Ketsui", message, 1, kOk, nullptr};
  int button = 0;
  SDL_ShowMessageBox(&data, &button);
}

}  // namespace

void KetsuiApp::OnConfigurePaths(rex::PathConfig& paths) {
  paths.config_path = UserConfigPath();
}

std::optional<rex::PathConfig> KetsuiApp::OnFinalizePaths(
    const rex::PathConfig& defaults, std::function<void(rex::PathConfig)> resume) {
  // The saved config is loaded before this hook, so the cvar already holds the
  // folder from an earlier run (or from --game_data_root).
  std::filesystem::path game_folder = REXCVAR_GET(game_data_root);
  if (game_folder.empty()) {
    game_folder = defaults.game_data_root;
  }
  if (IsGameFolder(game_folder)) {
    rex::PathConfig paths = defaults;
    paths.game_data_root = game_folder;
    return paths;
  }

  defaults_ = defaults;
  resume_ = std::move(resume);
  AskForGameFolder();
  return std::nullopt;
}

void KetsuiApp::AskForGameFolder() {
  static const SDL_MessageBoxButtonData kButtons[] = {
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Choose folder"},
      {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Quit"},
  };
  const SDL_MessageBoxData data = {
      SDL_MESSAGEBOX_INFORMATION,
      nullptr,
      "Ketsui",
      "Ketsui is not included with this program.\n\n"
      "Choose the folder that contains your copy of the game "
      "(the folder with default.xex, Data/ and nxeart).",
      2,
      kButtons,
      nullptr,
  };
  int button = 0;
  if (!SDL_ShowMessageBox(&data, &button) || button == 0) {
    app_context().RequestDeferredQuit();
    return;
  }
  SDL_ShowOpenFolderDialog(&KetsuiApp::OnFolderDialogDone, this, nullptr, nullptr, false);
}

void KetsuiApp::OnFolderDialogDone(void* userdata, const char* const* filelist,
                                    int /*filter*/) {
  auto* app = static_cast<KetsuiApp*>(userdata);
  std::string picked;
  if (filelist && filelist[0]) {
    picked = filelist[0];
  }
  // The dialog may call back from another thread: resume on the UI thread.
  app->app_context().CallInUIThread([app, picked]() {
    if (picked.empty()) {
      app->AskForGameFolder();
    } else {
      app->OnGameFolderPicked(picked);
    }
  });
}

void KetsuiApp::OnGameFolderPicked(const std::string& picked) {
  std::filesystem::path folder(picked);
  if (!IsGameFolder(folder)) {
    ShowInfo("There is no default.xex in that folder.", SDL_MESSAGEBOX_WARNING);
    AskForGameFolder();
    return;
  }

  REXCVAR_SET(game_data_root, folder.string());
  SaveGameFolder(UserConfigPath(), folder);

  rex::PathConfig paths = defaults_;
  paths.game_data_root = folder;
  resume_(std::move(paths));
}
