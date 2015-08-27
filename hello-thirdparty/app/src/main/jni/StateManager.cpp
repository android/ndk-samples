// Copyright (c) 2014 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "StateManager.h"

#include "android/log.h"
#define DEBUG_TAG "TeapotNativeActivity"
#define LOGI(...) \
    ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))

#include "gpg/achievement_manager.h"
bool StateManager::is_auth_in_progress_ = false;
std::unique_ptr<gpg::GameServices> StateManager::game_services_;

void OnAuthActionFinished(gpg::AuthOperation op, gpg::AuthStatus status) {
  LOGI("OnAuthActionFinished");
}

void OnAuthActionStarted(gpg::AuthOperation op) {
  LOGI("OnAuthActionStarted");
  switch ( op ) {
  case gpg::AuthOperation::SIGN_IN:
    LOGI("Signing In");
    break;
  case gpg::AuthOperation::SIGN_OUT:
    LOGI("Signing Out");
    break;
  }
}

gpg::GameServices *StateManager::GetGameServices() {
  return game_services_.get();
}

void StateManager::BeginUserInitiatedSignIn() {
  if (!game_services_->IsAuthorized()) {
    LOGI("StartAuthorizationUI");
    game_services_->StartAuthorizationUI();
  }
}

void StateManager::SignOut() {
  if (game_services_->IsAuthorized()) {
    LOGI("SignOut");
    game_services_->SignOut();
  }
}

void StateManager::UnlockAchievement(char const *achievement_id) {
  if (game_services_->IsAuthorized()) {
    LOGI("Achievement unlocked");
    game_services_->Achievements().Unlock(achievement_id);
  }
}

void StateManager::SubmitHighScore(char const *leaderboard_id, uint64_t score) {
  if (game_services_->IsAuthorized()) {
    LOGI("High score submitted");
    game_services_->Leaderboards().SubmitScore(leaderboard_id, score);
  }
}

void StateManager::ShowAchievements() {
  if (game_services_->IsAuthorized()) {
    LOGI("Show achievement");
    game_services_->Achievements().ShowAllUI();
  }
}

void StateManager::ShowLeaderboard(char const *leaderboard_id) {
  if (game_services_->IsAuthorized()) {
    LOGI("Show achievement");
    game_services_->Leaderboards().ShowUI(leaderboard_id);
  }
}


void StateManager::InitServices(
    gpg::PlatformConfiguration const &pc,
    gpg::GameServices::Builder::OnAuthActionStartedCallback started_callback,
    gpg::GameServices::Builder::OnAuthActionFinishedCallback
        finished_callback) {
  LOGI("Initializing Services");
  if (!game_services_) {
    LOGI("Uninitialized services, so creating");
    game_services_ = gpg::GameServices::Builder()
        .SetLogging(gpg::DEFAULT_ON_LOG, gpg::LogLevel::VERBOSE)
        .SetOnAuthActionStarted([started_callback](gpg::AuthOperation op) {
          is_auth_in_progress_ = true;
          if (started_callback != nullptr)
            started_callback(op);
        })
        .SetOnAuthActionFinished([finished_callback](gpg::AuthOperation op,
                                                     gpg::AuthStatus status) {
          LOGI("Sign in finished with a result of %d", status);
          is_auth_in_progress_ = false;
          if (finished_callback != nullptr)
            finished_callback(op, status);
          LOGI("Fetching all blocking");
          gpg::AchievementManager::FetchAllResponse fetchResponse = game_services_->Achievements().FetchAllBlocking(std::chrono::milliseconds(1000));
          LOGI("--------------------------------------------------------------");

          LOGI("Fetching all nonblocking");
          game_services_->Achievements().FetchAll(gpg::DataSource::CACHE_OR_NETWORK, [] (gpg::AchievementManager::FetchAllResponse response) {LOGI("Achievement response status: %d", response.status);});
          LOGI("--------------------------------------------------------------");

        })
        .Create(pc);
  }
  LOGI("Created");
}
