//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include <string>
#include <string_view>
#include "pico/util/datetime.h"
#include "at_parser.h"
#include "serial_modem_intf.h"

namespace gsm {


/**
 * @brief gsm module info callback
 * 
 */
enum class GsmInfoState
    {
        hwinit,
        wait,
        ping,
      //  modeminit,
        gnss,
        rx,
        tx
    };

typedef std::function<void (GsmInfoState state)> GSMInfoCallback;



/**
 * @brief Encapsulates the control of GSM modems such as SIM868, SIM800
 * 
 */
class GSM
{

public:
    explicit GSM(ISerialModem &s) : _serial(&s)
    {
    }

    /**
     * @brief tries to initialize the modem, 
     * or switch it on if it has been switched off and establish reliable communication. 
     * 
     * @param withGnss  - GPS init required
     * @return true 
     * @return false 
     */
    bool init(bool withGnss = false);

    /**
     * @brief check if modem is online
     * 
     * @return true - it works
     * @return false 
     */
    bool ping();

    /**
     * @brief switching on the echo modem
     * 
     * @return true - success
     * @return false 
     */
    bool echoOff();

    /**
     * @brief switching off the echo modem
     * 
     * @return true - success
     * @return false 
     */
    bool echoOn();

    /**
     * @brief disables the radio part of the modem and disconnects from the communication site
     * 
     * @return true - success
     * @return false 
     */
    bool flightMode();

    /**
     * @brief turns on the radio part of the modem. This operation will take some time.
     * 
     * @return true - success
     * @return false 
     */
    bool phoneMode();

    /**
     * @brief reads the time from the internal RTC, which is synchronised from the mobile site
     * 
     * @return std::optional<datetime_t> - if the value is valid, it is returned
     */
    std::optional<datetime_t> readRTC();

    /**
     * @brief Enable the Real Time ClocK and save configuration
     * required modem restart for the update state
     * 
     * @param enable true - ENABLE, false - DISABLE
     * @return true - success
     * @return false 
     */
    bool enableRTC(bool enable);

    /**
     * @brief Question if the radio part is registered using SIM to the mobile site
     * 
     * @return std::optional<uint8_t> - if exists, correct value is returned
     * OK is 1 or 5 for fully communication !
     * 
     *  0 - not registered
     *  1 - home registered
     *  2 - not registered searching
     *  3 - denied registered
     *  4 - unknown
     *  5- registered roaming
     *  6 - registered sms only  NB-IOT
     *  7 - registered sms only roaming NB-IOT
     */
    std::optional<uint8_t> isRegistered();

    /**
     * @briefinforms about the quality of the received signal. 
     * If communication with the modem has failed, no value is returned
     *
     * @return std::optional<uint8_t>
     * 0 113 dBm or less
     * 1 111 dBm
     * 31 51 dBm or greater
     * 2...30 109... 53 dBm
     * 99 not known or not detectable
     *
     * means:
     * 20-30 Excellent
     * 15-10 Good
     * 2-9 Marginal
     */
    std::optional<uint8_t> qualitySignal();

    /**
     * @brief It asks if you need a PIN to use the SIM card. 
     *  If communication with the modem has failed, no value is returned
     *
     * @return std::optional<bool> true - no pin required
     */
    std::optional<bool> pinNotRequired();

    /**
     * @brief  Integrated Circuit Card ID. It is a 19- or 20-digit number 
     * that's typically printed on the back of a SIM card. 
     * The ICCID is a globally unique serial number—a one-of-a-kind signature that identifies the SIM card itself.
     * 
     * Used to check the presence of a SIM card
     * 
     * @return std::optional<std::string>  if value exists returns ICCID
     */
    std::optional<std::string> getICCID();


    /**
     * @brief Get the Operator name
     * 
     * @return std::optional<std::string> - return string if success
     */
    std::optional<std::string> getOperator();


    /**
     * @brief send smsm to phone number
     * 
     * @param phoneNumber - phone numebr 
     * @param ascii7Text - ASCII 7bit 
     * @return true - success
     * @return false - failed
     */
    bool sendSMS(std::string_view phoneNumber, std::string_view ascii7Text);

    /**
     * @brief dials a phone call - ring. Then you need to call checkStatus to see if the line is busy.
     * 
     * @param phoneNumber - telephone number
     * @return true - success 
     * @return false  - failed
     */
    bool ring(std::string_view phoneNumber);
    
    /**
     * @brief terminate connection, hang up
     * 
     * @return true the operation went well
     * @return false failed
     */
    bool disconnect();

    /**
     * @brief answers the line for an incoming call, it is used after the incoming RING
     * 
     * @return true - success
     * @return false  - error
     */
    bool answer();

    /**
     * @brief checks the modem status, such as incoming call or sms
     * 
     * @param maxWait - optional parameter, defines the maximum time of state exploration
     * @return ResponseStatus -  ResponseStatus::unknown for any event
     */
    ResponseStatus checkStatus(uint32_t maxWait=_defaultCheckModem);

    /**
     * @brief Powr ON / OFF GNSS part if exists
     * 
     * @param up - true for ON / FALSE for OFF
     * @return true - success
     * @return false - failed
     */
    bool gnssPower(bool up);

    /**
     * @brief try to get the time from GPS
     * 
     * @return std::optional<std::tuple<datetime_t, int32_t>> 
     * 1.st. timestamp structure
     * 2.nd. fix status  on / off
     * 3.rd. run status on / off
     */
    std::optional<std::tuple<datetime_t, bool, bool>> gnssInfo();

    /**
     * @brief Delete defined SMS message defined by index
     * 
     * @param index - SMS index
     * @return true - success
     * @return false - failed
     */
    bool delSMS(uint32_t index);

    /**
     * @brief delete all SMS from the SIM storage
     * 
     * @return true - success
     * @return false - failed
     */
    bool delAllSMS() ;

    /**
     * @brief Returns the number of messages in the storage and the storage size
     * 
     * @return std::optional< std::tuple<uint32_t,uint32_t>>  
     *  1.st. = returns nember of SMS in the storage
     *  2.nd. = return total SMS storage capacity
     */
    std::optional< std::tuple<uint32_t,uint32_t>>  numberOfSMS();

    /**
     * @brief returns a message from the storage with a defined index
     * 
     * @param index - messex index
     * @return std::optional<std::tuple<std::string, std::string,  datetime_t>> 
     * 1.st. = message
     * 2.nd. = caller ID
     * 3.rd. = massage time stamp
     */
    std::optional<std::tuple<std::string, std::string,  datetime_t>> readSMS(uint32_t index);
    
    /**
     * @brief Get the Callers ID, can be called after ResponseStatus::callerid status 
     * 
     * @return std::string 
     */
    std::string getCallersID() const {
        return _lastNumber;
    }

    /**
     * @brief last incomming sms storage name, can be called after ResponseStatus::newsms status
     * 
     * @return std::string 
     */
    std::string getsSMSStorage() const {
        return _lastStorage;
    }

    /**
     * @brief last incomming sms index, can be called after ResponseStatus::newsms status
     * 
     * @return int32_t 
     */
    int32_t   incomingSMSIndex() const {
        return _lastMessageNumber;
    }  

    /**
     * @brief Sets the Caller ID will be available after incomming ring
     * 
     * @param enable - enabe caller ID
     * @return true 
     * @return false 
     */
    bool setCallerID(bool enable);

    void whitInfoCallback(GSMInfoCallback clb);

private:
    static const uint32_t _maxWaitTx{100};
    static const uint32_t  _defaultWaitRx{20000};
    static const uint32_t  _maxtWaitRx{200000};
    static const uint32_t _defaultCheckModem{500};

    bool sendCMD_waitResp(const char *str, const char *back, int timeout);
    void eatSerial(uint32_t maxWait = _defaultWaitRx);
    bool sendAndRead(std::string_view command, ResponseStatus reqResStatus, uint32_t maxWait = _defaultWaitRx);
    bool sendCommand(std::string_view command, uint32_t maxWait = _defaultWaitRx);
    bool sendData(std::string_view data, uint32_t maxWait);
    bool processResponse(uint32_t maxWait = _defaultWaitRx);
    std::size_t readLine(uint32_t maxWait);

    ATParser _parser;
    ISerialModem *_serial{nullptr};
    bool _echo{true};
    std::string _lastNumber;
    std::string _lastStorage;
    uint32_t     _lastMessageNumber{0};
    std::string _line;
    GSMInfoCallback _callback{};
};

} // namespace gsm 