//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   flash_storage.h
/// @author Petr Vanek

#pragma once

#include <string>
#include <string_view>
#include <map>
#include <inttypes.h>
#include <iomanip>
#include <cstring>
#include <functional>
#include <hardware/flash.h>
#include <pico/stdlib.h>
#include <hardware/sync.h>

using namespace std::literals;
/*
Example:

    FlashStorage fs;
    fs.memedump();
    if (!fs.fetch()) {
        fs.addItem("item1","data1");
        fs.addItem("item2","data2");
        fs.commit();
    }
    fs.memedump();

    fs.evaluateAllItem([](std::string_view key, std::string_view value) -> bool {
        printf("key=%s value=%s\n", key.data(), value.data());
        return true;
    });

    auto d = fs.getValue("item1");
    printf("%s\n", d.c_str());
    
*/

class FlashStorage
{

public:
    /**
     * @brief clear items
     *
     */
    void clear()
    {
        _items.clear();
    }

    /**
     * @brief add one item into storage
     * illegal characters are: "±" and "`"
     * escaping is not performed!!!
     *
     * @param key
     * @param value
     */
    bool addItem(std::string_view key, std::string_view value)
    {
        bool rc = false;
        do
        {
            // invlaid character
            if (key.find(_equ) != std::string::npos)
                break;
            if (key.find(_delim) != std::string::npos)
                break;

            _items[std::string(key)] = std::string(value);
        } while (false);
        return rc;
    }

    /**
     * @brief Get the Value object
     *
     * @param key
     * @return std::string
     */
    std::string getValue(std::string_view key)
    {
        std::string rc;
        auto it = _items.find(std::string(key));
        if (it != _items.end())
        {
            rc = it->second;
        }
        return rc;
    }

    /**
     * @brief save data to flash memory
     * TODO: verify after write
     */
    bool commit(bool disableInts = true)
    {
        bool rc = true;
        std::string tmp;
        tmp = _magicB;
        tmp += mapToString();

        // limited to one block
        tmp.resize(_block, '\0');
        if (disableInts) {
            auto ints = save_and_disable_interrupts();
            flash_range_erase(_address, _block);
            flash_range_program(_address, (const uint8_t *)tmp.c_str(), _block);
            restore_interrupts(ints);
        } else {
            flash_range_erase(_address, _block);
            flash_range_program(_address, (const uint8_t *)tmp.c_str(), _block);
        }

        return rc;
    }

    /**
     * @brief reads data from flash memory
     *
     * @return true - data exists and reads
     * @return false - storage not found
     */
    bool fetch()
    {
        std::string toparse;
        bool rc = false;

        do
        {
            if (_magicB.length() > _block)
                break;
            if (std::memcmp((const void *)(XIP_BASE + _address), _magicB.data(), _magicB.length()) != 0)
                break;
            const char *end = static_cast<const char *>(memchr((const void *)(XIP_BASE + _address), '\0', _block));
            std::size_t length = (std::size_t)end - (XIP_BASE + _address);
            toparse.assign((const char *)(XIP_BASE + _address + _magicB.length()), length);
            _items.clear();
            stringToMap(toparse);
            rc = true;

        } while (false);
        return rc;
    }

    /**
     * @brief memory dump to serialized storage
     *
     * @param len -  how many bytes to output
     */
    void memedump(std::size_t len = 128)
    {
        std::stringstream ss;
        char *p = (char *)XIP_BASE + _address;
        ss << HexDumper<const std::vector<uint8_t>>({p, p + len}, 16);
        DebugUtils::debugMsg("%s\n", ss.str().c_str());
    }

     /**
     * @brief how many positions does the map contain
     *
     * @return std::size_t
     */
    std::size_t numberOfItems()
    {
        return _items.size();
    }

    /**
     * @brief traversing through all positions in the map and returns them
     *
     * @param callback
     */
    void evaluateAllItem(std::function<bool(std::string_view key, std::string_view value)> callback) const
    {
        for (auto it : _items)
        {
            if (!callback(it.first, it.second))
                break;
        }
    }

    /**
     * @brief corrupt the memory area and mark it as unused. 
     * 
     * @param disableInts 
     */
    void unusedCommit(bool disableInts = true)
    {
        std::string tmp;
        tmp = _magicUnused;

        // limited to one block
        tmp.resize(_block, '\0');
        if (disableInts) {
            auto ints = save_and_disable_interrupts();
            flash_range_erase(_address, _block);
            flash_range_program(_address, (const uint8_t *)tmp.c_str(), _block);
            restore_interrupts(ints);
        } else {
            flash_range_erase(_address, _block);
            flash_range_program(_address, (const uint8_t *)tmp.c_str(), _block);
        }
    }

private:
    /**
     * @brief convert map to string, serialization
     *
     * @return std::string - string without magic
     */
    std::string mapToString()
    {
        std::ostringstream oss;
        for (const auto &[key, value] : _items)
        {
            oss << key << _equ << value << _delim;
        }
        std::string result = oss.str();
        return result;
    }

    /**
     * @brief converts serialized content into a map of positions
     *
     * @param input - string without magic
     */
    void stringToMap(const std::string &input)
    {
        std::istringstream iss(input);
        std::string token;
        while (std::getline(iss, token, _del))
        {
            auto pos = token.find(_equ);
            if (pos != std::string::npos)
            {
                std::string key = token.substr(0, pos);
                auto value = token.substr(pos + 1);
                _items[key] = value;
            }
        }
    }

   

private:
    static constexpr std::size_t _block{4096};                              ///<  memory block size - minimum see at PICO SDK
    static constexpr std::size_t _address{PICO_FLASH_SIZE_BYTES - _block};  ///<  start address in the flash memory for 2M version
    static constexpr char _del{'~'};                                        ///<  delimiter between item
    static constexpr std::string_view _delim{"~"};                          ///<  delimiter between item
    static constexpr std::string_view _equ{"^"};                            ///<  delimiter betweeb key & value
    static constexpr std::string_view _magicB{"$$FLSHSTORAGE@@@"sv};        ///<  a mark in memory by which it is known that data has been written
    std::map<std::string, std::string> _items;                              ///<  the map with all items
    static constexpr std::string_view _magicUnused{"===DIRTY==="sv};        ///<  unused memory marker - dirty flag
};
