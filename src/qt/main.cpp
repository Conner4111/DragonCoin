// Copyright (c) 2018-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT.

#include <qt/bitcoin.h>

#include <common/url.h>
#include <compat/compat.h>
#include <util/translation.h>

#include <QCoreApplication>

#include <functional>
#include <string>

/** Translate string to current locale using Qt. */
extern const std::function<std::string(const char*)> G_TRANSLATION_FUN = [](const char* psz) {
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
};
UrlDecodeFn* const URL_DECODE = urlDecode;

MAIN_FUNCTION
{
    return GuiMain(argc, argv);
}
