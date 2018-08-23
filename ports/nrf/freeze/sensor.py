# This file is part of the MicroPython project, http://micropython.org/
#
# The MIT License (MIT)
#
# Copyright (c) 2018 Glenn Ruben Bakke
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE

from machine import RTCounter, Temp, Pin, ADC
from ubluepy import Service, Characteristic, UUID, Peripheral, constants
import machine

def event_handler(id, handle, data):
    global rtc
    global periph
    global serv_env_sense
    global temp_notif_enabled
    global batt_notif_enabled
    global rain_notif_enabled
    global char_temp
    global char_rain
    global char_batt

    if id == constants.EVT_GAP_CONNECTED:
        pass

    elif id == constants.EVT_GAP_DISCONNECTED:
        # stop low power timer
        rtc.stop()
        temp_notif_enabled = False
        rain_notif_enabled = False
        # restart advertisment
        periph.advertise(device_name="micr_rain", services=[serv_env_sense])
        
    elif id == constants.EVT_GATTS_WRITE:
        # write to this Characteristic is to CCCD
        if handle == char_temp.getHandle() + 1:
            if int(data[0]) == 1:
                temp_notif_enabled = True
            else:
                temp_notif_enabled = False
        elif handle == char_batt.getHandle() + 1:
            if int(data[0]) == 1:
                batt_notif_enabled = True
            else:
                batt_notif_enabled = False
        elif handle == char_rain.getHandle() + 1:
            if int(data[0]) == 1:
                rain_notif_enabled = True
            else:
                rain_notif_enabled = False

        if temp_notif_enabled or batt_notif_enabled:
            # start low power timer
            rtc.start()

        if not temp_notif_enabled and not batt_notif_enabled:
            # stop low power timer
            rtc.stop()
        

def rtc_handler(timer_id):
    global temp_notif_enabled
    global char_temp
    global char_batt
    global adc2

    if temp_notif_enabled:
        # measure chip temperature
        #temp = Temp.read()
        #temp =  temp * 100 // 2
        temp = adc2.value()
        temp = (temp * 1200) // 255
        temp = temp - 500
        temp = temp // 5
        # get back the 5 div (rounding to half centigrade)
        # and get it scaled to BLE format (*10)
        temp = temp * 50
        char_temp.write(bytearray([temp & 0xFF, temp >> 8]))

    if batt_notif_enabled:
        level = ADC.battery_level()
        if level > 100:
            level = 100
        char_batt.write(bytearray([level]))

def send_rain_update(p):
    global rain_notif_enabled
    global char_rain
    global tick_count
    global wrap_around

    tick_count += 1

    if rain_notif_enabled:
        # measure chip temperature
        char_rain.write(bytearray([tick_count % 256, wrap_around]))
        wrap_around = 0

    if tick_count >= 255:
        tick_count = 0

rain_notif_enabled = False
temp_notif_enabled = False
batt_notif_enabled = False

tick_count = 0
wrap_around = 0

adc2 = ADC(2)

# use RTC1 as RTC0 is used by bluetooth stack 
# set up RTC callback every 5 second
rtc = RTCounter(1, period=50, mode=RTCounter.PERIODIC, callback=rtc_handler)

uuid_env_sense = UUID("0x181A") # Environmental Sensing service
uuid_temp = UUID("0x2A6E") # Temperature characteristic
uuid_rain = UUID("0x2A78") # Rainfall characteristic

serv_env_sense = Service(uuid_env_sense)

temp_props = Characteristic.PROP_NOTIFY | Characteristic.PROP_READ
temp_attrs = Characteristic.ATTR_CCCD
char_temp = Characteristic(uuid_temp, props = temp_props, attrs = temp_attrs)

rain_props = Characteristic.PROP_NOTIFY | Characteristic.PROP_READ
rain_attrs = Characteristic.ATTR_CCCD
char_rain = Characteristic(uuid_rain, props = rain_props, attrs = rain_attrs)

serv_env_sense.addCharacteristic(char_temp)
serv_env_sense.addCharacteristic(char_rain)

uuid_battery = UUID("0x180F") # Battery service
uuid_battery_level = UUID("0x2A19") # Battery level characteristic

serv_battery = Service(uuid_battery)

battery_props = Characteristic.PROP_NOTIFY | Characteristic.PROP_READ
battery_attrs = Characteristic.ATTR_CCCD
char_batt = Characteristic(uuid_battery, props = battery_props, attrs = battery_attrs)

serv_battery.addCharacteristic(char_batt)

periph = Peripheral()
periph.addService(serv_env_sense)
periph.addService(serv_battery)
periph.setConnectionHandler(event_handler)
periph.advertise(device_name="micr_rain", services=[serv_env_sense, serv_battery])

p = Pin(Pin.cpu.P18, mode=Pin.IN, pull=Pin.PULL_UP)
p.irq(handler=send_rain_update, trigger=Pin.IRQ_FALLING)

while True:
    machine.sleep()
