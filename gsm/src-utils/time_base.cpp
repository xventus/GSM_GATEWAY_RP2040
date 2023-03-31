
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   time_base.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "time_base.h"

TimeBase::TimeBase()
{
}

bool TimeBase::init()
{
    rtc_init();

    // make valid time base to 1.1.0001 00:00:00 Saturday
    datetime_t t = {
        .year = 1,
        .month = 1,
        .day = 1,
        .dotw = 6,
        .hour = 0,
        .min = 0,
        .sec = 0};
    auto rc = rtc_set_datetime(&t);
    if (rc)
        rc = rtc_running();
    
    if (rc) rtc_disable_alarm();

    return rc;
}

bool TimeBase::isValid() const {
    bool rc = false;
    if (year() > 2000) rc = true; 
    return rc;
}

bool TimeBase::updateTime(datetime_t &dt) {
    return rtc_set_datetime(&dt);
}

datetime_t TimeBase::getTimeDate() {
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    if (!r) memset(&t, 0, sizeof(datetime_t));
    return t;
}


int16_t TimeBase::year() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.year : -1);
}

int8_t TimeBase::month() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.month : -1);
}

int8_t TimeBase::day() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.day : -1);
}

int8_t TimeBase::hour() const
{
   datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.hour : -1);
}

int8_t TimeBase::minute() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.min : -1);
}

int8_t TimeBase::second() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.sec : -1);
}

int8_t TimeBase::weekDay() const
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);	
    return (r ? t.dotw : -1);
}

bool TimeBase::setYear(int16_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.year = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setMonth(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.month = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setDay(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.day = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setWeekDay(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.dotw = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setHour(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.hour = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setMinute(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.min = val;
    if (r) rtc_set_datetime(&t);
    return r;
}

bool TimeBase::setSecond(int8_t val)
{
    datetime_t t;
    auto r = rtc_get_datetime(&t);
    t.sec = val;
    if (r) rtc_set_datetime(&t);
    return r;
}


