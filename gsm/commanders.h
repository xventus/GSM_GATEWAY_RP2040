//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   commanders.h
/// @author Petr Vanek

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <string_view>

/**
 * @brief It encapsulates
 * the telephone operators who can control the SMS gateway.
 * Adding them, deleting the database, etc.
 *
 */
class Commanders
{

public:
    /**
     * @brief reads all commanders phone numbers
     *
     */
    void refresh();

    /**
     * @brief check if phone number is a commander
     *
     * @param id phone number
     * @return true
     * @return false
     */
    bool isExist(std::string_view id) const;

    /**
     * @brief is the supreme commander?
     *
     * @param id
     * @return true
     * @return false
     */
    bool isSupremeCommander(std::string_view id) const;

    /**
     * @brief adds new commander
     *
     * @param id phone number
     * @return true - if successfully added
     * @return false
     */
    bool addNew(std::string_view id);

    /**
     * @brief commander list is empty
     *
     * @return true
     * @return false
     */
    bool isEmpty();

    /**
     * @brief is commnader list is full
     *
     * @return true
     * @return false
     */
    bool isFull();

    /**
     * @brief Get the List of all commanders
     *
     * @return std::string
     */
    std::string getList() const;

private:
    std::vector<std::string> _commanders;
    const uint8_t _maxCommanders = 5;
};