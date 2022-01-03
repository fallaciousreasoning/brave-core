/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/eth_nonce_tracker.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "brave/components/brave_wallet/browser/eth_json_rpc_controller.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/components/brave_wallet/common/eth_address.h"

namespace brave_wallet {

namespace {

uint256_t GetHighestLocallyConfirmed(
    const std::vector<std::unique_ptr<EthTxStateManager::TxMeta>>& metas) {
  uint256_t highest = 0;
  for (auto& meta : metas) {
    DCHECK(meta->tx->nonce());  // Not supposed to happen for a confirmed tx.
    highest = std::max(highest, meta->tx->nonce().value() + (uint256_t)1);
  }
  return highest;
}

uint256_t GetHighestContinuousFrom(
    const std::vector<std::unique_ptr<EthTxStateManager::TxMeta>>& metas,
    uint256_t start) {
  uint256_t highest = start;
  for (auto& meta : metas) {
    DCHECK(meta->tx->nonce());  // Not supposed to happen for a submitted tx.
    if (meta->tx->nonce().value() == highest)
      highest++;
  }
  return highest;
}

}  // namespace

EthNonceTracker::EthNonceTracker(EthTxStateManager* tx_state_manager,
                                 EthJsonRpcController* rpc_controller)
    : tx_state_manager_(tx_state_manager),
      rpc_controller_(rpc_controller),
      weak_factory_(this) {}
EthNonceTracker::~EthNonceTracker() = default;

void EthNonceTracker::GetNextNonce(const EthAddress& from,
                                   GetNextNonceCallback callback) {
  const std::string hex_address = from.ToHex();
  rpc_controller_->GetTransactionCount(
      hex_address, base::BindOnce(&EthNonceTracker::OnGetNetworkNonce,
                                  weak_factory_.GetWeakPtr(), EthAddress(from),
                                  std::move(callback)));
}

void EthNonceTracker::OnGetNetworkNonce(EthAddress from,
                                        GetNextNonceCallback callback,
                                        uint256_t network_nonce,
                                        mojom::ProviderError error,
                                        const std::string& error_message) {
  if (!nonce_lock_.Try()) {
    std::move(callback).Run(false, network_nonce);
    return;
  }
  // TODO(darkdh): check status
  tx_state_manager_->GetTransactionsByStatus(
      mojom::TransactionStatus::Confirmed, from,
      base::BindOnce(&EthNonceTracker::OnGetConfirmedTxs,
                     weak_factory_.GetWeakPtr(), from, std::move(callback),
                     network_nonce));

  nonce_lock_.Release();
}

void EthNonceTracker::OnGetConfirmedTxs(
    EthAddress from,
    GetNextNonceCallback callback,
    uint256_t network_nonce,
    std::vector<std::unique_ptr<EthTxStateManager::TxMeta>> confirmed_txs) {
  if (!nonce_lock_.Try()) {
    std::move(callback).Run(false, network_nonce);
    return;
  }
  uint256_t local_highest = GetHighestLocallyConfirmed(confirmed_txs);

  uint256_t highest_confirmed = std::max(network_nonce, local_highest);

  tx_state_manager_->GetTransactionsByStatus(
      mojom::TransactionStatus::Submitted, from,
      base::BindOnce(&EthNonceTracker::OnGetPendingTxs,
                     weak_factory_.GetWeakPtr(), from, std::move(callback),
                     network_nonce, highest_confirmed));
  nonce_lock_.Release();
}

void EthNonceTracker::OnGetPendingTxs(
    EthAddress from,
    GetNextNonceCallback callback,
    uint256_t network_nonce,
    uint256_t highest_confirmed,
    std::vector<std::unique_ptr<EthTxStateManager::TxMeta>> pending_txs) {
  if (!nonce_lock_.Try()) {
    std::move(callback).Run(false, network_nonce);
    return;
  }
  uint256_t highest_continuous_from =
      GetHighestContinuousFrom(pending_txs, highest_confirmed);

  uint256_t nonce = std::max(network_nonce, highest_continuous_from);
  nonce_lock_.Release();
  std::move(callback).Run(true, nonce);
}

}  // namespace brave_wallet
