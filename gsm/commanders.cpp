//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   commanders.cpp
/// @author Petr Vanek

#include "commanders.h"
#include "src-utils/debug_utils.h"
#include "src-utils/flash_storage.h"

void Commanders::refresh()
{
    _commanders.clear();
    FlashStorage fs;

    if (fs.fetch())
    {
        fs.evaluateAllItem([this](std::string_view key, std::string_view value) -> bool {
        _commanders.emplace_back(std::string(value));
        return true; });
    }
}

bool Commanders::isExist(std::string_view id) const
{
    auto result = std::find_if(_commanders.begin(), _commanders.end(), [&id](auto val)  { 
            return (val.compare(id)==0); });
    return (result != _commanders.end());
}

bool Commanders::isSupremeCommander(std::string_view id) const
{
    bool rc = false;
    do
    {
        if (_commanders.empty())
            break;
        if (_commanders.at(0).find(id) == std::string::npos)
            break;
        rc = true;
    } while (false);
    return rc;
}

bool Commanders::addNew(std::string_view id)
{
    bool rc = false;
    int counter = 0;
    // always rewrite whole memory

    do
    {
        if ((_commanders.size() + 1) > _maxCommanders)
            break;

        auto it = std::find(_commanders.begin(), _commanders.end(), id);
        if (it == _commanders.end())
        {
            // not found - add new
            _commanders.emplace_back(std::string(id));

            FlashStorage fs;
            for (auto &element : _commanders)
            {
                counter++;
                std::string phnx;
                phnx = "phone";
                phnx += std::to_string(counter);
                fs.addItem(phnx, element);
            }

            rc = fs.commit();
        } else {
            //already registered - OK
            rc = true;
        }
    } while (false);
    return rc;
}

bool Commanders::isEmpty()
{
    return _commanders.empty();
}

bool Commanders::isFull()
{
    // maximum of commanders
    return (_commanders.size() > _maxCommanders);
}

std::string Commanders::getList() const
{
    std::string rc;
    int counter = 0;

    for (auto &element : _commanders)
    {
        counter++;
        rc = std::to_string(counter);
        rc += ": ";
        rc += element;
        rc += "\n\r";
    }

    return rc;
}