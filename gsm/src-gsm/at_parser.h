//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   at_parser.h
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
#include <optional>
#include "pico/util/datetime.h"
#include "at_enums.h"

namespace gsm {

class ATParser
{

public:
    /**
     * @brief initialize parser
     *
     * @param startAt - determines what is parsed
     * ParseCode::aBegin - answer
     * ParseCode::qBegin - query + answer
     *
     */
    void init(ParseCode startAt = ParseCode::aBegin);

    /**
     * @brief parse byte/char from stream
     *
     * @param inp - one char from serial line
     * @return true - parsed - finish
     * @return false - not finished
     */
    bool parse(char inp);

    /**
     * @brief parse string, line by line
     *
     * @param inp - string with CRLF termination
     * @return true - parsed - finish
     * @return false - not finished
     */
    bool parse(std::string_view inp);

    /**
     * @brief Gets query string, only if init(ParseCode::aBegin)
     *
     * @return std::string
     */
    std::string getQuery() const
    {
        return _query;
    }

    /**
     * @brief Gets query  param, only if init(ParseCode::aBegin)
     *
     * @return std::string
     */
    std::string getQueryParam() const
    {
        return _queryParam;
    }

    /**
     * @brief Get the Query Type
     *
     * @return QueryType
     */
    QueryType getQueryType() const
    {
        return _queryType;
    }

    /**
     * @brief Gets the Response Status
     *
     * @return ResponseStatus
     */
    ResponseStatus getResponseStatus() const
    {
        return _status;
    }

    /**
     * @brief Gets vector of response lines
     *
     * @return Responses
     */
    Responses getResponses() const
    {
        return _answer;
    }

    /**
     * @brief parses the response until the appropriate response status is found
     *
     * @param pref
     * @return ATParser&
     */
    ATParser &withPrefferedTag(ResponseStatus pref)
    {
        _prefferdResponse = pref;
        return *this;
    }

    /**
     * @brief see if there is a proper status in the answers
     *
     * @param status - required status
     * @return true - exists
     * @return false
     */
    bool isStatusExists(ResponseStatus status) const;

    /**
     * @brief searches the answers for a chain in the format: "string",number1,number2
     *
     * @return std::optional<std::tuple<std::string, int32_t, int32_t>>
     */
    std::optional<std::tuple<std::string, int32_t, int32_t>> evalueateTextAndTwoNumber();

    /**
     * @brief searches the answers for a chain in the format: "string",number1
     *
     * @return std::optional<std::tuple<std::string, int32_t>>
     */
    std::optional<std::tuple<std::string, int32_t>> evalueateTextAndNumber();

    /**
     * @brief searches the answers for a chain in the format: "23/03/04,08:22:23+04"
     *
     * @return std::optional<datetime_t>
     */
    std::optional<datetime_t> evalueateDateTime();

    /**
     * @brief searches the answers for a chain in the format: number1,number2
     *
     * @return std::optional<std::tuple<int32_t, int32_t>>
     */
    std::optional<std::tuple<int32_t, int32_t>> evalueate2ComaValues();

    /**
     * @brief check if input string exists as value, format: key:value
     *
     * @param input
     * @return std::optional<bool>
     */
    std::optional<bool> compareResponseText(std::string_view input);

    /**
     * @brief Get the Text value from responses, format: key:value
     *
     * @param ignore - ignore text
     * @return std::optional<std::string>
     */
    std::optional<std::string> getText(std::string_view ignore = ""sv);

    /**
     * @brief searches the answers for a chain in the format: number1,number2,string
     *
     * @return std::optional<std::tuple<int32_t, int32_t, std::string>>
     */
    std::optional<std::tuple<int32_t, int32_t, std::string>> evalueate2ComaValuesString();

    /**
     * @brief searches the answers for a chain in the format: string1,number,string2
     *
     * @return std::optional<std::tuple<std::string, int32_t, std::string>>
     */
    std::optional<std::tuple<std::string, int32_t, std::string>> evalueateTextNumberTxt();

    /**
     * @brief searches the answers (value) for a chain in the format: key:"value"
     *
     * @return std::optional<std::string>
     */
    std::optional<std::string> evalueateText();

    /**
     * @brief the reply to the SMS message breaks up
     *
     * @param item SMS message mark
     * @return std::optional<std::tuple<std::string, std::string, std::string>>
     * callerID, timestamp as string, message
     */
    std::optional<std::tuple<std::string, std::string, std::string>> evaluateSMS(std::string_view item);

    /**
     * @brief replay of GNSS time / position
     *
     * @param item GNSS message mark
     * @return std::optional<std::tuple<datetime_t, bool, bool>>
     * time stamp, fix, status
     */
    std::optional<std::tuple<datetime_t, bool, bool>> evaluateGNSSTime(std::string_view item);

    /**
     * @brief break time string into datetime stucture
     *
     * @param datestr format: yy/MM/dd,hh:mm:ss±zz   like "23/03/04,08:22:23+04
     * @return std::optional<datetime_t>
     */
    std::optional<datetime_t> breakTime(std::string_view datestr);

    /**
     * @brief remove all white spaces
     *
     * @param input
     */
    void eatWhiteSpaces(std::string &input);

    /**
     * @brief split string into key and value. As delimiter used ':'
     *
     * @param input - string
     * @return KeyValue
     */
    KeyValue getKeyValue(std::string_view input);

private:
    static constexpr uint32_t _maxLinesResponse{10};

    /**
     * @brief mark as invalid contnet and reinit parser
     *
     */
    void invalidContnet();

    /**
     * @brief parse query
     *
     * @param inp
     */
    void parseQueryContent(char inp);

    /**
     * @brief parse answer content
     *
     * @param inp
     */
    void parseAnswerContent(char inp);

    /**
     * @brief parse begin answer
     *
     * @param inp
     */
    void parseAnswerBegin(char inp);

    /**
     * @brief chcek if string ended with suffix
     *
     * @param str string
     * @param suffix
     * @return true
     * @return false
     */
    bool endsWith(std::string_view str, std::string_view suffix) const;

    /**
     * @brief check ifstring starts with prefix
     *
     * @param str string
     * @param prefix
     * @return true
     * @return false
     */
    bool startsWith(std::string_view str, std::string_view prefix) const;

    /**
     * @brief check if string contains prefix
     *
     * @param str string
     * @param prefix
     * @return true
     * @return false
     */
    bool containString(std::string_view str, std::string_view prefix) const;

    /**
     * @brief gets streing between marks
     *
     * @param strValue string
     * @param mark - mark
     * @return std::string
     */
    std::string betweenMarks(std::string_view strValue, std::string_view mark = "\"");

    /**
     * @brief exceptions in parsing, such as the beginning of an SMS message
     *
     */
    void specialCase();

    std::string _query;                                   ///< query contnet
    std::string _queryParam;                              ///< query params
    Responses _answer;                                    ///< vector of answers
    std::string _buffer;                                  ///< parser buffer
    QueryType _queryType{QueryType::unknown};             ///< query type
    ParseCode _flow{ParseCode::qBegin};                   ///< processing sentence type
    ResponseStatus _status{ResponseStatus::unknown};      ///< response status
    ResponseStatus _prefferdResponse{ResponseStatus::ok}; ///< preffered response status
};

} //namespace gsm 