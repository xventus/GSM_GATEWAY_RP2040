//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   at_enums.h
/// @author Petr Vanek

#pragma once

#include <algorithm>
#include <inttypes.h>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std::literals;

namespace gsm {

  enum class ParseCode
    {
        qBegin,   // query begin
        qContent, // query content
        aBegin,   // answer begin
        aContent, // answer content
        aEnd
    };

    enum class QueryType
    {
        unknown,
        test,
        get,
        set,
        urc,
        exec
    };

    const std::unordered_set<std::string_view> AtBeginLUT = {

        "+"sv,  // AT+...
        "#"sv,  // AT#...
        "$"sv,  // AT$...
        "%"sv,  // AT%...
        "\\"sv, // AT\...
        "&"sv,   // AT&...
    };

    enum class ResponseStatus
    {
        ok,
        error,
        status,
        unknown,
        clock,
        smsready,
        busy,
        ring,
        nocarrier,
        newsms,
        callerid,
        nodial,
        msgnum
    };

    using Responses = std::vector<std::string>;
    using ReqMap = std::unordered_map<std::string_view, QueryType>;
    using RespMapStatus = std::unordered_map<std::string_view, ResponseStatus>;
    using KeyValue = std::pair<std::string, std::string>;

    const ReqMap AtEndLUT{
        {"=?"sv, QueryType::test}, // e.g. AT+COPS=?
        {"?"sv, QueryType::get},   // e.g  AT+CPIN?
        {"="sv, QueryType::set},   // e.g. AT+CBC =”+923140”, 110
        {":"sv, QueryType::urc},
        {"\r"sv, QueryType::exec} // AT+CSQ,
    };

    const RespMapStatus ATStatus{
        {"CLIP"sv, ResponseStatus::callerid},  ///< special case
        {"> ", ResponseStatus::smsready},       ///< special case
        
        {"RING"sv, ResponseStatus::ring},
        {"OK"sv, ResponseStatus::ok},
        {"CPMS"sv, ResponseStatus::msgnum},
        
        {"ERROR"sv, ResponseStatus::error},
        {"NOT READY"sv, ResponseStatus::status},
        {"CONNECT OK"sv, ResponseStatus::status},
        {"CONNECT FAIL"sv, ResponseStatus::error},
        {"SEND OK"sv, ResponseStatus::status},
        {"SEND FAIL"sv, ResponseStatus::error},
        {"DATA ACCEPT"sv, ResponseStatus::status},
        {"CLOSED"sv, ResponseStatus::status},
        {">"sv, ResponseStatus::status},
        {"VOICE CALL: END"sv, ResponseStatus::status},
        {"CALL READY", ResponseStatus::status},
        {"SMS READY", ResponseStatus::status},
        {"NORMAL POWER DOWN", ResponseStatus::status},
        {"CCLK:", ResponseStatus::clock},
        
        {"BUSY", ResponseStatus::busy},
        

        {"CMTI",ResponseStatus::newsms},
        {"NO CARRIER", ResponseStatus::nocarrier},
        {"NO DIALTONE", ResponseStatus::nodial},
        
        {"UNKNOWN"sv, ResponseStatus::unknown} // fake state

    };

    const char _lineDelim = '\r';
    const char _ignorelineDelim = '\n';

} //namespace gsm 