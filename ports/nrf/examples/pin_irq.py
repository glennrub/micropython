from machine import Pin
p = Pin(8, mode=Pin.IN, pull=Pin.PULL_UP)
p.value()


def pin_callback(pin):
    print("Pin IRQ: ", pin.value())

p.irq(handler=pin_callback)
