#IoT test
#Ver 0.0.0　作成開始git 
#BMP280使用


import smbus2
import bme280
import datetime
import time
import pymysql.cursors
import signal

connection  = pymysql.connect(
    user    = "root",
    passwd  = "minoru0869553434",
    host    = "127.0.0.1",
    db      = "IoT_TEST",
    charset = "utf8mb4",
    )

port = 1
BMP280address = 0x76
bus = smbus2.SMBus(port)

BMP280calibration_params = bme280.load_calibration_params(bus, BMP280address)

def getData(arg1,args2):
    EnvData = bme280.sample(bus, BMP280address,BMP280calibration_params)
    timestamp=str(EnvData.timestamp)
    print(datetime.datetime.now())
    #connection.ping(reconnect = True)
    cur = connection.cursor()
    cur.execute("INSERT INTO BMP280 (time,temperature,pressure,humidity) VALUES(%s,%s,%s,%s)",(timestamp,EnvData.temperature,EnvData.pressure,EnvData.humidity))
    connection.commit()
    print(datetime.datetime.now())
    print(timestamp)
    print("temperature : %s *C" % str(round(EnvData.temperature,1)))
    print("pressure    : %s hpa" % str(round(EnvData.pressure,1)))
    print("humidity    : %s %%" % str(round(EnvData.humidity,1)))


while True:
    getData(1,1)
    time.sleep(1)

#signal.signal(signal.SIGALRM,getData)
#signal.setitimer(signal.ITIMER_REAL,1,1)

#time.sleep(10)
#while True:
#    print(time.time())
#
#   time.sleep(1)