// Copyright (c) 2016-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT.

#ifndef BITCOIN_WALLET_WALLETTOOL_H
#define BITCOIN_WALLET_WALLETTOOL_H

#include <string>

class ArgsManager;

namespace wallet {
namespace WalletTool {

bool ExecuteWalletToolFunc(const ArgsManager& args, const std::string& command);

} // namespace WalletTool
} // namespace wallet

#endif // BITCOIN_WALLET_WALLETTOOL_H
