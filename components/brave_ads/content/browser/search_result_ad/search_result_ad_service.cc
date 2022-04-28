/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/content/browser/search_result_ad/search_result_ad_service.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/callback.h"
#include "base/containers/contains.h"
#include "base/containers/fixed_flat_set.h"
#include "base/containers/flat_set.h"
#include "base/feature_list.h"
#include "base/strings/string_number_conversions.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/common/features.h"
#include "brave/components/brave_search/common/brave_search_utils.h"
#include "content/public/browser/render_frame_host.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

using ads::mojom::Conversion;
using ads::mojom::ConversionPtr;
using ads::mojom::SearchResultAd;
using ads::mojom::SearchResultAdPtr;

namespace brave_ads {

namespace {

constexpr char kProductType[] = "Product";
constexpr char kSearchResultAdType[] = "SearchResultAd";

constexpr char kContextPropertyName[] = "@context";
constexpr char kTypePropertyName[] = "@type";

constexpr char kDataPlacementId[] = "data-placement-id";
constexpr char kDataCreativeInstanceId[] = "data-creative-instance-id";
constexpr char kDataCreativeSetId[] = "data-creative-set-id";
constexpr char kDataCampaignId[] = "data-campaign-id";
constexpr char kDataAdvertiserId[] = "data-advertiser-id";
constexpr char kDataLandingPage[] = "data-landing-page";
constexpr char kDataHeadlineText[] = "data-headline-text";
constexpr char kDataDescription[] = "data-description";
constexpr char kDataRewardsValue[] = "data-rewards-value";
constexpr char kDataConversionTypeValue[] = "data-conversion-type-value";
constexpr char kDataConversionUrlPatternValue[] =
    "data-conversion-url-pattern-value";
constexpr char kDataConversionAdvertiserPublicKeyValue[] =
    "data-conversion-advertiser-public-key-value";
constexpr char kDataConversionObservationWindowValue[] =
    "data-conversion-observation-window-value";

constexpr auto kSearchResultAdAttributes =
    base::MakeFixedFlatSet<base::StringPiece>(
        {kDataPlacementId, kDataCreativeInstanceId, kDataCreativeSetId,
         kDataCampaignId, kDataAdvertiserId, kDataLandingPage,
         kDataHeadlineText, kDataDescription, kDataRewardsValue,
         kDataConversionTypeValue, kDataConversionUrlPatternValue,
         kDataConversionAdvertiserPublicKeyValue,
         kDataConversionObservationWindowValue});

bool GetStringValue(const schema_org::mojom::PropertyPtr& ad_property,
                    std::string* out_value) {
  DCHECK(ad_property);
  DCHECK(out_value);

  // Wrong attribute type
  if (!ad_property->values->is_string_values() ||
      ad_property->values->get_string_values().size() != 1) {
    return false;
  }

  *out_value = ad_property->values->get_string_values().front();

  return true;
}

bool GetIntValue(const schema_org::mojom::PropertyPtr& ad_property,
                 int32_t* out_value) {
  DCHECK(ad_property);
  DCHECK(out_value);

  // Wrong attribute type
  if (!ad_property->values->is_long_values() ||
      ad_property->values->get_long_values().size() != 1) {
    return false;
  }

  *out_value = ad_property->values->get_long_values().front();

  return true;
}

bool GetDoubleValue(const schema_org::mojom::PropertyPtr& ad_property,
                    double* out_value) {
  DCHECK(ad_property);
  DCHECK(out_value);

  // Wrong attribute type
  if (!ad_property->values->is_string_values() ||
      ad_property->values->get_string_values().size() != 1) {
    return false;
  }

  std::string value = ad_property->values->get_string_values().front();
  return base::StringToDouble(value, out_value);
}

bool SetSearchAdProperty(const schema_org::mojom::PropertyPtr& ad_property,
                         SearchResultAd* search_result_ad) {
  DCHECK(ad_property);
  DCHECK(search_result_ad);
  DCHECK(search_result_ad->conversion);

  const std::string& name = ad_property->name;
  if (name == kDataPlacementId) {
    return GetStringValue(ad_property, &search_result_ad->placement_id);
  } else if (name == kDataCreativeInstanceId) {
    return GetStringValue(ad_property, &search_result_ad->creative_instance_id);
  } else if (name == kDataCreativeSetId) {
    return GetStringValue(ad_property, &search_result_ad->creative_set_id);
  } else if (name == kDataCampaignId) {
    return GetStringValue(ad_property, &search_result_ad->campaign_id);
  } else if (name == kDataAdvertiserId) {
    return GetStringValue(ad_property, &search_result_ad->advertiser_id);
  } else if (name == kDataLandingPage) {
    return GetStringValue(ad_property, &search_result_ad->target_url);
  } else if (name == kDataHeadlineText) {
    return GetStringValue(ad_property, &search_result_ad->headline_text);
  } else if (name == kDataDescription) {
    return GetStringValue(ad_property, &search_result_ad->description);
  } else if (name == kDataRewardsValue) {
    return GetDoubleValue(ad_property, &search_result_ad->value);
  } else if (name == kDataConversionTypeValue) {
    return GetStringValue(ad_property, &search_result_ad->conversion->type);
  } else if (name == kDataConversionUrlPatternValue) {
    return GetStringValue(ad_property,
                          &search_result_ad->conversion->url_pattern);
  } else if (name == kDataConversionAdvertiserPublicKeyValue) {
    return GetStringValue(ad_property,
                          &search_result_ad->conversion->advertiser_public_key);
  } else if (name == kDataConversionObservationWindowValue) {
    return GetIntValue(ad_property,
                       &search_result_ad->conversion->observation_window);
  }

  NOTREACHED();

  return false;
}

absl::optional<SearchResultAdMap> ParseSearchResultAdMapEntityProperties(
    const schema_org::mojom::EntityPtr& entity) {
  DCHECK(entity);
  DCHECK_EQ(entity->type, kProductType);

  SearchResultAdMap search_result_ads;

  for (const auto& property : entity->properties) {
    if (!property || property->name == kContextPropertyName ||
        property->name == kTypePropertyName) {
      continue;
    }

    // Search result ads list product could have only "@context" and "creatives"
    // properties.
    if (property->name != "creatives") {
      return absl::nullopt;
    }

    if (!property->values->is_entity_values() ||
        property->values->get_entity_values().empty()) {
      LOG(ERROR) << "Search result ad attributes list is empty";
      return SearchResultAdMap();
    }

    for (const auto& ad_entity : property->values->get_entity_values()) {
      if (!ad_entity || ad_entity->type != kSearchResultAdType) {
        LOG(ERROR) << "Wrong search result ad type specified: "
                   << ad_entity->type;
        return SearchResultAdMap();
      }

      if (property->name == kTypePropertyName) {
        continue;
      }

      SearchResultAdPtr search_result_ad = SearchResultAd::New();
      search_result_ad->conversion = Conversion::New();

      base::flat_set<base::StringPiece> found_attributes;
      for (const auto& ad_property : ad_entity->properties) {
        // Wrong attribute name
        const auto* it = std::find(kSearchResultAdAttributes.begin(),
                                   kSearchResultAdAttributes.end(),
                                   base::StringPiece(ad_property->name));

        if (it == kSearchResultAdAttributes.end()) {
          LOG(ERROR) << "Wrong search result ad attribute specified: "
                     << ad_property->name;
          return SearchResultAdMap();
        }
        found_attributes.insert(*it);

        if (!SetSearchAdProperty(ad_property, search_result_ad.get())) {
          LOG(ERROR) << "Cannot read search result ad attribute value: "
                     << ad_property->name;
          return SearchResultAdMap();
        }
      }

      // Not all of attributes were specified.
      if (found_attributes.size() != kSearchResultAdAttributes.size()) {
        std::vector<base::StringPiece> absent_attributes;
        std::set_difference(kSearchResultAdAttributes.begin(),
                            kSearchResultAdAttributes.end(),
                            found_attributes.begin(), found_attributes.end(),
                            std::back_inserter(absent_attributes));

        LOG(ERROR) << "Some of search result ad attributes were not specified: "
                   << base::JoinString(absent_attributes, ", ");

        return SearchResultAdMap();
      }

      const std::string creative_instance_id =
          search_result_ad->creative_instance_id;
      search_result_ads.emplace(creative_instance_id,
                                std::move(search_result_ad));
    }

    // Creatives has been parsed.
    break;
  }

  return search_result_ads;
}

void LogSearchResultAdMap(const SearchResultAdMap& search_result_ads) {
  if (!VLOG_IS_ON(1)) {
    return;
  }

  if (search_result_ads.empty()) {
    VLOG(1) << "Parsed search result ads list is empty.";
    return;
  }

  VLOG(1) << "Parsed search result ads list:";
  for (const auto& search_result_ad_pair : search_result_ads) {
    const auto& search_result_ad = search_result_ad_pair.second;
    VLOG(1) << "Ad with \"" << kDataPlacementId
            << "\": " << search_result_ad->placement_id;
    VLOG(1) << "  \"" << kDataCreativeInstanceId
            << "\": " << search_result_ad->creative_instance_id;
    VLOG(1) << "  \"" << kDataCreativeSetId
            << "\": " << search_result_ad->creative_set_id;
    VLOG(1) << "  \"" << kDataCampaignId
            << "\": " << search_result_ad->campaign_id;
    VLOG(1) << "  \"" << kDataAdvertiserId
            << "\": " << search_result_ad->advertiser_id;
    VLOG(1) << "  \"" << kDataLandingPage
            << "\": " << search_result_ad->target_url;
    VLOG(1) << "  \"" << kDataHeadlineText
            << "\": " << search_result_ad->headline_text;
    VLOG(1) << "  \"" << kDataDescription
            << "\": " << search_result_ad->description;
    VLOG(1) << "  \"" << kDataRewardsValue << "\": " << search_result_ad->value;
    VLOG(1) << "  \"" << kDataConversionTypeValue
            << "\": " << search_result_ad->conversion->type;
    VLOG(1) << "  \"" << kDataConversionUrlPatternValue
            << "\": " << search_result_ad->conversion->url_pattern;
    VLOG(1) << "  \"" << kDataConversionAdvertiserPublicKeyValue
            << "\": " << search_result_ad->conversion->advertiser_public_key;
    VLOG(1) << "  \"" << kDataConversionObservationWindowValue
            << "\": " << search_result_ad->conversion->observation_window;
  }
}

SearchResultAdMap ParseWebPageEntities(blink::mojom::WebPagePtr web_page) {
  for (const auto& entity : web_page->entities) {
    if (entity->type != kProductType) {
      continue;
    }

    absl::optional<SearchResultAdMap> search_result_ads =
        ParseSearchResultAdMapEntityProperties(entity);

    if (search_result_ads) {
      LogSearchResultAdMap(*search_result_ads);
      return std::move(*search_result_ads);
    }
  }

  VLOG(1) << "No search result ad found.";

  return SearchResultAdMap();
}

}  // namespace

SearchResultAdService::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo() =
    default;
SearchResultAdService::AdViewedEventCallbackInfo::AdViewedEventCallbackInfo(
    AdViewedEventCallbackInfo&& info) = default;
SearchResultAdService::AdViewedEventCallbackInfo&
SearchResultAdService::AdViewedEventCallbackInfo::operator=(
    AdViewedEventCallbackInfo&& info) = default;
SearchResultAdService::AdViewedEventCallbackInfo::~AdViewedEventCallbackInfo() =
    default;

SearchResultAdService::SearchResultAdService(AdsService* ads_service)
    : ads_service_(ads_service) {
  DCHECK(ads_service_);
}

SearchResultAdService::~SearchResultAdService() = default;

void SearchResultAdService::MaybeRetrieveSearchResultAd(
    content::RenderFrameHost* render_frame_host,
    SessionID tab_id,
    bool should_trigger_viewed_event) {
  DCHECK(ads_service_);
  DCHECK(render_frame_host);

  if (!should_trigger_viewed_event || !ads_service_->IsEnabled() ||
      !base::FeatureList::IsEnabled(
          features::kSupportBraveSearchResultAdConfirmationEvents) ||
      !brave_search::IsAllowedHost(render_frame_host->GetLastCommittedURL())) {
    if (metadata_request_finished_callback_for_testing_) {
      std::move(metadata_request_finished_callback_for_testing_).Run();
    }
    search_result_ads_[tab_id] = SearchResultAdMap();
    RunAdViewedEventPendingCallbacks(tab_id, /* ads_fetched */ false);
    return;
  }

  mojo::Remote<blink::mojom::DocumentMetadata> document_metadata;
  render_frame_host->GetRemoteInterfaces()->GetInterface(
      document_metadata.BindNewPipeAndPassReceiver());
  DCHECK(document_metadata.is_bound());
  document_metadata.reset_on_disconnect();

  blink::mojom::DocumentMetadata* raw_document_metadata =
      document_metadata.get();
  raw_document_metadata->GetEntities(base::BindOnce(
      &SearchResultAdService::OnRetrieveSearchResultAdEntities,
      weak_factory_.GetWeakPtr(), std::move(document_metadata), tab_id));
}

void SearchResultAdService::OnDidFinishNavigation(SessionID tab_id) {
  // Clear the tab state from the previous load.
  ResetState(tab_id);
}

void SearchResultAdService::OnTabClosed(SessionID tab_id) {
  // Clear the tab state in memory.
  ResetState(tab_id);
}

void SearchResultAdService::MaybeTriggerSearchResultAdViewedEvent(
    const std::string& creative_instance_id,
    SessionID tab_id,
    base::OnceCallback<void(bool)> callback) {
  DCHECK(ads_service_);
  DCHECK(!creative_instance_id.empty());
  DCHECK(tab_id.is_valid());

  if (!ads_service_->IsEnabled()) {
    std::move(callback).Run(/* event_triggered */ false);
    return;
  }

  // Check if search result ad JSON-LD wasn't processed yet.
  if (!base::Contains(search_result_ads_, tab_id)) {
    AdViewedEventCallbackInfo callback_info;
    callback_info.creative_instance_id = creative_instance_id;
    callback_info.callback = std::move(callback);
    ad_viewed_event_pending_callbacks_[tab_id].push_back(
        std::move(callback_info));
    return;
  }

  const bool event_triggered =
      QueueSearchResultAdViewedEvent(creative_instance_id, tab_id);
  std::move(callback).Run(event_triggered);
}

void SearchResultAdService::SetMetadataRequestFinishedCallbackForTesting(
    base::OnceClosure callback) {
  metadata_request_finished_callback_for_testing_ = std::move(callback);
}

AdsService* SearchResultAdService::SetAdsServiceForTesting(
    AdsService* ads_service) {
  AdsService* previous_ads_service = ads_service_.get();
  ads_service_ = ads_service;
  return previous_ads_service;
}

void SearchResultAdService::ResetState(SessionID tab_id) {
  DCHECK(tab_id.is_valid());

  ad_viewed_event_pending_callbacks_.erase(tab_id);
  search_result_ads_.erase(tab_id);
}

void SearchResultAdService::OnRetrieveSearchResultAdEntities(
    mojo::Remote<blink::mojom::DocumentMetadata> document_metadata,
    SessionID tab_id,
    blink::mojom::WebPagePtr web_page) {
  if (metadata_request_finished_callback_for_testing_) {
    std::move(metadata_request_finished_callback_for_testing_).Run();
  }

  if (!web_page) {
    search_result_ads_[tab_id] = SearchResultAdMap();
    RunAdViewedEventPendingCallbacks(tab_id, /* ads_fetched */ false);
    return;
  }

  SearchResultAdMap search_result_ads =
      ParseWebPageEntities(std::move(web_page));

  search_result_ads_.emplace(tab_id, std::move(search_result_ads));

  RunAdViewedEventPendingCallbacks(tab_id, /* ads_fetched */ true);
}

void SearchResultAdService::RunAdViewedEventPendingCallbacks(SessionID tab_id,
                                                             bool ads_fetched) {
  for (auto& callback_info : ad_viewed_event_pending_callbacks_[tab_id]) {
    bool event_triggered = false;
    if (ads_fetched) {
      event_triggered = QueueSearchResultAdViewedEvent(
          callback_info.creative_instance_id, tab_id);
    }
    if (event_triggered) {
      VLOG(1) << "Triggered search result ad viewed event for "
              << callback_info.creative_instance_id;
    } else {
      VLOG(1) << "Failed to trigger search result ad viewed event for "
              << callback_info.creative_instance_id;
    }
    std::move(callback_info.callback).Run(event_triggered);
  }
  ad_viewed_event_pending_callbacks_.erase(tab_id);
}

bool SearchResultAdService::QueueSearchResultAdViewedEvent(
    const std::string& creative_instance_id,
    SessionID tab_id) {
  DCHECK(!creative_instance_id.empty());
  DCHECK(tab_id.is_valid());

  SearchResultAdMap& ad_map = search_result_ads_[tab_id];
  auto it = ad_map.find(creative_instance_id);
  if (it == ad_map.end()) {
    return false;
  }

  ad_viewed_event_queue_.push_front(std::move(it->second));
  ad_map.erase(creative_instance_id);
  TriggerSearchResultAdViewedEventFromQueue();

  return true;
}

void SearchResultAdService::TriggerSearchResultAdViewedEventFromQueue() {
  DCHECK(ads_service_);
  DCHECK(!ad_viewed_event_queue_.empty() ||
         !trigger_ad_viewed_event_in_progress_);

  if (ad_viewed_event_queue_.empty() || trigger_ad_viewed_event_in_progress_) {
    return;
  }
  trigger_ad_viewed_event_in_progress_ = true;

  ads::mojom::SearchResultAdPtr search_result_ad =
      std::move(ad_viewed_event_queue_.back());
  ad_viewed_event_queue_.pop_back();

  ads_service_->TriggerSearchResultAdEvent(
      std::move(search_result_ad), ads::mojom::SearchResultAdEventType::kViewed,
      base::BindOnce(&SearchResultAdService::OnTriggerSearchResultAdViewedEvent,
                     weak_factory_.GetWeakPtr()));
}

void SearchResultAdService::OnTriggerSearchResultAdViewedEvent(
    const bool success,
    const std::string& placement_id,
    ads::mojom::SearchResultAdEventType ad_event_type) {
  trigger_ad_viewed_event_in_progress_ = false;
  TriggerSearchResultAdViewedEventFromQueue();

  if (!success) {
    VLOG(1) << "Error during processing of search result ad event for "
            << placement_id;
  }
}

}  // namespace brave_ads
