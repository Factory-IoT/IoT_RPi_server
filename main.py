#M5StickC - RPi wifi
#Ver 0.0.0　作成開始git 
#Ver 1.0.0  m5stick加速度、BMP280環境データ測定
#Ver 1.1.0 library をadafruiteに変更、analog class追加、空圧測定追加
#Ver 1.1.1 いろいろ修正。テーブルテスト完了

import pymysql.cursors
import signal
import time
import datetime
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
import adafruit_bme280
import serial
import subprocess

#i2c 設定
i2c = busio.I2C(board.SCL,board.SDA)

#bme280 インスタンス
bme280 = adafruit_bme280.Adafruit_BME280_I2C(board.I2C(),address = 0x76)

#ADS1115 インスタンス ADS0 24V系 P0:チラー流量 P1:チラー温度 P2:未使用 P3:未使用
#ADS1115 インスタンス ADS1 24V系 P0:エア圧 P1-3:未使用
ADS0 = ADS.ADS1115(i2c,gain = 1,address = 0x48)
WaterFlow1 = AnalogIn(ADS0,ADS.P0)
WaterTemp1 = AnalogIn(ADS0,ADS.P1)
ADS1 = ADS.ADS1115(i2c,gain = 1,address = 0x49)
AirPress1 = AnalogIn(ADS1,ADS.P0) 

timesecond = 60 #測定間隔(sec)
AccelFreq  = 60 #加速度記録の頻度

BT = False                               #Bluetooth接続モード On:True Off:False
#MAC_ADDRESS = "D8:A0:1D:55:27:7A"
MAC_ADDRESS = "94:B9:7E:8C:64:CA"

if BT == True:
    res = subprocess.check_call(["sudo","rfcomm","bind","1", MAC_ADDRESS])
    ser=serial.Serial("/dev/rfcomm1",115200,timeout=2.0)
    ser.close()

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
        print("DB init")
        self.count = 1
    def Write(self):
        print("db write")
        TimeStamp = str(datetime.datetime.now())
        connection.ping(reconnect = True)
        cur = connection.cursor()
        cur.execute("INSERT INTO Environment (Time,Temp,Hum,Press) VALUES(%s,%s,%s,%s)",
            (BME280.TimeStamp,BME280.Temp,BME280.Hum,BME280.Press))
        cur.execute("INSERT INTO Air (Time,Press) VALUES(%s,%s)",(Air.TimeStamp,Air.Press))
        cur.execute("INSERT INTO Water (Time,ChillFlow,ChillTemp) VALUES(%s,%s,%s)",(Water.TimeStamp,Water.ChillFlow,Water.ChillTemp))

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

        self.count += 1 #加速度測定頻度管理用カウンタ

        connection.commit()

class BME280:
    def __init__(self):
        self.TimeStamp = datetime.datetime.now()
        self.Temp  = 0.0
        self.Press = 0.0
        self.Hum   = 0.0

    def Read(self):
        self.TimeStamp = datetime.datetime.now()
        self.Temp = round(bme280.temperature,2)
        self.Press = round(bme280.pressure,2)
        self.Hum = round(bme280.relative_humidity,2)

class Water:
    def __init__(self):
        self.TimeStamp = datetime.datetime.now()
        self.ChillFlow = round(WaterFlow1.voltage * 208.33333 - 125,2)
        self.ChillTemp = round(WaterTemp1.voltage * 58.33333 - 55,2)

    def Read(self):
        self.TimeStamp = datetime.datetime.now()
        self.ChillFlow = round(WaterFlow1.voltage * 208.33333 - 125,2)
        self.ChillTemp = round(WaterTemp1.voltage * 58.33333 - 55,2)

class Air:
    def __init__(self):
        self.TimeStamp = datetime.datetime.now()
        self.Press = round((AirPress1.voltage * 1.524796) * 0.25 - 0.25, 2)
    def Read(self):
        self.TimeStamp = datetime.datetime.now()
        self.Press = round(AirPress1.voltage * 1.524796 * 0.25 - 0.25, 2)
    
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

    def Read(self):
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


def printData():
    BME280.Read()
    Air.Read()
    Water.Read()
    print("==========================================")
    print("Env TimeStamp   : %s" % BME280.TimeStamp)
    print("Env Temp        : %0.2f" % BME280.Temp)
    print("Env Hum         : %0.2f" % BME280.Hum)
    print("Env Press       : %0.2f" % BME280.Press)

    print("Air TimeStamp   : %s" % Air.TimeStamp)
    print("Air Press       : %0.2f" % Air.Press)

    print("Chill TimeStamp : %s" % Water.TimeStamp)
    print("Chill Flow      : %0.2f" % Water.ChillFlow)
    print("Chill Temp      : %0.2f" % Water.ChillTemp)


#setup
BME280 = BME280()
Water = Water()
Air = Air()
Accel = Accel()
DB = DB()

def task(arg1,args2):
    BME280.Read()
    Air.Read()
    Water.Read()
#    Accel.Read()

    DB.Write()
    time.sleep(1)

signal.signal(signal.SIGALRM,task)
signal.setitimer(signal.ITIMER_REAL,1,timesecond)

while True:
    printData()
    time.sleep(10)