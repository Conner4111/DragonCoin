// Copyright (c) 2019-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT.

#include <consensus/validation.h>
#include <core_memusage.h>
#include <policy/policy.h>
#include <primitives/transaction.h>
#include <streams.h>
#include <test/fuzz/fuzz.h>
#include <version.h>

#include <cassert>

FUZZ_TARGET(tx_in)
{
    DataStream ds{buffer};
    CTxIn tx_in;
    try {
        ds >> tx_in;
    } catch (const std::ios_base::failure&) {
        return;
    }

    (void)GetTransactionInputWeight(tx_in);
    (void)GetVirtualTransactionInputSize(tx_in);
    (void)RecursiveDynamicUsage(tx_in);

    (void)tx_in.ToString();
}
