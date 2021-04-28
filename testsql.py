import smbus2
import bme280
import datetime
import time
import pymysql.cursors
import signal

connection  = pymysql.connect(
    user    = "root",
    passwd  = "minoru0869553434",
#    host    = "localhost",
#    host    = "192.168.1.104",
    host    = "127.0.0.1",
    db      = "IoT_TEST",
    charset = "utf8mb4",
    )

port = 1
BMP280address = 0x76
bus = smbus2.SMBus(port)

BMP280calibration_params = bme280.load_calibration_params(bus, BMP280address)

EnvData = bme280.sample(bus, BMP280address,BMP280calibration_params)
timestamp=str(EnvData.timestamp)

connection.ping(reconnect = True)
cur = connection.cursor()
cur.execute("INSERT INTO BMP280 (time,temperature,pressure,humidity) VALUES(%s,%s,%s,%s)",(timestamp,EnvData.temperature,EnvData.pressure,EnvData.humidity))
connection.commit()