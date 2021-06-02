
import bme280
import datetime
import time
import pymysql.cursors
import signal
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
import adafruit_bme280

i2c = busio.I2C(board.SCL,board.SDA)
bme280 = adafruit_bme280.Adafruit_BME280_I2C(board.I2C(),address = 0x76)
ADS0 = ADS.ADS1115(i2c,gain = 1,address = 0x48)
ADS1 = ADS.ADS1115(i2c,gain = 1,address = 0x49)
ch0_10 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

connection  = pymysql.connect(
    user    = "root",
    passwd  = "minoru0869553434",
#    host    = "localhost",
#    host    = "192.168.1.104",
    host    = "127.0.0.1",
    db      = "IoT_TEST",
    charset = "utf8mb4",
    )


def getData(arg1,args2):
    #EnvData = bme280.sample(bus, BMP280address,BMP280calibration_params)
    #timestamp=str(EnvData.timestamp)
    TimeStamp = datetime.datetime.now()
    Humidity = bme280.relative_humidity
    Pressure = bme280.pressure
    Temperature = bme280.temperature
    ch1= AnalogIn(ADS1,ADS.P0)
    for i in range(30):
        ch0 = AnalogIn(ADS0,ADS.P0)
        ch0_10.pop(0)
        ch0_10.append(ch0.voltage)
        time.sleep(0.05)
        ave = sum(ch0_10) / len(ch0_10)

    print("\nTime Stamp: %s" % TimeStamp)
    print("Humidity: %0.1f %%" % Humidity)
    print("Pressure: %0.1f MPa" % Pressure)
    print("Temperature: %0.2f *C" % Temperature)
    print("ch0 raw: %d" % ch0.value)
    print("ch0 v: %f" % ch0.voltage)
    print("ch0 10times average: %0.2f" % ave)
    print(ADS0.read(0))
    print("ch1 v: %f" % ch1. voltage)
     

while True:
    getData(1,1)
#    time.sleep(1)

#connection.ping(reconnect = True)
#cur = connection.cursor()
#cur.execute("INSERT INTO BMP280 (time,temperature,pressure,humidity) VALUES(%s,%s,%s,%s)",(timestamp,EnvData.temperature,EnvData.pressure,EnvData.humidity))
#connection.commit()