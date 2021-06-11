from board import *
from time import sleep
import busio
import adafruit_ssd1306
 
i2c = busio.I2C(SCL, SDA)
display = adafruit_ssd1306.SSD1306_I2C(128, 64, i2c, addr=0x3c)
 
print("start")
display.fill(0)
display.text('hello world', 0, 0, 1)
display.show()
del display
sleep(1)