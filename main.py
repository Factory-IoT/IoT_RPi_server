#M5StickC - RPi wifi
#Ver 0.0.0　作成開始git 
#Ver 1.0.0  m5stick加速度、BMP280環境データ測定

import subprocess
import serial
import smbus2
import bme280
import pymysql.cursors
import signal

BT = True                               #Bluetooth接続モード On:True Off:False
#MAC_ADDRESS = "D8:A0:1D:55:27:7A"
MAC_ADDRESS = "94:B9:7E:8C:64:CA"

DIR = "/home/pi/Workspace/"

if BT == True:
    res = subprocess.check_call(["sudo","rfcomm","bind","1", MAC_ADDRESS])
    ser=serial.Serial("/dev/rfcomm1",115200,timeout=5.0)
    ser.close()

#I2C init
port = 1
BMP280address = 0x76
bus = smbus2.SMBus(port)

class DB:
    def __init__(self):
        print("init")
    def DBwrite(self):
        print("db write")

class BMP280:
    def __init__(self):
        self.BMP280calibration_paramsBMP280calibration_params = bme280.load_calibration_params(bus, BMP280address)
        self.EnvData = bme280.sample(bus, BMP280address,self.BMP280calibration_params)
        self.temp = 0.0
        self.pres = 0.0
        self.humi = 0.0

        #print(EnvData.timestamp)
        #print("temperature : %s *C" % str(round(EnvData.temperature,1)))
        #print("pressure    : %s %%" % str(round(EnvData.pressure,1)))
        #print("humidity    : %s hPa" % str(round(EnvData.humidity,1)))
        print(EnvData)

    def get(self):
        self.EnvData = bme280.sample(bus, BMP280address,self.BMP280calibration_params)
        self.temp = round(self.EnvData.temperature,1)
        self.pres = round(self.EnvData.puressssure,1)
        self.humi = round(self.EnvData.humidity,1)

class Accel:
    def __init__(self):
        self.samplingTime = []
        self.ax = []
        self.ay = []
        self.az = []
        self.ampx = []
        self.ampy = []
        self.ampz = []
        self.freq = []

        self.mesureNum = 0
        self.sampleSize = 0

    def get(self):
        if BT ==True:
            ser.open()
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            print("send check code")
            BTCommand = 1
            BTCommandByte=bytes([BTCommand])
            ser.write(BTCommandByte)
            
            sampleSizeRaw = ser.readline()
            self.sampleSize = int(repr(sampleSizeRaw.decode())[1:-5])
    
            mesureNumRaw = ser.readline()
            self.mesureNum = int(repr(mesureNumRaw.decode())[1:-5])
    
            print("sampleSize = " + str(self.sampleSize))
            print("mesureNum = " + str(self.mesureNum))
            print("data recieving." , end = "")
       
            for i in range(self.sampleSize):
                samplingTime_data = ser.readline()
                ax_data = ser.readline()
                ay_data = ser.readline()
                az_data = ser.readline()
                self.samplingTime.append(float(repr(samplingTime_data.decode())[1:-5]))
                self.ax.append(float(repr(ax_data.decode())[1:-5]))
                self.ay.append(float(repr(ay_data.decode())[1:-5]))
                self.az.append(float(repr(az_data.decode())[1:-5]))

            ser.close()

#setup
BMP280()
Accel()



while True:
    BMP280.get()
    time.sleep(1)
