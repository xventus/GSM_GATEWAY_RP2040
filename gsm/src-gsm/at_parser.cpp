
//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   at_parser.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>

#include "at_parser.h"
#include "../src-utils/str_comparators.h"
#include "pico/util/datetime.h"
#include "src-utils/time_utils.h"

namespace gsm {

void ATParser::init(ParseCode startAt)
{
    _flow = startAt;
    invalidContnet();
}

void ATParser::invalidContnet()
{
    _query.clear();
    _answer.clear();
    _buffer.clear();
    _queryParam.clear();

    _queryType = QueryType::unknown;
    _status = ResponseStatus::unknown;
}

void ATParser::parseAnswerContent(char inp)
{

    if (inp == _lineDelim)
    {
        if (!_buffer.empty())
        {
            if (_prefferdResponse == ResponseStatus::unknown)
            {
                // search first valid status
                auto part = std::find_if(ATStatus.begin(), ATStatus.end(), [this](const auto &item)
                                         { return startsWith(_buffer, item.first); });

                if (part != ATStatus.end())
                {
                    // finished
                    _flow = ParseCode::aEnd;
                    _status = part->second;
                    _answer.emplace_back(_buffer);
                }
                else
                {
                    _answer.emplace_back(_buffer);
                    if (_answer.size() > _maxLinesResponse)
                    {
                        // invalid data - too long response or invalid data source
                        _status = ResponseStatus::unknown;
                        _flow = ParseCode::aEnd;
                    }
                }
            }
            else
            {
                auto part = std::find_if(ATStatus.begin(), ATStatus.end(), [this](const auto &item)
                                         { 
                    if (_prefferdResponse ==  item.second) {
                        return true;
                    }
                    return false; });

                if (part != ATStatus.end())
                {
                    // search preffered
                    _flow = ParseCode::aBegin;
                    if (containString(_buffer, part->first))
                    {
                        _status = part->second;
                        _answer.emplace_back(_buffer);
                        _flow = ParseCode::aEnd;
                    }
                    else
                    {
                        if (!_buffer.empty())
                        {
                            _answer.emplace_back(_buffer);
                        }
                    }
                }
            }
        }

        _buffer.clear();
    }
    else
    {
        _buffer += inp;
    }

    specialCase();
}

void ATParser::specialCase()
{
    if (_prefferdResponse == ResponseStatus::smsready || _prefferdResponse == ResponseStatus::unknown)
    {

        auto part = std::find_if(ATStatus.begin(), ATStatus.end(), [this](const auto &item)
                                 { return startsWith(_buffer, item.first); });

        if (part != ATStatus.end())
        {
            if (part->second == ResponseStatus::smsready)
            {
                _status = part->second;
                _answer.emplace_back(_buffer);
                _flow = ParseCode::aEnd;
            }

            if (part->second == ResponseStatus::callerid)
            { // there is no terminating character at the end
                // TODO: when it's time to optimize 
                _status = part->second;
                if (!betweenMarks(_buffer).empty()) {
                    _answer.emplace_back(_buffer);
                    _flow = ParseCode::aEnd;
                }
                
            }

        }
    }
    
}

void ATParser::parseQueryContent(char inp)
{
    _buffer += inp;
    auto part = std::find_if(AtEndLUT.begin(), AtEndLUT.end(), [this](const auto &item)
                             {
   
        auto rc = endsWith(_buffer, item.first);
        if (rc) {
            auto sz = item.first.length(); 
            // delete end delimiter
            _buffer.erase(_buffer.length()- sz, sz);
        }
        return rc; });

    if (part != AtEndLUT.end())
    {

        if (part->second == QueryType::exec)
        {
            // exec - finalize

            if (_queryType == QueryType::unknown)
            {
                // exec without param
                _queryType = part->second;
                _query = _buffer;
            }
            else
            {
                // collected query param
                _queryParam = _buffer;
            }

            _flow = ParseCode::aBegin;
        }
        else
        {
            _queryType = part->second;
            _query = _buffer;
        }

        _buffer.clear();
    }
}

void ATParser::parseAnswerBegin(char inp)
{

    if (inp != _lineDelim)
    {
        _buffer += inp;
        auto part = std::find_if(AtBeginLUT.begin(), AtBeginLUT.end(), [this](const auto &item)
                                 { return startsWith(_buffer, item); });

        if (part != AtBeginLUT.end())
        {
            _buffer.clear();
        }
        else
        {
            _flow = ParseCode::aContent;
        }
    }
}

bool ATParser::parse(std::string_view inp)
{
    auto rc = false;
    for (auto c : inp)
    {
        rc = parse(c);
        if (rc)
            break;
    }
    return rc;
}

bool ATParser::parse(char inp)
{
    std::string part;

    do
    {

        // ignore
        if (inp == _ignorelineDelim)
            break;

        switch (_flow)
        {

        // Query begin
        case ParseCode::qBegin:
            invalidContnet();
            _buffer += inp;
            _flow = ParseCode::qContent;
            break;

        // Query content
        case ParseCode::qContent:
            parseQueryContent(inp);
            break;

        case ParseCode::aBegin:

            parseAnswerBegin(inp);

            break;

        case ParseCode::aContent:
            parseAnswerContent(inp);
            break;

        case ParseCode::aEnd:
            // final parse
            break;
        }
    } while (false);

    return (_flow == ParseCode::aEnd);
}

std::string ATParser::betweenMarks(std::string_view strValue, std::string_view mark)
{
    std::string str;
    size_t begin = strValue.find(mark);

    if (begin != std::string::npos)
    {
        size_t end = strValue.find(mark, begin + 1);

        if (end != std::string::npos)
        {
            str = strValue.substr(begin + 1, end - begin - 1);
        }
    }
    return str;
}

bool ATParser::endsWith(std::string_view str, std::string_view suffix) const
{
    auto rc = false;
    do
    {
        if (str.size() < suffix.size())
            break;
        rc = compareInsensitiveStr(str.substr(str.size() - suffix.size(), suffix.size()), suffix);
    } while (false);
    return rc;
 
}

bool ATParser::startsWith(std::string_view str, std::string_view prefix) const
{
    return compareInsensitiveStr(str.substr(0, prefix.size()), prefix);
}

bool ATParser::containString(std::string_view str, std::string_view prefix) const
{
    return (findInsensitiveStr(str, prefix) != std::string::npos);
}

void ATParser::eatWhiteSpaces(std::string &input)
{
    input.erase(std::remove_if(input.begin(), input.end(), [](unsigned char x)
                               { return std::isspace(x); }),
                input.end());
}

KeyValue ATParser::getKeyValue(std::string_view input)
{
    KeyValue rc = std::make_pair("", "");
    auto splited = input.find(":");
    if (splited != std::string::npos)
    {
        auto key = std::string(input.substr(0, splited));
        eatWhiteSpaces(key);
        std::string value;
        if ((splited + 1) < input.length())
        {
            value = std::string(input.substr(splited + 1, input.length() - splited));
            eatWhiteSpaces(value);
        }
        rc = std::make_pair(key, value);
    }

    return rc;
}

std::optional<bool> ATParser::compareResponseText(std::string_view input)
{
    std::optional<bool> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);
            rc = compareInsensitiveStr(std::string_view(kv.second), input);
            break;
        }
    } while (false);

    return rc;
}

std::optional<std::string> ATParser::getText(std::string_view ignore)
{
    std::optional<std::string> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            if (x.empty())
                continue;
            if (!ignore.empty() && findInsensitiveStr(std::string_view(x), ignore) != std::string::npos)
                continue;
            eatWhiteSpaces(x);
            rc = x;
            break;
        }
    } while (false);

    return rc;
}

std::optional<std::tuple<datetime_t, bool, bool>> ATParser::evaluateGNSSTime(std::string_view item)
{
    std::optional<std::tuple<datetime_t, bool, bool>> rc = std::nullopt;
    std::string tm;
    int32_t fix;
    int32_t stat;
    datetime_t tmx;
    for (auto xx : _answer)
    {
        if (findInsensitiveStr((std::string_view)xx, item) == 0)
        {
            auto kv = getKeyValue(xx);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);
            tm.resize(kv.second.length() + 1);
            fix = 0;
            stat = 0;
            auto count = std::sscanf(kv.second.c_str(), "%d,%d,%[^,]",
                                     &stat, &fix, &tm[0]);

            if (count == 3 && tm.length() >= 14)
            {

                memset(&tmx, 0, sizeof(tmx));

                tmx.year = std::stoi(tm.substr(0, 4));
                tmx.month = std::stoi(tm.substr(4, 2));
                tmx.day = std::stoi(tm.substr(6, 2));
                tmx.hour = std::stoi(tm.substr(8, 2));
                tmx.min = std::stoi(tm.substr(10, 2));
                tmx.sec = std::stoi(tm.substr(12, 2));
                TimeUtils::updateDayOfWeek(tmx);
                rc = std::make_tuple(tmx, fix ? true : false, stat ? true : false);
            }
        }
    }

    return rc;
}

std::optional<std::tuple<std::string, std::string, std::string>> ATParser::evaluateSMS(std::string_view item)
{
    std::optional<std::tuple<std::string, std::string, std::string>> rc = std::nullopt;
    std::string tm;
    std::string id;
    std::string aux1;
    std::string aux2;
    for (auto xx : _answer)
    {
        if (findInsensitiveStr((std::string_view)xx, item) == 0)
        {
            auto kv = getKeyValue(xx);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);
            tm.resize(kv.second.length() + 1);
            id.resize(kv.second.length() + 1);
            aux1.resize(kv.second.length() + 1);
            aux2.resize(kv.second.length() + 1);
            auto count = std::sscanf(kv.second.c_str(), "%[^,],%[^,],%[^,],%s",
                                     &aux1[0], &id[0], &aux2[0], &tm[0]);
        }
        else if (!tm.empty() && !id.empty())
        {
            rc = std::make_tuple(betweenMarks(id), betweenMarks(tm), xx);
            break;
        }
    }

    return rc;
}

std::optional<std::tuple<int32_t, int32_t>> ATParser::evalueate2ComaValues()
{
    int32_t a = 0, b = 0;
    std::optional<std::tuple<int32_t, int32_t>> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);

            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);

            auto count = std::sscanf(kv.second.c_str(), "%d,%d",
                                     &a, &b);

            if (count != 2)
                continue;
            rc = std::make_tuple(a, b);
            break;
        }

    } while (false);
    return rc;
}

std::optional<std::tuple<std::string, int32_t, int32_t>> ATParser::evalueateTextAndTwoNumber()
{
    int32_t a = 0, b = 0;
    std::string info;
    std::optional<std::tuple<std::string, int32_t, int32_t>> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {

            auto kv = getKeyValue(x);

            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);

            info.resize(kv.second.length() + 1);
            auto count = std::sscanf(kv.second.c_str(), "%[^,],%d,%d",
                                     &info[0], &a, &b);
            if (count != 3)
                continue;
            rc = std::make_tuple(betweenMarks(info), a, b);
            break;
        }

    } while (false);
    return rc;
}

std::optional<std::tuple<std::string, int32_t>> ATParser::evalueateTextAndNumber()
{
    int32_t a = 0;
    std::string info;
    std::optional<std::tuple<std::string, int32_t>> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);

            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);
            info.resize(kv.second.length() + 1);
            auto count = std::sscanf(kv.second.c_str(), "%[^,],%d",
                                     &info[0], &a);
            if (count != 2)
                continue;
            rc = std::make_tuple(betweenMarks(info), a);
            break;
        }

    } while (false);
    return rc;
}

std::optional<std::string> ATParser::evalueateText()
{
    int32_t a = 0;
    std::optional<std::string> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);
            auto str = betweenMarks(kv.second);
            if (str.empty())
                continue;
            rc = str;
            break;
        }

    } while (false);
    return rc;
}

std::optional<std::tuple<int32_t, int32_t, std::string>> ATParser::evalueate2ComaValuesString()
{
    int32_t a = 0, b = 0;
    std::string str;
    std::optional<std::tuple<int32_t, int32_t, std::string>> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);

            str.resize(kv.second.size() + 1);

            auto count = std::sscanf(kv.second.c_str(), "%d,%d,%s",
                                     &a, &b, &str[0]);
            if (count != 3)
                continue;

            rc = std::make_tuple(a, b, betweenMarks(str));
            break;
        }

    } while (false);
    return rc;
}

std::optional<std::tuple<std::string, int32_t, std::string>> ATParser::evalueateTextNumberTxt()
{
    int32_t a = 0, b = 0;
    std::string str1;
    std::string str2;
    std::optional<std::tuple<std::string, int32_t, std::string>> rc = std::nullopt;
    do
    {
        if (_answer.empty())
            break;

        for (auto x : _answer)
        {
            auto kv = getKeyValue(x);
            if (kv.first.empty() || kv.second.empty())
                continue;
            eatWhiteSpaces(kv.second);

            str1.resize(kv.second.size() + 1);
            str2.resize(kv.second.size() + 1);

            auto count = std::sscanf(kv.second.c_str(), "%[^,],%d,%s",
                                     &str1[0], &b, &str2[0]);

            if (count != 3)
                continue;

            rc = std::make_tuple(betweenMarks(str1), b, betweenMarks(str2));
            break;
        }

    } while (false);
    return rc;
}

bool ATParser::isStatusExists(ResponseStatus status) const
{
    bool rc = false;
    do
    {

        auto part = std::find_if(ATStatus.begin(), ATStatus.end(), [status](const auto &item)
                                 { return (status == item.second); });

        if (part == ATStatus.end())
            break;

        for (auto a : _answer)
        {
            if (containString(a, part->first))
            {
                rc = true;
                break;
            }
        }
    } while (false);
    return rc;
}

std::optional<datetime_t> ATParser::evalueateDateTime()
{
    std::optional<datetime_t> rc = std::nullopt;
    bool minussgn = false;
    datetime_t r;
    uint32_t day = 0, month = 0, year = 0, hour = 0, min = 0, sec = 0, zone = 0;

    memset(&r, 0, sizeof(datetime_t));

    do
    {
        if (_answer.empty())
            break;
        std::string datestr;
        for (auto a : _answer)
        {
            datestr = betweenMarks(a);
            if (!datestr.empty())
                break;
        }

        if (datestr.empty())
            break;

        if (datestr.size() < 20)
            break;

        rc = breakTime(datestr);

    } while (false);
    return rc;
}

std::optional<datetime_t> ATParser::breakTime(std::string_view datestr)
{
    std::optional<datetime_t> rc = std::nullopt;
    bool minussgn = false;
    datetime_t r;
    int count = 0;
    uint32_t day = 0, month = 0, year = 0, hour = 0, min = 0, sec = 0, zone = 0;

    do
    {
        // yy/MM/dd,hh:mm:ss±zz   like "23/03/04,08:22:23+04"
        auto count = std::sscanf(datestr.data(), "%u/%u/%u,%u:%u:%u+%u",
                                 &year, &month, &day, &hour, &min, &sec, &zone);
        if (count != 7)
        {
            minussgn = true;
            count = std::sscanf(datestr.data(), "%u/%u/%u,%u:%u:%u-%u",
                                &year, &month, &day, &hour, &min, &sec, &zone);
        }
        if (count != 7)
            break;

        auto unxtm = TimeUtils::makeUnixTime(2000 + year, month, day, hour, min, sec);

        if (!minussgn)
            unxtm -= zone * 15 * 60;
        else
            unxtm += zone * 15 * 60;

        TimeUtils::breakUnixTime(unxtm, r);
        rc = r;
    } while (false);

    return rc;
}

} //namespace gsm 