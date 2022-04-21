/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/publisher/publisher_registry.h"

#include <string>
#include <utility>

#include "bat/ledger/internal/core/bat_ledger_job.h"
#include "bat/ledger/internal/ledger_impl.h"

namespace ledger {

namespace {

struct Publisher {
  std::string id;
  bool registered = false;
};

class GetPublisherJob : public BATLedgerJob<absl::optional<Publisher>> {
 public:
  void Start(const std::string& publisher_id, bool use_prefix_list) {
    publisher_id_ = publisher_id;
    use_prefix_list_ = use_prefix_list;

    context().GetLedgerImpl()->database()->GetServerPublisherInfo(
        publisher_id,
        ContinueWithLambda(this, &GetPublisherJob::OnDatabaseRead));
  }

 private:
  void OnDatabaseRead(mojom::ServerPublisherInfoPtr publisher_info) {
    bool should_fetch =
        context().GetLedgerImpl()->publisher()->ShouldFetchServerPublisherInfo(
            publisher_info.get());

    if (!should_fetch) {
      return CompleteWith(std::move(publisher_info));
    }

    stale_data_ = std::move(publisher_info);

    if (use_prefix_list_) {
      context().GetLedgerImpl()->database()->SearchPublisherPrefixList(
          publisher_id_,
          ContinueWithLambda(this, &GetPublisherJob::OnPrefixListSearched));
      return;
    }

    FetchData();
  }

  void OnPrefixListSearched(bool publisher_exists) {
    if (!publisher_exists) {
      return CompleteWith(std::move(stale_data_));
    }

    FetchData();
  }

  void FetchData() {
    context().GetLedgerImpl()->publisher()->FetchServerPublisherInfo(
        publisher_id_,
        ContinueWithLambda(this, &GetPublisherJob::OnDataFetched));
  }

  void OnDataFetched(mojom::ServerPublisherInfoPtr publisher_info) {
    CompleteWith(std::move(publisher_info ? publisher_info : stale_data_));
  }

  void CompleteWith(mojom::ServerPublisherInfoPtr publisher) {
    if (!publisher) {
      return Complete({});
    }

    Publisher p;
    p.id = publisher->publisher_key;
    p.registered = publisher->status != mojom::PublisherStatus::NOT_VERIFIED;

    Complete(std::move(p));
  }

  std::string publisher_id_;
  bool use_prefix_list_ = false;
  mojom::ServerPublisherInfoPtr stale_data_;
};

}  // namespace

Future<bool> PublisherRegistry::IsPublisherRegistered(
    const std::string& publisher_id) {
  return context()
      .StartJob<GetPublisherJob>(publisher_id, true)
      .Then(base::BindOnce([](absl::optional<Publisher> publisher) {
        return publisher && publisher->registered;
      }));
}

}  // namespace ledger
