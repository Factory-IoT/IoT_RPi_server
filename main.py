#M5StickC - RPi wifi
#Ver 0.0.0　作成開始git 
#Ver 1.0.0  m5stick加速度、BMP280環境データ測定

import subprocess
import serial
import smbus2
import bme280
import pymysql.cursors
import signal
import time
import datetime

timesecond = 60 #測定間隔(sec)
AccelFreq  = 60 #加速度記録の頻度

BT = True                               #Bluetooth接続モード On:True Off:False
#MAC_ADDRESS = "D8:A0:1D:55:27:7A"
MAC_ADDRESS = "94:B9:7E:8C:64:CA"

#if BT == True:
#    res = subprocess.check_call(["sudo","rfcomm","bind","1", MAC_ADDRESS])
#    ser=serial.Serial("/dev/rfcomm1",115200,timeout=2.0)
#    ser.close()

#I2C init
port = 1
BMP280address = 0x76
bus = smbus2.SMBus(port)

#DB init
connection  = pymysql.connect(
    user    = "root",
    passwd  = "minoru0869553434",
    host    = "127.0.0.1",
    db      = "IoT_TEST",
#    db      = "DA755"
    charset = "utf8mb4",
    )
    


class DB:
    def __init__(self):
        print("init")
        self.count = 1
    def write(self):
        print("db write")
        TimeStamp = str(datetime.datetime.now())
        connection.ping(reconnect = True)
        cur = connection.cursor()
        cur.execute("INSERT INTO Environment (Time,AtmTemp,AtmHum,AtmPress) VALUES(%s,%s,%s,%s)",
            (TimeStamp,BMP280.Temp,BMP280.Hum,BMP280.Press))
        #connection.commit()
        if(self.count % AccelFreq == 0):
            self.count = 1
            for i in range(Accel.sampleSize):
#                print(Accel.samplingTime[i])
#                print(Accel.ax[i])
                #cur = connection.cursor()
                cur.execute("INSERT INTO OilMotorAccel (Time,dt,OilMOtorAccelX,OilMotorAccelY,OilMotorAccelZ) VALUES(%s,%s,%s,%s,%s)",
                    (TimeStamp,Accel.samplingTime[i],Accel.ax[i],Accel.ay[i],Accel.az[i])) 
                #connection.commit()
#        print(self.count)
        self.count += 1

        connection.commit()

class BMP280:
    def __init__(self):
        self.BMP280calibration_params = bme280.load_calibration_params(bus, BMP280address)
        self.EnvData = bme280.sample(bus, BMP280address,self.BMP280calibration_params)
        self.Temp  = 0.0
        self.press = 0.0
        self.Hum   = 0.0

        #print(EnvData.timestamp)
        #print("temperature : %s *C" % str(round(EnvData.temperature,1)))
        #print("pressure    : %s %%" % str(round(EnvData.pressure,1)))
        #print("humidity    : %s hPa" % str(round(EnvData.humidity,1)))
        print(self.EnvData)

    def get(self):
        print("get")
        self.EnvData = bme280.sample(bus, BMP280address,self.BMP280calibration_params)
        self.Temp = round(self.EnvData.temperature,1)
        self.Press = round(self.EnvData.pressure,1)
        self.Hum = round(self.EnvData.humidity,1)
        print(self.EnvData)


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
            sampleSizeRaw = b''
            try:
                res = subprocess.check_call(["sudo","rfcomm","bind","1", MAC_ADDRESS])
                ser = serial.Serial("/dev/rfcomm1",115200,timeout=2.0)
                ser.reset_input_buffer()
                ser.reset_output_buffer()
                print("send check code")
                BTCommand = 1
                BTCommandByte=bytes([BTCommand])
                ser.write(BTCommandByte)
                sampleSizeRaw = ser.readline()
                print(sampleSizeRaw)
            except:
                print("cant open")
#            print(type(SampleSizeRaw))
            if(sampleSizeRaw == b''):
                res = subprocess.check_call(["sudo","rfcomm","release","1"])
                return
            self.sampleSize = int(repr(sampleSizeRaw.decode())[1:-5])
            
            mesureNumRaw = ser.readline()
            if(mesureNumRaw == b''):
                res = subprocess.check_call(["sudo","rfcomm","release","1"])
                return
            self.mesureNum = int(repr(mesureNumRaw.decode())[1:-5])
    
            print("sampleSize = " + str(self.sampleSize))
            print("mesureNum = " + str(self.mesureNum))
            print("data recieving." , end = "")
       
            for i in range(self.sampleSize):
                samplingTime_data = ser.readline()
                if(samplingTime_data == b''):
                    res = subprocess.check_call(["sudo","rfcomm","release","1"])
                    print("sampleSize")
                    return
                ax_data = ser.readline()
                if(ax_data == b''):
                    res = subprocess.check_call(["sudo","rfcomm","release","1"])
                    return
                ay_data = ser.readline()
                if(ay_data == b''):
                    res = subprocess.check_call(["sudo","rfcomm","release","1"])
                    return
                az_data = ser.readline()
                if(az_data == b''):
                    res = subprocess.check_call(["sudo","rfcomm","release","1"])
                    return
                self.samplingTime.append(float(repr(samplingTime_data.decode())[1:-5]))
                self.ax.append(float(repr(ax_data.decode())[1:-5]))
                self.ay.append(float(repr(ay_data.decode())[1:-5]))
                self.az.append(float(repr(az_data.decode())[1:-5]))

                #self.samplingTime.append(str(repr(samplingTime_data.decode())[1:-5]))
                #self.ax.append(str(repr(ax_data.decode())[1:-5]))
                #self.ay.append(str(repr(ay_data.decode())[1:-5]))
                #self.az.append(str(repr(az_data.decode())[1:-5]))


            ser.close()
            print("release")
            res = subprocess.check_call(["sudo","rfcomm","release","1"])
#                res = subprocess.check_call(["sudo","rfcomm","bind","1", MAC_ADDRESS])
#                ser=serial.Serial("/dev/rfcomm1",115200,timeout=2.0)



#setup
BMP280 = BMP280()
Accel = Accel()
DB = DB()

def task(arg1,args2):
    print(datetime.datetime.now())
    BMP280.get()
    print(datetime.datetime.now())
    Accel.get()
    print(datetime.datetime.now())
    DB.write()
    time.sleep(1)

signal.signal(signal.SIGALRM,task)
signal.setitimer(signal.ITIMER_REAL,1,timesecond)

while True:
    print(time.time())
    time.sleep(10)