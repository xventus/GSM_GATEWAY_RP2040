//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm_commands.h
/// @author Petr Vanek

#pragma once

#include <string_view>

using namespace std::literals;

namespace gsm {

namespace gsm_cmd
{
std::string_view C_AT{"AT"};
std::string_view C_ATE0{"ATE0"};
std::string_view C_ATE1{"ATE1"};
std::string_view C_ATCFUN0{"AT+CFUN=0"};
std::string_view C_ATCFUN1{"AT+CFUN=1"};
std::string_view C_ATCCLK{"AT+CCLK?"};
std::string_view C_ATCCLKON{"AT+CLTS=1"};
std::string_view C_ATCCLKOFF{"AT+CLTS=0"};
std::string_view C_ATW{"AT&W"};
std::string_view C_ATCREG{"AT+CREG?"};
std::string_view C_ATCSQ{"AT+CSQ"};
std::string_view C_ATCPIN{"AT+CPIN?"};
std::string_view C_ATCMGFON{"AT+CMGF=1"};
std::string_view C_ATCMGS{"AT+CMGS"};
std::string_view C_ATCCID{"AT+CCID"};
std::string_view C_ATCOPS{"AT+COPS?"};
std::string_view C_ATA{"ATA"};
std::string_view C_ATH{"ATH"};
std::string_view C_ATD{"ATD+"};
std::string_view C_CLIPON{"AT+CLIP=1"};
std::string_view C_CLIPOFF{"AT+CLIP=0"};
std::string_view C_CMGD000{"AT+CMGD="};
std::string_view C_CMGR000{"AT+CMGR="};
std::string_view C_CSDHON{"AT+CSDH=1"};
std::string_view C_CATPMS{"AT+CPMS?"};
std::string_view C_CPMS{"CPMS"};
std::string_view C_CMGDAALL{"AT+CMGDA=\"DEL ALL\""};
std::string_view C_CMGR{"CMGR"};
std::string_view C_CGNSINFA{"CGNSINF"};
std::string_view C_READY{"READY"};
std::string_view CTRLZCR{"\x1a\r\n"};
std::string_view C_CGNSPWRON{"AT+CGNSPWR=1"};
std::string_view C_CGNSPWROFF{"AT+CGNSPWR=0"};
std::string_view C_CCGNSINF{"AT+CGNSINF"};

}

} //namespace gsm 