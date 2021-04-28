#IoT test
#Ver 0.0.0　作成開始git 
#BMP280使用


import smbus2
import bme280
import datetime
import pymysql.cursors



port = 1
BMP280address = 0x76
bus = smbus2.SMBus(port)

BMP280calibration_params = bme280.load_calibration_params(bus, BMP280address)
EnvData = bme280.sample(bus, BMP280address,BMP280calibration_params)

print(EnvData.timestamp)
print("temperature : %s *C" % str(round(EnvData.temperature,1)))
print("pressure    : %s %%" % str(round(EnvData.pressure,1)))
print("humidity    : %s hPa" % str(round(EnvData.humidity,1)))

print(EnvData)