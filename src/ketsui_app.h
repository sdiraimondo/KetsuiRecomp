// ketsui - ReXGlue Recompiled Project
//
// Customize your app by overriding virtual hooks from rex::ReXApp.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <SDL3/SDL_dialog.h>

#include <rex/rex_app.h>

#include "generated/default/ketsui_init.h"

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
  std::optional<rex::PathConfig> OnFinalizePaths(
      const rex::PathConfig& defaults,
      std::function<void(rex::PathConfig)> resume) override;

 private:
  void AskForGameFolder();
  void OnGameFolderPicked(const std::string& picked);
  static void SDLCALL OnFolderDialogDone(void* userdata, const char* const* filelist,
                                         int filter);

  rex::PathConfig defaults_;
  std::function<void(rex::PathConfig)> resume_;
};
