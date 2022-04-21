/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/brave_ads_host.h"

#include <utility>

#include "brave/browser/brave_ads/ads_service_factory.h"
#include "brave/browser/brave_rewards/rewards_panel_service_factory.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "chrome/browser/profiles/profile.h"

namespace brave_ads {

BraveAdsHost::BraveAdsHost(Profile* profile) : profile_(profile) {
  DCHECK(profile_);
}

BraveAdsHost::~BraveAdsHost() {}

void BraveAdsHost::RequestAdsEnabled(RequestAdsEnabledCallback callback) {
  DCHECK(callback);

  const AdsService* ads_service = AdsServiceFactory::GetForProfile(profile_);
  auto* panel_service =
      brave_rewards::RewardsPanelServiceFactory::GetForProfile(profile_);
  if (!panel_service || !ads_service || !ads_service->IsSupportedLocale()) {
    std::move(callback).Run(false);
    return;
  }

  if (ads_service->IsEnabled()) {
    std::move(callback).Run(true);
    return;
  }

  if (!callbacks_.empty()) {
    callbacks_.push_back(std::move(callback));
    return;
  }

  callbacks_.push_back(std::move(callback));

  panel_observation_.Observe(panel_service);

  if (!panel_service->ShowBraveTalkOptIn()) {
    RunCallbacksAndReset(false);
  }
}

void BraveAdsHost::OnRewardsPanelClosed(Browser* browser) {
  // TODO(zenparsing): Make sure that popup is not closed until after ads has
  // been successfully enabled. We might not be able to ensure that. Also, do
  // we want to listen again for "ads enabled" so that if they are enabled in
  // some other way we can hear that? Ideally, enabling Ads should be
  // synchronous (with perhaps asynchronous side-effects).
  auto* ads_service = AdsServiceFactory::GetForProfile(profile_);
  RunCallbacksAndReset(ads_service && ads_service->IsEnabled());
}

void BraveAdsHost::RunCallbacksAndReset(bool result) {
  DCHECK(!callbacks_.empty());

  panel_observation_.Reset();

  for (auto& callback : callbacks_) {
    std::move(callback).Run(result);
  }
  callbacks_.clear();
}

}  // namespace brave_ads
