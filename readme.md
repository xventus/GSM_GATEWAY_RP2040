# GSM gateway
The GSM & GPS gateway serves as a control gateway for outputs controllable directly on the GPIO RP2040, also accessible via a specific serial protocol via RS485. The protocol also provides GMT normal time from GSM or GPS source. 

# Configuring storage in flash
The RP2040 does not contain EEPROM for configuration storage, so the 4k block of FLASH memory located at the end of it can be used for configuration storage. Specifically for the RP2040 PICO version the board contains 2MB of memory. 

For more information see flash_storage.h

way of storing data call / value in memory:
```
000000: 24 24 46 4c 53 48 53 54 4f 52 41 47 45 40 40 40 [ $$FLSHSTORAGE@@@ ]
000010: 70 68 6f 6e 65 31 5e 2b 34 32 30 36 30 33 33 33 [ phone1^+42061322 ]
000020: 36 31 34 31 7e 70 68 6f 6e 65 32 5e 2b 34 32 30 [ 3333~phone2^+420 ]
000030: 37 33 31 39 32 36 37 35 34 7e 00 00 00 00 00 00 [ 123458754~...... ]
000040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 [ ................ ]
000050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 [ ................ ]
000060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 [ ................ ]
```

# SMS commands and user registration

In case of uninitialized storage or when the REINIT_BUTTON_PIN input (hardware.h) is activated, the gateway enters the registration mode. In this mode, it expects the first user who acts as an administrator to register by ringing the gateway. The administrator can also allow additional users to be added. Complex SMS sending is not necessary, just ring the modem.  The display in this case shows the message "Call Me". The user will receive a confirmation SMS back. 

From this moment the user can send commands that are used to control the gate. 

The commands can be changed by hitting the structure array see gsm_message.h.

The user can view the list of enabled commands by sending any message that is not occupied between commands. For example. HELP. 

![screen](/img/gtw.png)

The control of the individual outputs is performed in the default configuration e.g. 2 ON, which turns on output 2. The user receives back the information about the receipt of the command. The State command can be used to display information about the individual outputs. 

![screen](/img/gtw2.png)

The administrator (the first registered user) can add more users using the ADD command. After this command, the gateway expects the new user to call the gateway again and the new user is registered. 

The next command is LIST, which the administrator uses to display a list of all registered users. 


# Display

The display shows the status of modem initialization, SIM card checking, site registration, etc. If an error occurs, the display shows the error. The heartbeat status LED will flash faster. 

The individual lines on the display inform:

1, Operator name
2, UTC date 
3, UTC time
4, Output states, status information
5, Signal strength

The date and time is obtained from the GPS (if available) and the GSM site. This case is then used to synchronize the internal RTC. 

The time is shown on the display with a certain period when the information from the modem is obtained. Therefore, the time may be out of date. It is only active when the time is read and displayed. This time is only for checking. Deviation up to 1 minute. 


![screen](/img/gtw3.png)


# Terminal protocol 

The terminal protocol is used for communication with other devices, primarily via RS485. It distinguishes two types of commands. One is for testing and sending via terminal programs. The second type is for device to device communication, where the last byte contains the checksum (terminal_proto.h) of the command. 

This primitive protocol was created for the needs of controlling the output on another device and getting the exact UTC time. GSM gateway is used for these two purposes. Although the GSM gateway allows and controls directly its outputs on the RP2040, the main purpose was to pass this information to another device which after receiving and executing the command has the possibility to switch off the outputs as a whole. 


The whole protocol is based on ASCII communication. 

```
       Request:Address|Commnad|;<Checksum>
       Address|Commnad|numeric value;<Checksum>

       Address - ASCII character, e.g. "T" - terminal
       Commnad - ASCII capital letter is a command without checkusm
               - ASCII lowercase letter with checksum
       Checkusum - ASCII representation of checksum

       Commands:
           R, r - read output status
           C, c - clear output state (0 - 255) set bits to be cleared
           T, t - get time
           A, a - get human readable time - UTC

       Response:
           Address|Command|Value number;<Checksum>
           Address and command are repeated in the response

       the semicolon is always a separator between the value and the optional checksum
```

Example - get UTC time as formated string :
```
TA;
TA16:32:06 02/04/2023;
```

Example - get UTC time as unix time :
```
TT;
TT1680453242;
```

Example - determine which outputs are swithech on (bits ar sets)  :
decimal value 7 means bits 0, 1, 2 are sets to ON
```
TR;
TR7;
```

Example - reset outputs :
```
TC;
TC0;
```

# Hardware

Stacked modules can easily be used for the entire assembly.  As a basis raspberry PICO, GSM modem module and RS 485. The LCD 5110 display is then connected to it.

Raspberry Pi Pico

https://www.raspberrypi.com/products/raspberry-pi-pico/

Pico SIM868-GSM/GPRS/GNSS

https://www.waveshare.com/wiki/Pico-SIM868-GSM/GPRS/GNSS

Disconnect (cut off) the pins on GPIO0 and GPIO1 between the RS485 module and the modem. Only one channel can be used from the RS485 module. GPIO0 and GPIO1 are used for communication with the GSM module. 

Pico-2CH-RS485 

https://www.waveshare.com/wiki/Pico-2CH-RS485



# License

For non-commercial use only!
