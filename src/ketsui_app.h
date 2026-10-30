// ketsui - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>

#include <rex/rex_app.h>

#include "generated/default/ketsui_init.h"

class ExitConfirmDialog;

class KetsuiApp : public rex::ReXApp {
 public:
  using rex::ReXApp::ReXApp;

  static std::unique_ptr<rex::ui::WindowedApp> Create(
      rex::ui::WindowedAppContext& ctx) {
    return std::unique_ptr<KetsuiApp>(new KetsuiApp(ctx, "ketsui",
        PPCImageConfig));
  }

  void OnConfigurePaths(rex::PathConfig& paths) override;
  void OnPreSetup(rex::RuntimeConfig& config) override;
  void OnPostSetup() override;
  void OnKeyDown(rex::ui::KeyEvent& e) override;
  std::optional<rex::PathConfig> OnFinalizePaths(
      const rex::PathConfig& defaults,
      std::function<void(rex::PathConfig)> resume) override;

 private:
  void AskForGameFolder();
  void OpenExitDialog();
  void OnExitAnswer(bool yes);
  bool IsOnMainMenu() const;
  void OnGamepadButton(SDL_GamepadButton button);
  static bool SDLCALL OnSdlEvent(void* userdata, SDL_Event* event);
  // After the dialog is closed by a button or key, the game must not see that
  // press: its input stays blocked until the release event (plus a short delay).
  void ArmInputRelease(SDL_EventType release_type, uint32_t release_code);
  bool InputAllowed() const;
  void OnReleaseEvent(SDL_Event* event);
  std::atomic<bool> input_release_pending_{false};
  std::atomic<int64_t> input_resume_ns_{0};
  std::atomic<int64_t> input_armed_ns_{0};
  SDL_EventType input_release_type_ = SDL_EVENT_FIRST;
  uint32_t input_release_code_ = 0;
  void OnGameFolderPicked(const std::string& picked);
  static void SDLCALL OnFolderDialogDone(void* userdata, const char* const* filelist,
                                         int filter);

  ExitConfirmDialog* exit_dialog_ = nullptr;
  rex::PathConfig defaults_;
  std::function<void(rex::PathConfig)> resume_;
};
