// M5StickCで振動測定を行い、シリアル送信する。
// IoT RPi server用
// RPi - M5Stick USB接続
// Ver 1.0.0
#include <WiFi.h>
#include <WiFiUDP.h>
#include <M5StickC.h>
#include "BluetoothSerial.h"
#include "arduinoFFT.h"

//測定条件設定項目
#define SAMPLE_RATE 333             // サンプリング間隔(μsec) SAMPLE_RATE = 1000000(μsec)/サンプリング周波数(Hz)
#define SAMPLE_SIZE 1024            // 一回の測定のサンプリング数。　※FFTを行う際は1024推奨（2のべき乗にすること）。m5stackcの画面でグラフ確認するだけなら150推奨。

//その他設定
#define I2C_SPEED 500000             // デバイス内のI2C通信速度。仕様上は400000が最大だが、500000でも動く。
#define X0 5                        // 横軸の描画開始座標
#define MINZ 0                      // 縦軸の最小値
#define MAXZ 2000                   // 縦軸の最大値
#define MAXX 150                    // 横軸の最大値

//M5StickC IO設定
#define BTN_A_PIN 37
#define BTN_B_PIN 39
#define LED_PIN 10
#define LED_ON LOW
#define LED_OFF HIGH
#define BTN_ON LOW
#define BTN_OFF HIGH
#define N 1024


//FFT設定
double vReal[SAMPLE_SIZE];
double vImag[SAMPLE_SIZE];
const double samplingFrequency = 1000000.0 / (double)SAMPLE_RATE;
double x; //メジャー周波数

arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLE_SIZE, samplingFrequency);

float ax[SAMPLE_SIZE], ay[SAMPLE_SIZE], az[SAMPLE_SIZE];  // 加速度データの保存先
long samplingTime[SAMPLE_SIZE];                     //サンプリング周期の保存先
boolean serialMonitor = 0;                          //シリアルモニターへ出力する場合は1
boolean m5LcdMonitor  = 1;                          //M5StickCディスプレイへ出力する場合は1
boolean graphMonitor  = 0;                          //加速度グラフ描画

//WiFi設定

const char* ssid = "raspberry";
const char* password = "minoru0869553434";
IPAddress ip(192,168,4,100);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);
IPAddress DNS(192,168,4,1);

const int my_port = 5000;  //M5Stackのポート
const char *raspi_addr = "192.168.4.1";  //ラズパイのIPアドレス
const int raspi_port = 5001; //ラズパイのポート
WiFiUDP udp;
String  TxBuffer;
void setup_wifi(){

  // Wi-Fi接続開始を画面表示
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Connecting to ");
  M5.Lcd.println(ssid);

  // WiFi接続性改善のため、いったん切断
  WiFi.disconnect( true, true ); //WiFi OFF, eraseAP=true
  delay(500);

  // Wi-Fiモジュールの開始
  WiFi.config(ip,gateway,subnet,DNS);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // 通信相手との接続待ち
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    M5.Lcd.print(".");
  }
  // Wi-Fi接続完了時の画面表示
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("");
  M5.Lcd.println("WiFi connected.");
  // M5StackのIPアドレスを表示
  M5.Lcd.print("IP address: ");
  M5.Lcd.println(WiFi.localIP());
  M5.Lcd.print("Port: ");
  M5.Lcd.println(my_port);
  // UDP開始   
  udp.begin(my_port);
}

// データ送信
void Tx_Data(){
  udp.beginPacket(raspi_addr, raspi_port);
  int i = 0;
  while( TxBuffer.length() > i){
    char temp = TxBuffer.charAt(i);
    udp.write(temp);
    i++;
  }
  udp.endPacket();
//  M5.Lcd.setCursor(0, 40);
//  M5.Lcd.print("                          ");  
//  M5.Lcd.setCursor(0, 40);
//  M5.Lcd.printf("RPM = : %s",TxBuffer);  
}

// 送信タスク
void task0(void* arg){
  while (1) {
    M5.update();
  // ボタンに応じて、メッセージを変える
    if( M5.BtnA.wasPressed() == 1 ){
      RPMmesure();
      TxBuffer = String(x);
      Tx_Data();
    }
    delay(100);
  }
}

//　受信タスク
void task1(void* arg) {
  char packetBuffer[N];
  String s;
  while (1) {
    int packetSize = udp.parsePacket();
    // UDPパケットの受信処理
    if (packetSize){
      int len = udp.read(packetBuffer, packetSize);
      if (len > 0){
        packetBuffer[len] = '\0'; // end
      }
      s = packetBuffer;

    // UDPパケット受信時の画面表示
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("                      <");  
      M5.Lcd.setCursor(0, 0);
      
      M5.Lcd.println(s);
      
      if (s == "mes"){
        RPMmesure();
        TxBuffer = String(x);
        Tx_Data();
      } else {
        TxBuffer = packetBuffer;
        Tx_Data();
      }
    }
  }
}


//レジスタ書き込み用関数　ヘッダファイル変更できる方は mpu6886.hの同じ関数をpublic化して使っても可。
void I2C_Write_NBytes(uint8_t driver_Addr, uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer){
    Wire1.beginTransmission(driver_Addr);
    Wire1.write(start_Addr);
    Wire1.write(*write_Buffer);
    Wire1.endTransmission();
}

//加速度データ取得
void getAccel(){
    M5.Lcd.println("mesuring...");
    int i = 0;  
    long t = 0;
    long dT;

    t = micros();
    while (i < SAMPLE_SIZE) {
        dT = micros() - t;
        if (dT >= SAMPLE_RATE){
            t = micros();
            M5.MPU6886.getAccelData(&ax[i],&ay[i],&az[i]);  // MPU6886から加速度を取得

            ax[i] *= 1000;
            ay[i] *= 1000;
            az[i] *= 1000;
            vReal[i] = az[i];
            vImag[i] = 0;
            if (graphMonitor == 1 && i % MAXX > 1) {
                M5.Lcd.fillScreen(BLACK);  // 画面をクリア
                int y0 = map((int)(az[i - 1]), MINZ, MAXZ, M5.Lcd.height(), 0);
                int y1 = map((int)(az[i]), MINZ, MAXZ, M5.Lcd.height(), 0);
                M5.Lcd.drawLine(i % MAXX - 1 + X0, y0, i % MAXX + X0, y1, GREEN);
            }

            samplingTime[i] = dT;
            i += 1;
        }
    }
//    M5.Lcd.print(SAMPLE_SIZE);
//    M5.Lcd.println(" cycles completed.");
}

void DCRemoval(double *vData, uint16_t samples){
    double mean = 0;
    for (uint16_t i = 0; i < samples; i++){
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 0; i < samples; i++){
        vData[i] -= mean;
    }
}


void RPMSend(){
    Serial.println(x);
    M5.Lcd.print("RPM = ");
    M5.Lcd.println(x); 
    M5.Lcd.println("Data sending");
    M5.Lcd.println("finish.");
    digitalWrite(LED_PIN, LED_OFF);
    M5.Lcd.println("Wait Commnad.");
}

void AccelSend(){
    Serial.println(SAMPLE_SIZE);
    M5.Lcd.println("Data sending");
    for (int i = 0; i < SAMPLE_SIZE; i++){
        Serial.println(samplingTime[i]);
        Serial.println(ax[i]);
        Serial.println(ay[i]);
        Serial.println(az[i]);
    }
    M5.Lcd.println("finish.");
    digitalWrite(LED_PIN, LED_OFF);
}

void setup() {
    M5.begin();

//M5StickCの入出力定義
    M5.Lcd.setRotation(3);
    pinMode(BTN_A_PIN, INPUT_PULLUP);                       //ボタン、LED設定
    pinMode(BTN_B_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF);

//LCDクリア
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println("Wait Commnad.");

//通信回りの初期化、設定

    Serial.begin(115200);
    Wire1.setClock(I2C_SPEED);                              // オンボードデバイスのI2C通信速度設定
    setup_wifi();
    xTaskCreatePinnedToCore(task0,"Task0",4096,NULL,1,NULL,0);
    xTaskCreatePinnedToCore(task1,"Task1",4096,NULL,1,NULL,1);

//加速度センサ　MPU6886の測定モード設定
    M5.MPU6886.Init();                                      // MPU6886を初期化

    //unsigned char buf;
    unsigned char regdata;

    regdata = 0x00;                                         //サンプルの間引きを0に変更。
    I2C_Write_NBytes(MPU6886_ADDRESS,0x19,1,&regdata);      //SAMPLE LATE DIVIDER,デフォルトはregdata = 0x05

    regdata = 0x07;                                         //サンプリングレートアップ
    I2C_Write_NBytes(MPU6886_ADDRESS,0x1A,1,&regdata);      //CONFIG,デフォルトは 0x01

    regdata = 0x08;                                         //加速度のLPFをバイパスしてサンプリングレートアップ
    I2C_Write_NBytes(MPU6886_ADDRESS,0x1D,1,&regdata);      //ACCEL_CONFIG_2,デフォルトは0x00
}

void RPMmesure(){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Button A was Pressed. mesure RPM.");
  getAccel();
  DCRemoval(vReal,SAMPLE_SIZE);
  FFT.Windowing(FFT_WIN_TYP_HAMMING,FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();
  x = FFT.MajorPeak();
  M5.Lcd.printf("Peak : %6.2fHz",x);
  M5.Lcd.println("");
  M5.Lcd.println("Push Button A to mesure.");  
}

void loop() {
  M5.update();
//  if ( M5.BtnA.wasPressed() ) {
//    RPMmesure(); 
//  }
  delay(1000);
}
