// Copyright (c) 2023-2024 The Dash Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <coinjoin/context.h>

#include <net.h>
#include <txmempool.h>
#include <validation.h>

#ifdef ENABLE_WALLET
#include <coinjoin/client.h>
#endif // ENABLE_WALLET
#include <coinjoin/server.h>

CJContext::CJContext(CChainState& chainstate, CConnman& connman, CDeterministicMNManager& dmnman, CTxMemPool& mempool,
                     const CActiveMasternodeManager* mn_activeman, const CMasternodeSync& mn_sync, bool relay_txes) :
    dstxman{std::make_unique<CDSTXManager>()},
#ifdef ENABLE_WALLET
    walletman{std::make_unique<CoinJoinWalletManager>(connman, dmnman, mempool, mn_sync, queueman)},
    queueman {relay_txes ? std::make_unique<CCoinJoinClientQueueManager>(connman, *walletman, dmnman, mn_sync) : nullptr},
#endif // ENABLE_WALLET
    server{std::make_unique<CCoinJoinServer>(chainstate, connman, dmnman, *dstxman, mempool, mn_activeman, mn_sync)}
{}

CJContext::~CJContext() {}