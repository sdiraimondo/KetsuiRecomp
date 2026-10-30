// ketsui - ReXGlue Recompiled Project

#include "ketsui_app.h"

constexpr int64_t kInputResumeDelayNs = 200'000'000;     // 200 ms
constexpr int64_t kInputReleaseTimeoutNs = 2'000'000'000;  // 2 s

int64_t NowNs() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// Guest word holding the update handler of the current screen. The main menu
// uses kMainMenuHandler (checked on the title, config and menu screens).
constexpr uint32_t kScreenHandlerAddress = 0x82E30E24;
constexpr uint32_t kMainMenuHandler = 0x8224A340;

// "Exit game?" confirmation. "No" is selected by default. Left/Right change the
// selection, Enter or Space confirms it, Escape cancels.
class ExitConfirmDialog final : public rex::ui::ImGuiDialog {
 public:
  ExitConfirmDialog(rex::ui::ImGuiDrawer* drawer, std::function<void(bool)> on_answer)
      : ImGuiDialog(drawer), on_answer_(std::move(on_answer)) {}

  void Move() { selected_ = 1 - selected_; }
  void Activate() { Answer(selected_ == 0); }
  void Cancel() { Answer(false); }

 protected:
  void OnDraw(ImGuiIO& io) override {
    (void)io;
    static constexpr char kTitle[] = "Exit game?";
    if (!has_opened_) {
      ImGui::OpenPopup(kTitle);
      has_opened_ = true;
    }
    if (!ImGui::BeginPopupModal(kTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      Close();
      return;
    }
    ImGui::SetWindowFontScale(1.6f);
    const char* labels[2] = {"Yes", "No"};
    for (int i = 0; i < 2; ++i) {
      if (i > 0) {
        ImGui::SameLine();
      }
      // The selected button is bright, the other one is dimmed.
      const bool selected = selected_ == i;
      const ImVec4 color = selected ? ImVec4(0.20f, 0.45f, 0.85f, 1.0f)
                                    : ImVec4(0.18f, 0.18f, 0.20f, 1.0f);
      ImGui::PushStyleColor(ImGuiCol_Button, color);
      if (ImGui::Button(labels[i], ImVec2(140, 0))) {
        selected_ = i;
        Activate();
      }
      ImGui::PopStyleColor();
    }
    if (answered_) {
      ImGui::CloseCurrentPopup();
      Close();
    }
    ImGui::EndPopup();
  }

 private:
  void Answer(bool yes) {
    if (answered_) {
      return;
    }
    answered_ = true;
    on_answer_(yes);
  }

  std::function<void(bool)> on_answer_;
  bool has_opened_ = false;
  bool answered_ = false;
  int selected_ = 1;  // 0 = Yes, 1 = No
};

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_dialog.h>
#include <toml++/toml.hpp>

#include <rex/cvar.h>
#include <rex/filesystem.h>
#include <rex/perf/counter.h>
#include <rex/runtime.h>
#include <rex/ui/imgui_dialog.h>
#include <rex/input/input_system.h>
#include <rex/memory/utils.h>
#include <rex/system/kernel_state.h>
#include <rex/ui/keybinds.h>
#include <rex/ui/virtual_key.h>

#include <imgui.h>

REXCVAR_DECLARE(std::string, perf_log_csv);

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

void KetsuiApp::OnKeyDown(rex::ui::KeyEvent& e) {
  if (exit_dialog_) {
    switch (e.virtual_key()) {
      case rex::ui::VirtualKey::kLeft:
      case rex::ui::VirtualKey::kRight:
        exit_dialog_->Move();
        break;
      case rex::ui::VirtualKey::kReturn:
        ArmInputRelease(SDL_EVENT_KEY_UP, SDLK_RETURN);
        exit_dialog_->Activate();
        break;
      case rex::ui::VirtualKey::kSpace:
        ArmInputRelease(SDL_EVENT_KEY_UP, SDLK_SPACE);
        exit_dialog_->Activate();
        break;
      case rex::ui::VirtualKey::kEscape:
        ArmInputRelease(SDL_EVENT_KEY_UP, SDLK_ESCAPE);
        exit_dialog_->Cancel();
        break;
      default:
        break;
    }
    return;
  }
  if (e.virtual_key() == rex::ui::VirtualKey::kEscape) {
    OpenExitDialog();
    return;
  }
  rex::ui::ProcessKeyEvent(e);
}

bool KetsuiApp::IsOnMainMenu() const {
  if (!runtime() || !runtime()->kernel_state()) {
    return false;
  }
  auto* memory = runtime()->kernel_state()->memory();
  const void* handler = memory->TranslateVirtual<const void*>(kScreenHandlerAddress);
  return rex::memory::load_and_swap<uint32_t>(handler) == kMainMenuHandler;
}

void KetsuiApp::ArmInputRelease(SDL_EventType release_type, uint32_t release_code) {
  input_release_type_ = release_type;
  input_release_code_ = release_code;
  input_armed_ns_.store(NowNs());
  input_release_pending_.store(true);
}

bool KetsuiApp::InputAllowed() const {
  // A missing release event must not block the game for good: give up after 2 s.
  const bool waiting = input_release_pending_.load() &&
                       NowNs() - input_armed_ns_.load() < kInputReleaseTimeoutNs;
  return exit_dialog_ == nullptr && !waiting && NowNs() >= input_resume_ns_.load();
}

void KetsuiApp::OnReleaseEvent(SDL_Event* event) {
  if (!input_release_pending_.load() || event->type != input_release_type_) {
    return;
  }
  const uint32_t code = event->type == SDL_EVENT_KEY_UP
                            ? static_cast<uint32_t>(event->key.key)
                            : static_cast<uint32_t>(event->gbutton.button);
  if (code == input_release_code_) {
    input_resume_ns_.store(NowNs() + kInputResumeDelayNs);
    input_release_pending_.store(false);
  }
}

void KetsuiApp::OpenExitDialog() {
  // While the dialog is open, the game receives no input (so A cannot enter CONFIG).
  auto* input_sys = static_cast<rex::input::InputSystem*>(runtime()->input_system());
  if (input_sys) {
    input_sys->SetActiveCallback([this]() { return InputAllowed(); });
  }
  exit_dialog_ = new ExitConfirmDialog(imgui_drawer(), [this](bool yes) { OnExitAnswer(yes); });
}

void KetsuiApp::OnGamepadButton(SDL_GamepadButton button) {
  if (exit_dialog_) {
    switch (button) {
      case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
      case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        exit_dialog_->Move();
        break;
      case SDL_GAMEPAD_BUTTON_SOUTH:
        ArmInputRelease(SDL_EVENT_GAMEPAD_BUTTON_UP, SDL_GAMEPAD_BUTTON_SOUTH);
        exit_dialog_->Activate();
        break;
      case SDL_GAMEPAD_BUTTON_EAST:
        ArmInputRelease(SDL_EVENT_GAMEPAD_BUTTON_UP, SDL_GAMEPAD_BUTTON_EAST);
        exit_dialog_->Cancel();
        break;
      default:
        break;
    }
    return;
  }
  if (button == SDL_GAMEPAD_BUTTON_EAST && IsOnMainMenu()) {
    OpenExitDialog();
  }
}

bool SDLCALL KetsuiApp::OnSdlEvent(void* userdata, SDL_Event* event) {
  static_cast<KetsuiApp*>(userdata)->OnReleaseEvent(event);
  if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
    static_cast<KetsuiApp*>(userdata)->OnGamepadButton(
        static_cast<SDL_GamepadButton>(event->gbutton.button));
  }
  return true;
}

void KetsuiApp::OnExitAnswer(bool yes) {
  // The dialog deletes itself once it has closed.
  exit_dialog_ = nullptr;
  if (yes && window()) {
    window()->RequestClose();
  }
}

void KetsuiApp::OnPreSetup(rex::RuntimeConfig& config) {
  // Without a GPU plugin the game runs with no graphics at all: default to the
  // Xenos emulation, unless --gpu_plugin names another one.
  if (config.gpu_plugin.empty()) {
    config.gpu_plugin = "xenos";
  }
}

void KetsuiApp::OnPostSetup() {
  SDL_AddEventWatch(&KetsuiApp::OnSdlEvent, this);
  // The SDK defines --perf_log_csv but never opens the file: nothing calls this.
  rex::perf::SetCsvLogPath(REXCVAR_GET(perf_log_csv));
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
