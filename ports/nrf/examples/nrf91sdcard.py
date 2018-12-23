import os
import sdcard
from machine import Pin
from machine import SPI
from sdcard import SDCard

sck = Pin(21, mode=Pin.OUT)
miso = Pin(22, mode=Pin.IN, pull=Pin.PULL_UP)
mosi = Pin(19, mode=Pin.OUT)
cs = Pin(25, mode=Pin.OUT, pull=Pin.PULL_UP)
from machine import SPI
spi = SPI(1, sck=sck, mosi=mosi, miso=miso)
sd = SDCard(spi, cs)
os.mount(sd, '/sd')

