// M5Stack Basic + Unit MPU6886
// 
// ver 2.0.0 sd　保存、最大ファイル数720

#define M5STACK_MPU6886 
#include <M5Stack.h>
#include "arduinoFFT.h"

int FileNo = 0;

//測定条件設定項目
#define MAX_SAMPLE_SIZE 1024      // 最大のサンプリング数は1024とする
#define MAX_SAMPLE_FREQ 3000      // 最大の測定周波数は3000Hzとする。
#define MAX_SAMPLE_NUM 720        // 測定回数。最大はとりあえず720回にしておく　5sec間隔=1h　60sec間隔＝12hours　3600sec間隔=30days
#define MAX_SAMPLE_INTERVAL 86400 // サンプリング間隔(sec) 3s/5s/10s/30s/1m/5m/10m/30m/1h/2h/6h/12h/24h

//その他設定
#define I2C_SPEED 500000             // デバイス内のI2C通信速度。仕様上は400000が最大だが、500000でも動く。
#define X0 5                        // 横軸の描画開始座標
#define MINZ 0                      // 縦軸の最小値
#define MAXZ 2000                   // 縦軸の最大値 defo 2000
#define MAXX 320                    // 横軸の最大値
#define MAXY 240                    //

int SAMPLE_FREQ = 3000;            // サンプリング周波数 100/200/500/1000/2000/3000 サンプリング間隔(μsec)はSAMPLE_RATE = 1000000(μsec)/サンプリング周波数(Hz)
int SAMPLE_SIZE = 1024;            // 一回の測定のサンプリング数。64/128/256/512/1024　※2のべき乗にすること。defo 
int SAMPLE_NUM = 1;               // 測定回数。最大はとりあえず720回にしておく　5sec間隔=1h　60sec間隔＝12hours　3600sec間隔=30days
int SAMPLE_INTERVAL = 1;          // サンプリング間隔(sec) 1s/3s/5s/10s/30s/1m/5m/10m/30m/1h/2h/6h/12h/24h

//FFT設定
double vReal[MAX_SAMPLE_SIZE];
double vImag[MAX_SAMPLE_SIZE];
double accel[MAX_SAMPLE_SIZE];
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLE_SIZE, SAMPLE_FREQ);  // FFTオブジェクトを作る

int _yOffset = 20;
int _height = 180;
int _width = 320;
float dmax = 10000.0;

float ax[MAX_SAMPLE_SIZE], ay[MAX_SAMPLE_SIZE], az[MAX_SAMPLE_SIZE];  // 加速度データの保存先
long samplingTime[MAX_SAMPLE_SIZE];         //サンプリング周期の保存先
int mesureNum = 0;                                  //測定済み回数
boolean serialMoniter = 0;                          //シリアルモニターへ出力する場合は1

int freq_set[6] = {100,200,500,1000,2000,3000};
int size_set[5] = {64,128,256,512,1024};
//int size_set[5] = {1024,1024,1024,1024,1024};
int num_set[19] = {1,2,3,4,5,6,10,12,15,20,30,60,90,120,150,180,240,360,720};
int interval_set[14] = {1,3,5,10,30,60,300,600,1800,3600,7200,21600,43200,86400};
int freq_no = 5;
int size_no = 4;
int num_no = 0;
int interval_no = 0;

void setting1(){
    M5.Lcd.fillScreen(BLACK);  // 画面をクリア
    int i;
    int line = 1;
    int MENU = 2;
    M5.Lcd.setTextColor(WHITE,RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillRect(20,224,84,16,RED);
    M5.Lcd.setCursor(58,224);
    M5.Lcd.print("<");
    M5.Lcd.fillRect(118,224,84,16,RED);
    M5.Lcd.setCursor(136,224);
    M5.Lcd.print("NEXT");
    M5.Lcd.fillRect(216,224,84,16,RED);
    M5.Lcd.setCursor(250,224);
    M5.Lcd.print(">");
    M5.Lcd.setTextColor(GREEN,BLACK);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setCursor(0,24);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("                    [Parameters]\n");
    M5.Lcd.printf("                     Frequency             : %5dHz\n",SAMPLE_FREQ);
    M5.Lcd.printf("                     Number of Samples     : %5d\n",SAMPLE_SIZE);
    M5.Lcd.printf("                     Number of Measurement : %5d\n",SAMPLE_NUM);
    M5.Lcd.printf("                     Sampling Interval     : %5dsec\n",SAMPLE_INTERVAL);
    M5.Lcd.setCursor(162,72);
    M5.Lcd.print("[MENU]/[TOP]/[MEASURE]");

    M5.Lcd.setTextColor(BLACK,GREEN);
    M5.Lcd.setCursor(270,32);
    M5.Lcd.printf("%5d",SAMPLE_FREQ);

    
    while(i == 0){
       
        M5.update();
        if (M5.BtnB.wasPressed()){
            if (line == 1) {
                M5.Lcd.setTextColor(GREEN,BLACK);
                M5.Lcd.setCursor(270,32);
                M5.Lcd.printf("%5d",SAMPLE_FREQ);
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,40);
                M5.Lcd.printf("%5d",SAMPLE_SIZE); 
            }
            if (line == 2) {
                M5.Lcd.setTextColor(GREEN,BLACK);
                M5.Lcd.setCursor(270,40);
                M5.Lcd.printf("%5d",SAMPLE_SIZE);
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,48);
                M5.Lcd.printf("%5d",SAMPLE_NUM); 
            }
            if (line == 3) {
                M5.Lcd.setTextColor(GREEN,BLACK);
                M5.Lcd.setCursor(270,48);
                M5.Lcd.printf("%5d",SAMPLE_NUM);
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,56);
                M5.Lcd.printf("%5d",SAMPLE_INTERVAL); 
            }
            if (line == 4) {
                M5.Lcd.setTextColor(GREEN,BLACK);
                M5.Lcd.setCursor(270,56);
                M5.Lcd.printf("%5d",SAMPLE_INTERVAL);
                M5.Lcd.setTextColor(BLACK,GREEN);
                if (MENU == 1) {
                    M5.Lcd.setCursor(168,72);
                    M5.Lcd.print("MENU");
                }
                if (MENU == 2) {
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                }
                if (MENU == 3) {
                    M5.Lcd.setCursor(246,72);
                    M5.Lcd.print("MEASURE");
                }

            }
            if (line == 5) {
                M5.Lcd.setTextColor(GREEN,BLACK);
                if (MENU == 1) {
                    //M5.Lcd.setCursor(168,72);
                    //M5.Lcd.print("MENU");
                    menu1();
                }
                if (MENU == 2) {
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                }
                if (MENU == 3) {
                    //M5.Lcd.setCursor(246,72);
                    //M5.Lcd.print("MEASURE");
                    getAccel();
                }
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,32);
                M5.Lcd.printf("%5d",SAMPLE_FREQ); 
            }
            line += 1;
            if(line > 5) line = 1;
        }
        if (M5.BtnA.wasPressed()){
            if (line == 1) {
                freq_no -= 1;
                if (freq_no > 5 ) freq_no = 0;
                if (freq_no < 0 ) freq_no = 5;
                SAMPLE_FREQ = freq_set[freq_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,32);
                M5.Lcd.printf("%5d",SAMPLE_FREQ);
            }
            if (line == 2) {
                size_no -= 1;
                if (size_no > 4 ) size_no = 0;
                if (size_no < 0 ) size_no = 4;
                SAMPLE_SIZE = size_set[size_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,40);
                M5.Lcd.printf("%5d",SAMPLE_SIZE);
            }
            if (line == 3) {
                num_no -= 1;
                if (num_no > 18 ) num_no = 0;
                if (num_no < 0 ) num_no = 18;
                SAMPLE_NUM = num_set[num_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,48);
                M5.Lcd.printf("%5d",SAMPLE_NUM);
            }
            if (line == 4) {
                interval_no -= 1;
                if (interval_no > 13 ) interval_no = 0;
                if (interval_no < 0 ) interval_no = 13;
                SAMPLE_INTERVAL = interval_set[interval_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,56);
                M5.Lcd.printf("%5d",SAMPLE_INTERVAL);
               }
            if (line == 5) {
                MENU -= 1;
                if (MENU > 3) MENU = 1;
                if (MENU < 1) MENU = 3;

                if (MENU == 1) {
                    M5.Lcd.setTextColor(BLACK,GREEN);
                    M5.Lcd.setCursor(168,72);
                    M5.Lcd.print("MENU");
                    M5.Lcd.setTextColor(GREEN,BLACK);
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                }
                if (MENU == 2) {
                    M5.Lcd.setTextColor(BLACK,GREEN);                  
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                    M5.Lcd.setTextColor(GREEN,BLACK);                    
                    M5.Lcd.setCursor(246,72);
                    M5.Lcd.print("MEASURE");
                }
                if (MENU == 3) {
                    M5.Lcd.setTextColor(BLACK,GREEN);
                    M5.Lcd.setCursor(246,72);
                    M5.Lcd.print("MEASURE");
                    M5.Lcd.setTextColor(GREEN,BLACK);
                    M5.Lcd.setCursor(168,72);
                    M5.Lcd.print("MENU");
                    
                }
            }
        }
        if (M5.BtnC.wasPressed()){
            if (line == 1) {
                freq_no += 1;
                if (freq_no > 5 ) freq_no = 0;
                if (freq_no < 0 ) freq_no = 5;
                SAMPLE_FREQ = freq_set[freq_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,32);
                M5.Lcd.printf("%5d",SAMPLE_FREQ);
            }
            if (line == 2) {
                size_no += 1;
                if (size_no > 4 ) size_no = 0;
                if (size_no < 0 ) size_no = 4;
                SAMPLE_SIZE = size_set[size_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,40);
                M5.Lcd.printf("%5d",SAMPLE_SIZE);
            }
            if (line == 3) {
                num_no += 1;
                if (num_no > 18 ) num_no = 0;
                if (num_no < 0 ) num_no = 18;
                SAMPLE_NUM = num_set[num_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,48);
                M5.Lcd.printf("%5d",SAMPLE_NUM);
            }
            if (line == 4) {
                interval_no += 1;
                if (interval_no > 13 ) interval_no = 0;
                if (interval_no < 0 ) interval_no = 13;
                SAMPLE_INTERVAL = interval_set[interval_no];
                M5.Lcd.setTextColor(BLACK,GREEN);
                M5.Lcd.setCursor(270,56);
                M5.Lcd.printf("%5d",SAMPLE_INTERVAL);
               }
            if (line == 5) {
                MENU += 1;
                if (MENU > 3) MENU = 1;
                if (MENU < 1) MENU = 3;

                if (MENU == 1) {
                    M5.Lcd.setTextColor(BLACK,GREEN);
                    M5.Lcd.setCursor(168,72);
                    M5.Lcd.print("MENU");
                    M5.Lcd.setTextColor(GREEN,BLACK);                    
                    M5.Lcd.setCursor(246,72);
                    M5.Lcd.print("MEASURE");
                }
                if (MENU == 2) {
                    M5.Lcd.setTextColor(BLACK,GREEN);                  
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                    M5.Lcd.setTextColor(GREEN,BLACK);
                    M5.Lcd.setCursor(168,72);
                    M5.Lcd.print("MENU");
                }
                if (MENU == 3) {
                    M5.Lcd.setTextColor(BLACK,GREEN);
                    M5.Lcd.setCursor(246,72);
                    M5.Lcd.print("MEASURE");
                    M5.Lcd.setTextColor(GREEN,BLACK);
                    M5.Lcd.setCursor(210,72);
                    M5.Lcd.print("TOP");
                }
            }
        }
    }
}



//レジスタ書き込み用関数　ヘッダファイル変更できる方は mpu6886.hの同じ関数をpublic化して使っても可。
void I2C_Write_NBytes(uint8_t driver_Addr, uint8_t start_Addr, uint8_t number_Bytes, uint8_t *write_Buffer) {
  M5.I2C.writeBytes(driver_Addr, start_Addr, write_Buffer, number_Bytes);
}

void DrawFFTChart() {
    int band_width = 1;
    for (int band = 0; band < SAMPLE_SIZE /2 ; band++) {
        int hpos = band * band_width;
        float d = vReal[band];
        if (d > dmax) d = dmax;
        int h = (int)((d / dmax) * (_height));
        if ( band < 320){
            M5.Lcd.drawFastVLine(band, _height + _yOffset - h, h, WHITE);
            if ((band % 40) == 0) {
                M5.Lcd.drawFastVLine(hpos, _height + _yOffset,5,WHITE);
                M5.Lcd.drawFastHLine(0,_height+_yOffset,320,WHITE);
                M5.Lcd.setCursor(hpos, _height + _yOffset + 5);
                M5.Lcd.printf("%d",(int)((band * 1.0 * SAMPLE_FREQ) / SAMPLE_SIZE));
                M5.Lcd.setCursor(280, _height + _yOffset + 13);
                M5.Lcd.printf("[Hz]",WHITE);
            }
        }
    }
}

void DrawAccelChart(){
    for (int i = 1 ; i < SAMPLE_SIZE ; i++ ) {
        int y0 = map((int)(accel[i - 1]), MINZ, MAXZ, M5.Lcd.height()/2, 0);
        int y1 = map((int)(accel[i]), MINZ, MAXZ, M5.Lcd.height()/2, 0);
        M5.Lcd.drawLine(i % MAXX - 1 + X0, y0, i % MAXX + X0, y1, YELLOW);
    }
}

void DCRemoval(double *vData, uint16_t samples) {
    double mean = 0;
    for (uint16_t i = 0; i < samples; i++) {
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 0; i < samples; i++) {
        vData[i] -= mean;
    }
}

//加速度データ取得
void getAccel(){
    long t = 0;
    long dT;
    double Freq = 0.0;

    M5.Lcd.fillScreen(BLACK);  // 画面をクリア
    menu2();
    int i = 0;
    while(i == 0){

        M5.update();
        if (M5.BtnA.wasPressed()){
            menu1();
        }
        
        if (M5.BtnB.wasPressed()){
            setting1();
        }
        
        if (M5.BtnC.wasPressed()){
            m5.Lcd.setTextColor(GREEN);       
            M5.Lcd.setCursor(218,0);
            M5.Lcd.printf("%5d times left.",SAMPLE_NUM);
//            timer(3);
            int j = 0;
            while (j < SAMPLE_NUM) {
                M5.Lcd.fillScreen(BLACK);  // 画面をクリア
                menu2();
                t = micros();
                Freq = 0.0;
                int i = 0;
                while (i < SAMPLE_SIZE) {
                    dT = micros() - t;
                    if (dT >= (1000000 / SAMPLE_FREQ)){
                        t = micros();
                        M5.IMU.getAccelData(&ax[i],&ay[i],&az[i]);  // MPU6886から加速度を取得

                        ax[i] *= 1000;
                        ay[i] *= 1000;
                        az[i] *= 1000;
                        vReal[i] = az[i];
                        accel[i] = az[i];
                        vImag[i] = 0; 
                        samplingTime[i] = dT;
                        Freq = Freq + dT;
                        i += 1;
                    }
                }
                DCRemoval(vReal,SAMPLE_SIZE);
                DCRemoval(accel,SAMPLE_SIZE);
                Freq = SAMPLE_SIZE / (Freq / 1000000) ;  

//FFT部結果表示
                FFT.Windowing(vReal,SAMPLE_SIZE,FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
                FFT.Compute(vReal,vImag,SAMPLE_SIZE,FFT_FORWARD); // FFT処理(複素数で計算)
                FFT.ComplexToMagnitude(vReal,vImag,SAMPLE_SIZE); // 複素数を実数に変換
                double x = FFT.MajorPeak(vReal,SAMPLE_SIZE,SAMPLE_FREQ);
                double y = (double)SAMPLE_FREQ/SAMPLE_SIZE;
                M5.Lcd.setTextColor(WHITE);
                DrawFFTChart();
                M5.Lcd.setCursor(0, _height + _yOffset + 13);
                M5.Lcd.printf("Peak: %.2fHz  /  Resolution = %.2fHz  ", x,y);

//加速度センサー値表示部分
                m5.Lcd.setTextColor(YELLOW);
                DrawAccelChart();
                M5.Lcd.setCursor(0,8);
                M5.Lcd.printf("Actual Frequency = %.2fHz",Freq);

//残り回数、タイマーカウントダウン表示
                j += 1;
                m5.Lcd.setTextColor(GREEN,BLACK);       
                M5.Lcd.setCursor(218,0);
                M5.Lcd.printf("%5d times left.",SAMPLE_NUM - j);

                if (j < SAMPLE_NUM){
                    timer(SAMPLE_INTERVAL);         
                }
            }
            int i = 0;
            delay(100);
        }
    }
}

void menu2(){
  
    M5.Lcd.setCursor(0,24);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf("                    [Parameters]\n");
    M5.Lcd.printf("                     Frequency             : %5dHz\n",SAMPLE_FREQ);
    M5.Lcd.printf("                     Number of Samples     : %5d\n",SAMPLE_SIZE);
    M5.Lcd.printf("                     Number of Measurement : %5d\n",SAMPLE_NUM);
    M5.Lcd.printf("                     Sampling Interval     : %5dsec\n",SAMPLE_INTERVAL);

    M5.Lcd.setTextColor(WHITE,RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillRect(20,224,84,16,RED);
    M5.Lcd.setCursor(40,224);
    M5.Lcd.print("MENU");
    M5.Lcd.fillRect(118,224,84,16,RED);
    M5.Lcd.setCursor(118,224);
    M5.Lcd.print("SETTING");
    M5.Lcd.fillRect(216,224,84,16,RED);
    M5.Lcd.setCursor(216,224);
    M5.Lcd.print("MEASURE");
    M5.Lcd.setTextColor(GREEN,BLACK);
    M5.Lcd.setTextSize(1);
}

void timer(int sec){
    int i = 0;
//    double remainSec;
//    char remainSecStr[10];
    while( i < sec ){

        M5.Lcd.fillRect(0,0,99,8,BLACK);
        M5.Lcd.setCursor(0,0);
        M5.Lcd.printf("%5d sec before.",sec - i);
//        remainSec = sec -i ;
//        dtostrf(remainSec,5,0,remainSecStr);
//        M5.Lcd.setCursor(0,0);
//        M5.Lcd.print(remainSecStr);
//        M5.Lcd.print(" sec before.");
        delay(1000);
        i += 1;
    }
}

//シリアルモニターへデータ出力
void Serialout(){

//    M5.Lcd.setCursor(0,70);
//    M5.Lcd.print("Data sending... ");
    for (int i = 0; i < SAMPLE_SIZE; i++){
        Serial.print("測定周期(μsec)：");
        Serial.print(samplingTime[i]);
//        Serial.print(" , x軸方向加速度(mG)：");
//        Serial.print(ax[i]);
//        Serial.print(" , y軸方向加速度(mG)：");
//        Serial.print(ay[i]);
//        Serial.print(" , Z軸方向加速度(mG)：");
//        Serial.print(az[i]);
        Serial.print(vReal[i]);
        Serial.print(" , サンプル番号：");
        Serial.println(i);  
    }
    Serial.println(SAMPLE_SIZE);
//    M5.Lcd.print("completed.");
}

//メニュー画面表示
void menu1(){
    int i = 0;
    M5.Lcd.fillScreen(BLACK);  // 画面をクリア
    while (i == 0){
        M5.update();
        M5.Lcd.setCursor(0,2);
        M5.Lcd.print("Number of mesurements : ");
        M5.Lcd.println(mesureNum);
        M5.Lcd.println("");
        M5.Lcd.println("Button A : Back to MENU.");
        M5.Lcd.println("Button B : Setting Conditions.");
        M5.Lcd.println("Button C : Start Mesurment.");
        if (M5.BtnA.wasPressed()){
            M5.Lcd.setCursor(50,50);
            M5.Lcd.println("Button A was pressed");
            delay(1000);
            M5.Lcd.fillRect(50,50,120,8,BLACK);
        }
        
        if (M5.BtnB.wasPressed()){
            setting1();
        }
        
        if (M5.BtnC.wasPressed()){
            getAccel();
        }
        delay(100);
    }
}


void setup() {
    M5.begin();
    M5.Power.begin();

//micro SD 初期化
    if (!SD.begin()) {
        M5.Lcd.println("Card failed, or not present");
        while (1);
    }
    Serial.println("TF card initialized.");

//通信回りの初期化、設定
    Wire.setClock(I2C_SPEED);                              // オンボードデバイスのI2C通信速度設定

//画面初期化
    M5.Lcd.setRotation(1);
    M5.Lcd.clear(BLACK);
    M5.Lcd.setTextColor(GREEN);

//加速度センサ　MPU6886の測定モード設定
    M5.IMU.Init();                                      // MPU6886を初期化

    unsigned char buf;
    unsigned char regdata;

    regdata = 0x00;                                         //サンプルの間引きを0に変更。
    I2C_Write_NBytes(MPU6886_ADDRESS,0x19,1,&regdata);      //SAMPLE LATE DIVIDER,デフォルトはregdata = 0x05

    regdata = 0x07;                                         //サンプリングレートアップ
    I2C_Write_NBytes(MPU6886_ADDRESS,0x1A,1,&regdata);      //CONFIG,デフォルトは 0x01

    regdata = 0x08;                                         //加速度のLPFをバイパスしてサンプリングレートアップ
    I2C_Write_NBytes(MPU6886_ADDRESS,0x1D,1,&regdata);      //ACCEL_CONFIG_2,デフォルトは0x00
    menu1();
}

void loop() {
    if (M5.BtnC.wasPressed()){
        getAccel();
    }
    delay(1000);
    M5.update();
}
