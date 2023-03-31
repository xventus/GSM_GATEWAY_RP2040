//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   time_base.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include <sstream>
#include <iomanip>
#include "pico/time.h"
#include "pico/util/datetime.h"
#include "time.h"

/**
 * @brief - Auxiliary routines for working with time
 */
class TimeUtils
{
public:
    /**
     * @brief a structure describing the time shift for determining the change of daylight saving time and standard time
     *
     */
    struct TimePoint
    {
        uint8_t _week;  // 1st. 2nd ... week in month or 0 for last week
        uint8_t _month; // 1..12, 1 is January
        uint8_t _hour;  // hour of change 0..23
        uint8_t _dow;   // day of week change, 0..6, 0 is Sunday
    };

    static constexpr uint8_t _dayOf[]{0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    static constexpr uint8_t _monthDays[]{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    static constexpr uint32_t _secPerMin{60};
    static constexpr uint32_t _secPerHour{_secPerMin * 60};
    static constexpr uint32_t _secPerDay{_secPerHour * 24};

    /*
        Predefined constants for changing the daylight saving interval

        summer time CEST, UTC+02:00
        from  1:00 UTC last Sunday in March ->
        to 1:00 UTC  last Sunday in October ->
        else (CET, UTC+01:00)
    */
    static constexpr TimePoint CESTFrom{
        0, // 0 - last week
        3, // March
        1, // 1::00 UTC
        0  // Sunday
    };

    static constexpr TimePoint CESTTo{
        0,  // 0 - last week
        10, // October
        1,  // 1::00 UTC
        0   // Sunday
    };

    // Offset constants for summer and winter time
    static constexpr uint32_t CETOffset{_secPerHour};
    static constexpr uint32_t CESTOffset{2 * _secPerHour};

    /**
     * @brief calculate local time from UTC
     * 
     * @param utcTime UTC time 
     * @param DTSFrom DST from time - calculate for each year
     * @param DTSTo DST to time - calculate for each year 
     * @param stdTime - standard time
     * @param dstTime - dayl light saving time
     * @return uint32_t - local unix time
     */
    inline static uint32_t localTime(uint32_t utcTime,
                                     uint32_t DTSFrom,
                                     uint32_t DTSTo,
                                     uint32_t stdTime = CETOffset,
                                     uint32_t dstTime = CESTOffset)
    {

        if (utcTime >= DTSFrom && utcTime <= DTSTo)
        {
            return utcTime + dstTime;
        }

        return utcTime + stdTime;
    }

    /**
     * @brief calculate the time stamp (unixtimsubleme
     * e) for the specified year defined by TimePoint
     *
     * @param r - time shift structure
     * @param year - calculate for defined year
     * @return uint32_t  - unixtime
     */
    static uint32_t timeShift(const TimePoint &r, int year)
    {
        uint8_t m = r._month;
        uint8_t w = r._week;
        if (w == 0)
        {
            if (++m > 12)
            {
                m = 1;
                year++;
            }
            w = 1;
        }

        auto t = makeUnixTime(year, m, 1, r._hour, 0, 0);
        t += ((r._dow - dayOfWeek(year, m, 1) + 7) % 7 + (w - 1) * 7) * _secPerDay;
        if (r._week == 0)
            t -= 7 * _secPerDay;
        return t;
    }

    /**
     * @brief calculates the day of the week from the specified date
     *
     * @param year
     * @param month
     * @param day
     * @return uint8_t Day of the week. 0 is Sunday,  0 - 6
     */
    static uint8_t dayOfWeek(int16_t year, int8_t month, int8_t day)
    {
        // Note: https://www.tondering.dk/claus/cal/chrweek.php#calcdow

        year -= month < 3;
        return (year + year / 4 - year / 100 + year / 400 + _dayOf[month - 1] + day) % 7;
    }

    /**
     * @brief from the structure fields such as year, month and day calculate the day of the week and put it into the structure
     *
     * @param tm - datetime_t structure
     */
    static void updateDayOfWeek(datetime_t &tm)
    {
        tm.dotw = dayOfWeek(tm.year, tm.month, tm.day);
    }

    /**
     * @brief Central European Time (CET)
     * at 01:00 Coordinated Universal Time (UTC), which means that it changes at 02:00 Central European Time (CET)
     * at 03:00 UTC disappears, means changes at 02:00
     * time ignore for now
     *
     * @param day
     * @param month
     * @param dow
     * @return true
     * @return false
     */
    static bool isDst(int8_t day, int8_t month, int8_t dow)
    {
        if (month < 3 || month > 10)
            return false;
        if (month > 3 && month < 10)
            return true;

        auto prevSun = day - dow;

        if (month == 3)
            return prevSun >= 25;
        if (month == 10)
            return prevSun < 25;

        return false; // this line never gonna happend
    }

    /**
     * @brief calculate unixtime since 1970 number of seconds from datetime_t
     *
     * @param tm
     * @return uint32_t
     */
    static uint32_t makeUnixTime(const datetime_t &tm)
    {
        return TimeUtils::makeUnixTime((int16_t)tm.year, (int8_t)tm.month, (int8_t)tm.day, (int8_t)tm.hour, (int8_t)tm.min, (int8_t)tm.sec);
    }

    /**
     * @brief calculate unixtime since 1970 number of seconds
     *
     * @param year - value of the year
     * @param mon  - value of the month
     * @param day - value of the day
     * @param hour - value of the hour
     * @param min - value of the minute
     * @param sec - value of the second
     * @return uint32_t
     */
    static uint32_t makeUnixTime(
        uint32_t year,
        uint32_t mon,
        uint32_t day,
        uint32_t hour,
        uint32_t min,
        uint32_t sec)
    {

        if (year < 1970)
            return 0;

        // 1..12 -> 11,12,1..10
        if (0 >= (int32_t)(mon -= 2))
        {
            mon += 12;
            year -= 1;
        }

        return ((((
                      (uint32_t)(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) + year * 365 - 719499) *
                      24 +
                  hour) *
                     60 +
                 min) *
                    60 +
                sec);
    }

    /**
     * @brief test if the year is a leap year
     *
     * @param year - year value
     * @return true  - leap year
     * @return false
     */
    inline static bool isLeapYear(int year)
    {
        if (year % 400 == 0)
        {
            return true;
        }

        if (year % 100 == 0)
        {
            return false;
        }

        if (year % 4 == 0)
        {
            return true;
        }

        return false;
    }

    /**
     * @brief Decompose the unixtime into individual positions of the structure
     *
     * @param intime - input unixtime
     * @param tm [out] - updated time structure
     */
    static void breakUnixTime(uint32_t intime, datetime_t &tm)
    {

        uint16_t year;
        uint8_t month, monthLength;
        uint32_t time;
        unsigned long days;

        time = (uint32_t)intime;
        tm.sec = time % 60;
        time /= 60;
        tm.min = time % 60;
        time /= 60;
        tm.hour = time % 24;
        time /= 24;
        tm.dotw = ((time + 4) % 7); // 0 is Sunday

        year = 1970;
        days = 0;
        while ((unsigned)(days += (isLeapYear(year) ? 366 : 365)) <= time)
        {
            year++;
        }
        tm.year = year; // year from 1970

        days -= isLeapYear(year) ? 366 : 365;
        time -= days;
        days = 0;
        month = 0;
        monthLength = 0;
        for (month = 0; month < 12; month++)
        {
            if (month == 1)
            { // february
                if (isLeapYear(year))
                {
                    monthLength = 29;
                }
                else
                {
                    monthLength = 28;
                }
            }
            else
            {
                monthLength = _monthDays[month];
            }

            if (time >= monthLength)
            {
                time -= monthLength;
            }
            else
            {
                break;
            }
        }

        // 1..12, 1 is January
        tm.month = month + 1;

        // 1..28,29,30,31 depending on month
        tm.day = time + 1;
    }

    /**
     * @brief converts a time stamp to an abbreviated short time string
     * 
     * @param dt  - timestamp
     * @return std::string - short time string
     */
    static std::string timeToStringShort(const datetime_t &dt) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << (uint32_t) dt.hour << ':'
            << std::setfill('0') << std::setw(2) << (uint32_t) dt.min;
        return oss.str();
    }


    /**
     * @brief converts a time stamp to an abbreviated time string
     * 
     * @param dt  - timestamp
     * @return std::string - long time string
     */
    static std::string timeToString(const datetime_t &dt) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << (uint32_t) dt.hour << ':'
            << std::setfill('0') << std::setw(2) << (uint32_t) dt.min << ':'
            << std::setfill('0') << std::setw(2) << (uint32_t) dt.sec;
        return oss.str();
    }


    /**
     * @brief converts a time stamp to an abbreviated date string
     * 
     * @param dt  - timestamp
     * @return std::string - date string
     */
    static std::string dateToString(const datetime_t &dt) {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << (uint32_t) dt.day << '/'
            << std::setfill('0') << std::setw(2) << (uint32_t) dt.month << '/'
            << std::setfill('0') << std::setw(2) << (uint32_t) dt.year;
        return oss.str();
    }

    /**
     * @brief simple chceck if structure if correct
     * 
     * @param dt 
     * @return true - valid timestamp
     * @return false - failed
     */
    static bool isTimestampCorrect(const datetime_t &dt) {
        bool rc = false;
        do {
            if (dt.day == 0) break;
            if (dt.month == 0) break;
            if (dt.year < 2000) break;
            if (dt.hour > 23) break;
            if (dt.min > 59) break;
            if (dt.sec > 59) break;
            rc = true;
        } while(false);
        
        return rc;
    }

    
};