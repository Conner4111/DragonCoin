// Copyright (c) 2021-present The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT.

#include <util/thread.h>

#include <logging.h>
#include <util/system.h>
#include <util/threadnames.h>

#include <exception>
#include <functional>
#include <string>
#include <utility>

void util::TraceThread(std::string_view thread_name, std::function<void()> thread_func)
{
    util::ThreadRename(std::string{thread_name});
    try {
        LogPrintf("%s thread start\n", thread_name);
        thread_func();
        LogPrintf("%s thread exit\n", thread_name);
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, thread_name);
        throw;
    } catch (...) {
        PrintExceptionContinue(nullptr, thread_name);
        throw;
    }
}
