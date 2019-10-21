import time
from machine import RTCounter
def cd(timer_id):
    global tick
    print("callback", tick)
    tick += 1
tick = 0
rtc = RTCounter(0, period=50, mode=RTCounter.PERIODIC, callback=cd)
rtc.start()
time.sleep_ms(10 * 1000)
rtc.stop()
