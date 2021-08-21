// Copyright (c) 2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/context.h>
#include <wallet/wallet.h>
WalletContext::WalletContext(const WalletContext& contextIn)  {
    chain = contextIn.chain;
    args = contextIn.args;
    nodeContext = contextIn.nodeContext;
    wallets = contextIn.wallets;
}
WalletContext::WalletContext() {}
WalletContext::~WalletContext() {}
