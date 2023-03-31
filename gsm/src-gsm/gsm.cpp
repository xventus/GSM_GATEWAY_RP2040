//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   gsm.h
/// @author Petr Vanek

#include <memory.h>
#include "gsm.h"
#include "gsm_commands.h"

namespace gsm
{

	bool GSM::init(bool withGnss)
	{

		auto callbck = [this](GsmInfoState nfo)
		{
			if (_callback)
				_callback(nfo);
		};

		bool rc = false;
		do
		{

			if (!_serial)
				break;

			callbck(GsmInfoState::hwinit);
			_serial->hardwareInit();
			callbck(GsmInfoState::wait);
			_serial->delay(5000);

			while (true)
			{
				callbck(GsmInfoState::ping);
				if (ping())
				{
					callbck(GsmInfoState::ping);
					eatSerial(_defaultWaitRx);
					callbck(GsmInfoState::ping);
					if (ping() && ping())
					{
						callbck(GsmInfoState::ping);
						rc = echoOff();
						break;
					}
				}
				callbck(GsmInfoState::ping);
				_serial->modemInit();
				callbck(GsmInfoState::ping);
				_serial->delay(2000);
			}

			if (withGnss)
			{
				callbck(GsmInfoState::gnss);
				rc = gnssPower(true);
			}

		} while (false);
		return rc;
	}

	ResponseStatus GSM::checkStatus(uint32_t maxWait)
	{
		ResponseStatus rc = ResponseStatus::unknown;
		uint64_t t = 0;

		_lastStorage.clear();
		_lastMessageNumber = 0;
		_lastNumber.clear();

		do
		{
			if (!_serial)
				break;

			_parser.init(ParseCode::aBegin);
			_parser.withPrefferedTag(ResponseStatus::unknown);

			if (!readLine(maxWait))
				break;

			if (!_parser.parse(_line))
				break;

			rc = _parser.getResponseStatus();

			if (rc == ResponseStatus::newsms)
			{
				auto smsmrx = _parser.evalueateTextAndNumber();
				if (smsmrx.has_value())
				{
					auto [lastStorage, lastMessageNumber] = smsmrx.value();
					_lastStorage = lastStorage;
					_lastMessageNumber = (uint32_t)lastMessageNumber;
				}
			}
			else if (rc == ResponseStatus::callerid)
			{
				auto callid = _parser.evalueateText();
				if (callid.has_value())
				{
					_lastNumber = callid.value();
				}
			}

		} while (false);

		return rc;
	}

	void GSM::eatSerial(uint32_t maxWait)
	{
		uint64_t t = 0;
		while (true)
		{
			if ((_serial->getus() - t) > (maxWait * 1000))
				break;
			if (_callback) _callback(GsmInfoState::ping);
			readLine(maxWait);
		}
		return;
	}

	bool GSM::flightMode()
	{
		return sendAndRead(gsm_cmd::C_ATCFUN0, ResponseStatus::ok, _defaultWaitRx);
	}

	bool GSM::phoneMode()
	{
		auto rc = sendAndRead(gsm_cmd::C_ATCFUN1, ResponseStatus::ok, _defaultWaitRx);
		if (rc)
		{
			// eat like this
			// RDY
			// +CFUN: 1
			// ..
			eatSerial(2000);
		}
		return rc;
	}

	bool GSM::echoOff()
	{
		auto rc = sendAndRead(gsm_cmd::C_ATE0, ResponseStatus::ok, _defaultWaitRx);
		if (rc)
			_echo = false;
		return rc;
	}

	bool GSM::echoOn()
	{
		auto rc = sendAndRead(gsm_cmd::C_ATE1, ResponseStatus::ok, _defaultWaitRx);
		if (rc)
			_echo = true;
		return rc;
	}

	bool GSM::ping()
	{
		return sendAndRead(gsm_cmd::C_AT, ResponseStatus::ok, _defaultCheckModem);
	}

	bool GSM::setCallerID(bool enable)
	{
		return sendAndRead(enable ? gsm_cmd::C_CLIPON : gsm_cmd::C_CLIPOFF, ResponseStatus::ok, _defaultWaitRx);
	}

	void GSM::whitInfoCallback(GSMInfoCallback clb)
	{
		_callback = clb;
	}

	bool GSM::ring(std::string_view phoneNumber)
	{
		bool rc = false;
		std::string message;
		do
		{
			if (phoneNumber.empty())
				break;

			if (!disconnect())
				break;

			message = gsm_cmd::C_ATD;
			message += " ";
			message += phoneNumber;
			message += ";";

			if (!sendAndRead(message, ResponseStatus::ok, _defaultWaitRx))
				break;

			rc = true;

		} while (false);
		return rc;
	}

	bool GSM::disconnect()
	{
		return sendAndRead(gsm_cmd::C_ATH, ResponseStatus::ok, _defaultCheckModem);
	}

	bool GSM::answer()
	{
		return sendAndRead(gsm_cmd::C_ATA, ResponseStatus::ok, _defaultCheckModem);
	}

	std::optional<bool> GSM::pinNotRequired()
	{
		std::optional<bool> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCPIN, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto val = _parser.compareResponseText(gsm_cmd::C_READY);
			if (val.has_value())
			{
				rc = val.value();
			}

		} while (false);
		return rc;
	}

	std::optional<std::string> GSM::getICCID()
	{
		std::optional<std::string> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCCID, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto val = _parser.getText(gsm_cmd::C_ATCCID);
			if (val.has_value())
			{
				rc = val.value();
			}

		} while (false);
		return rc;
	}

	std::optional<std::string> GSM::getOperator()
	{
		std::optional<std::string> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCOPS, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto val = _parser.evalueate2ComaValuesString();
			if (val.has_value())
			{
				auto [a, b, str] = val.value();
				rc = str;
			}

		} while (false);
		return rc;
	}

	std::optional<std::tuple<datetime_t, bool, bool>> GSM::gnssInfo()
	{
		std::optional<std::tuple<datetime_t, bool, bool>> rc = std::nullopt;
		sendAndRead(gsm_cmd::C_CCGNSINF, ResponseStatus::ok, _defaultCheckModem);
		rc = _parser.evaluateGNSSTime(gsm_cmd::C_CGNSINFA);
		return rc;
	}

	bool GSM::gnssPower(bool up)
	{
		return sendAndRead((up) ? gsm_cmd::C_CGNSPWRON : gsm_cmd::C_CGNSPWROFF, ResponseStatus::ok, _defaultWaitRx);
	}

	std::optional<std::tuple<uint32_t, uint32_t>> GSM::numberOfSMS()
	{
		std::optional<std::tuple<uint32_t, uint32_t>> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_CATPMS, ResponseStatus::msgnum, _defaultWaitRx))
				break;

			auto val = _parser.evalueateTextAndTwoNumber();
			if (val.has_value())
			{
				auto [storage, cnt, all] = val.value();
				rc = std::make_tuple(cnt, all);
				_lastStorage = storage;
			}

		} while (false);

		return rc;
	}

	std::optional<std::tuple<std::string, std::string, datetime_t>> GSM::readSMS(uint32_t index)
	{
		std::optional<std::tuple<std::string, std::string, datetime_t>> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCMGFON, ResponseStatus::ok, _defaultWaitRx))
				break;

			std::string cmd;
			cmd = gsm_cmd::C_CMGR000;
			cmd += std::to_string(index);
			if (!sendAndRead(cmd, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto sms = _parser.evaluateSMS(gsm_cmd::C_CMGR);
			if (sms.has_value())
			{
				auto [id, tm, msg] = sms.value();
				auto tmx = _parser.breakTime(tm);
				if (!tmx.has_value())
					break;

				rc = std::make_tuple(msg, id, tmx.value());
			}

		} while (false);
		return rc;
	}

	bool GSM::delAllSMS()
	{
		auto rc = false;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCMGFON, ResponseStatus::ok, _defaultWaitRx))
				break;

			std::string cmd;
			cmd = gsm_cmd::C_CMGD000;
			cmd += "1,4";
			if (!sendAndRead(cmd, ResponseStatus::ok, _defaultWaitRx))
				break;

			rc = true;

		} while (false);
		return rc;
	}

	bool GSM::delSMS(uint32_t index)
	{
		bool rc = false;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCMGFON, ResponseStatus::ok, _defaultWaitRx))
				break;

			std::string cmd;
			cmd = gsm_cmd::C_CMGD000;
			cmd += std::to_string(index);
			if (!sendAndRead(cmd, ResponseStatus::ok, _defaultWaitRx))
				break;

			rc = true;

		} while (false);

		return rc;
	}

	bool GSM::sendSMS(std::string_view phoneNumber, std::string_view ascii7Text)
	{

		bool rc = false;
		std::string message;
		do
		{
			if (phoneNumber.empty())
				break;

			if (ascii7Text.empty())
				break;

			if (!sendAndRead(gsm_cmd::C_ATCMGFON, ResponseStatus::ok, _defaultWaitRx))
				break;

			message = gsm_cmd::C_ATCMGS;
			message += "=\"";
			message += phoneNumber;
			message += "\"";

			if (!sendAndRead(message, ResponseStatus::smsready, _defaultWaitRx))
				break;

			if (!sendData(ascii7Text, _maxWaitTx))
				break;

			if (!sendAndRead(gsm_cmd::CTRLZCR, ResponseStatus::ok, _defaultWaitRx))
				break;

			rc = true;

		} while (false);
		return rc;
	}

	std::optional<uint8_t> GSM::qualitySignal()
	{
		std::optional<uint8_t> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCSQ, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto vals = _parser.evalueate2ComaValues();
			if (vals.has_value())
			{
				auto [rssi, ber] = vals.value();
				rc = rssi;
			}

		} while (false);
		return rc;
	}

	std::optional<uint8_t> GSM::isRegistered()
	{
		std::optional<uint8_t> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCREG, ResponseStatus::ok, _defaultWaitRx))
				break;

			auto vals = _parser.evalueate2ComaValues();
			if (vals.has_value())
			{
				auto [n, stat] = vals.value();
				rc = stat;
			}

		} while (false);
		return rc;
	}

	std::optional<datetime_t> GSM::readRTC()
	{

		std::optional<datetime_t> rc = std::nullopt;
		do
		{
			if (!sendAndRead(gsm_cmd::C_ATCCLK, ResponseStatus::clock, _defaultWaitRx))
				break;

			rc = _parser.evalueateDateTime();

		} while (false);
		return rc;
	}

	bool GSM::enableRTC(bool enable)
	{
		auto rc = false;
		do
		{
			rc = sendAndRead((enable) ? gsm_cmd::C_ATCCLKON : gsm_cmd::C_ATCCLKOFF, ResponseStatus::ok, _defaultWaitRx);
			if (!rc)
				break;
			rc = sendAndRead(gsm_cmd::C_ATW, ResponseStatus::ok, _defaultWaitRx);

		} while (false);
		return rc;
	}

	bool GSM::sendAndRead(std::string_view command, ResponseStatus reqResStatus, uint32_t maxWait)
	{
		bool rc = false;
		do
		{
			if (!sendCommand(command))
				break;

			_parser.withPrefferedTag(reqResStatus);
			if (!processResponse(maxWait))
				break;

			if (_parser.getResponseStatus() == reqResStatus)
			{
				rc = true;
				break;
			}

		} while (false);
		return rc;
	}

	std::size_t GSM::readLine(uint32_t maxWait)
	{

		_line.clear();
		auto tt = _serial->getus();
		while (_serial->getus() - tt < maxWait * 1000)
		{
			if (_serial->isReadable())
			{
				auto c = _serial->getc();
				_line += c;
				if (c == _ignorelineDelim)
					break;
			}
		}

		return _line.length();
	}

	bool GSM::processResponse(uint32_t maxWait)
	{
		auto callbck = [this](GsmInfoState nfo)
		{
			if (_callback)
				_callback(nfo);
		};

		bool rc = false;

		_parser.init(ParseCode::aBegin);

		auto tt = _serial->getus();

		callbck(GsmInfoState::rx);

		while (true)
		{

			if (readLine(maxWait))
			{
				if (_parser.parse(_line))
				{
					rc = true;
					break;
				}
			}

			if (_serial->getus() - tt > maxWait * 1000)
				break;
		}

		return rc;
	}

	bool GSM::sendCommand(std::string_view command, uint32_t maxWait)
	{
		bool rc = false;
		std::size_t pos{0};
		uint64_t t = 0;
		t = _serial->getus();
		std::string acc;

		if (_callback)
			_callback(GsmInfoState::tx);

		while (true)
		{
			if (!_serial)
				break;

			if ((_serial->getus() - t) > maxWait * 1000)
				break;

			if (!_serial->isWritable())
				continue;

			if (pos != command.length())
			{
				acc += command.at(pos);
				_serial->putch(command.at(pos++));
			}
			else
			{
				_serial->putch('\r');
				_serial->putch('\n');
				rc = true;
				break;
			}
		}

		return rc;
	}

	bool GSM::sendData(std::string_view data, uint32_t maxWait)
	{
		bool rc = false;
		std::size_t pos{0};
		uint64_t t = 0;
		t = _serial->getus();
		std::string acc;

		if (_callback)
			_callback(GsmInfoState::tx);

		while (true)
		{
			if (!_serial)
				break;

			if ((_serial->getus() - t) > maxWait * 1000)
				break;

			if (!_serial->isWritable())
				continue;

			if (pos != data.length())
			{
				acc += data.at(pos);
				_serial->putch(data.at(pos++));
			}
			else
			{
				rc = true;
				break;
			}
		}

		return rc;
	}

} // namespace gsm
