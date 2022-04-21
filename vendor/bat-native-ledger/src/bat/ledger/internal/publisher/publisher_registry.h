/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_PUBLISHER_PUBLISHER_REGISTRY_H_
#define BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_PUBLISHER_PUBLISHER_REGISTRY_H_

#include <string>

#include "bat/ledger/internal/core/bat_ledger_context.h"
#include "bat/ledger/internal/core/future.h"

namespace ledger {

// Provides access to the publicly-accessible Brave publisher registry.
class PublisherRegistry : public BATLedgerContext::Object {
 public:
  inline static const char kContextKey[] = "publisher-registry";

  // Returns a value indicating whether the specified publisher is registered.
  // A returned value of |true| does not necessarily indicate that the publisher
  // has established a "verified" wallet that is capable of accepting tips.
  Future<bool> IsPublisherRegistered(const std::string& publisher_id);
};

}  // namespace ledger

#endif  // BRAVE_VENDOR_BAT_NATIVE_LEDGER_SRC_BAT_LEDGER_INTERNAL_PUBLISHER_PUBLISHER_REGISTRY_H_
