#ifndef TEAPOT_JNI_STATE_MANAGER_H
#define TEAPOT_JNI_STATE_MANAGER_H

#ifdef __OBJC__
#include <objc/NSObjCRuntime.h>
#endif

#include "gpg/achievement.h"
#include "gpg/achievement_manager.h"
#include "gpg/builder.h"
#include "gpg/debug.h"
#include "gpg/default_callbacks.h"
#include "gpg/game_services.h"
#include "gpg/leaderboard.h"
#include "gpg/leaderboard_manager.h"
#include "gpg/platform_configuration.h"
#include "gpg/player_manager.h"
#include "gpg/score_page.h"
#include "gpg/types.h"

class StateManager {
 public:
  static void InitServices(
      gpg::PlatformConfiguration const &pc,
      gpg::GameServices::Builder::OnAuthActionStartedCallback started_callback,
      gpg::GameServices::Builder::OnAuthActionFinishedCallback
          finished_callback);
  static gpg::GameServices *GetGameServices();
  static void BeginUserInitiatedSignIn();
  static void SignOut();
  static void UnlockAchievement(const char *achievementId);
  static void SubmitHighScore(const char *leaderboardId, uint64_t score);
  static void ShowAchievements();
  static void ShowLeaderboard(const char *leaderboardId);
  static bool IsAuthInProgress() {
    return is_auth_in_progress_;
  }

 private:
  static bool is_auth_in_progress_;
  static std::unique_ptr<gpg::GameServices> game_services_;
};

#endif  // TEAPOT_JNI_STATE_MANAGER_H
