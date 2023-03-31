//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   sms_command_analyzer.cpp
/// @author Petr Vanek

#include "sms_command_analyzer.h"
#include "src-utils/str_comparators.h"

std::tuple<GSMMessageType, uint32_t, bool> SmsCommandAnalyzer::analyze(std::string_view content)
{
    std::tuple<GSMMessageType, uint32_t, bool> rc = std::make_tuple(GSMMessageType::none, UINT32_MAX, false);

    for (const auto& item : gsmCommandArray) {
        if (findInsensitiveStr(content, item._cmd) != std::string::npos) {
            rc = std::make_tuple(item._msgT, item._pin, item._onOff);
            break;
        }
    }

    return rc;
}

std::string SmsCommandAnalyzer::listOfCommands() {
    std::string rc;
for (auto &r : gsmCommandArray)
        {
            if (!r._adm)
            {
                rc += r._cmd;
                rc += "\n";
            }
        }

    return rc;
}