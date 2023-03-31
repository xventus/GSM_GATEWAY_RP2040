//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   time_base.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "pico/time.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

/**
 * @brief - encapsulates the basic real-time control clock in the rp2040
 *        - Basically it works with 0 - 24 hour cycle.
 */
class TimeBase
{

public:

    explicit TimeBase();
    bool init();
    bool updateTime(datetime_t &dt);
    datetime_t getTimeDate();
   
    /**
     * @brief reads year
     *
     * @return int16_t - 0-99
     */
    int16_t year() const;

    /**
     * @brief reads month
     *
     * @return int8_t  1 - 12
     */
    int8_t month() const;

    /**
     * @brief reads day
     *
     * @return int8_t  1 - 31
     */
    int8_t day() const;

    /**
     * @brief reads hour
     *
     * @return int8_t  0 - 23 / 0 - 12
     */
    int8_t hour() const;

    /**
     * @brief reads minutes
     *
     * @return int8_t  0 - 59
     */
    int8_t minute() const;

    /**
     * @brief reads seconds
     *
     * @return int8_t  0 - 59
     */
    int8_t second() const;

    /**
     * @brief reads day of week
     *
     * @return int8_t 0 - 6
     */
    int8_t weekDay()  const;

    /**
     * @brief Set the Year
     *
     * @param val 0-99 or 2000-2099
     */
    bool setYear(int16_t val);

    /**
     * @brief Set the Month
     *
     * @param val
     */
    bool setMonth(int8_t val);

    /**
     * @brief Set the Day
     *
     * @param val
     */
    bool setDay(int8_t val);

    /**
     * @brief Set the Hour
     *
     * @param val
     */
    bool setHour(int8_t val);

    /**
     * @brief Set the Minute
     *
     * @param val
     */
    bool setMinute(int8_t val);

    /**
     * @brief Set the Second
     *
     * @param val
     */
    bool setSecond(int8_t val);

    /**
     * @brief Set the Week Day
     *
     * @param val  0 - 6
     */
    bool setWeekDay(int8_t val);

    /**
     * @brief question whether the time has been updated
     * 
     * @return true 
     * @return false 
     */
    bool isValid() const;

   
private:
   
   
};
