#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <time.h>
#include <IRremote.hpp>
#define IR_SEND_PIN       4
//#include "PinDefinitionsAndMore.h"

const char* ssid = "A0957F978755-2G";      // WiFiのSSID
const char* password = "ftn4y7hen4sr6h";  // WiFiのパスワード

const char* ntpServer = "pool.ntp.org";  // NTPサーバ
const long  gmtOffset_sec = 9 * 3600;    // 日本の場合、GMT+9 (日本標準時)
const int   daylightOffset_sec = 0;      // 夏時間のオフセット

/// 独自の設定を行うクラスを、LGFX_Deviceから派生して作成します。
class LGFX : public lgfx::LGFX_Device
{

lgfx::Panel_ST7789      _panel_instance;


// パネルを接続するバスの種類にあったインスタンスを用意します。
  lgfx::Bus_SPI        _bus_instance;   // SPIバスのインスタンス
// バックライト制御が可能な場合はインスタンスを用意します。(必要なければ削除)
  lgfx::Light_PWM     _light_instance;

public:
  // コンストラクタを作成し、ここで各種設定を行います。
  // クラス名を変更した場合はコンストラクタも同じ名前を指定してください。
  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // バス設定用の構造体を取得します。

// SPIバスの設定
      cfg.spi_host = VSPI_HOST;     // 使用するSPIを選択  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
      // ※ ESP-IDFバージョンアップに伴い、VSPI_HOST , HSPI_HOSTの記述は非推奨になるため、エラーが出る場合は代わりにSPI2_HOST , SPI3_HOSTを使用してください。
      cfg.spi_mode = 3;             // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 40000000;    // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 16000000;    // 受信時のSPIクロック
      cfg.spi_3wire  = true;        // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = true;        // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = SPI_DMA_CH_AUTO; // 使用するDMAチャンネルを設定 (0=DMA不使用 / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=自動設定)
      // ※ ESP-IDFバージョンアップに伴い、DMAチャンネルはSPI_DMA_CH_AUTO(自動設定)が推奨になりました。1ch,2chの指定は非推奨になります。
      cfg.pin_sclk = 18;            // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 23;            // SPIのMOSIピン番号を設定
      cfg.pin_miso = -1;            // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = 22;            // SPIのD/Cピン番号を設定  (-1 = disable)
     // SDカードと共通のSPIバスを使う場合、MISOは省略せず必ず設定してください。
//*/

      _bus_instance.config(cfg);    // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // バスをパネルにセットします。
    }

    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      cfg.pin_cs           =    -1;  // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    16;  // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // BUSYが接続されているピン番号 (-1 = disable)

      // ※ 以下の設定値はパネル毎に一般的な初期値が設定されていますので、不明な項目はコメントアウトして試してみてください。

      cfg.panel_width      =   240;  // 実際に表示可能な幅
      cfg.panel_height     =   240;  // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // パネルのY方向オフセット量
      cfg.offset_rotation  =     0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // データ読出しが可能な場合 trueに設定
      cfg.invert           = false;  // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // 16bitパラレルやSPIでデータ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)
      _panel_instance.config(cfg);
    }


    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

// 準備したクラスのインスタンスを作成します。
LGFX display;
static LGFX_Sprite sprite(&display); // スプライトを使う場合はLGFX_Spriteのインスタンスを作成。

   //  int sensorValue = analogRead(34); 
int mapAnalogRead(int value) {
    if (value >= 1000 && value < 2000) {
        return 1;
    } else if (value >= 2000 && value <= 3000) {
        return 2;
    } else if (value > 3000 && value <= 4000) {
        return 3;
    } else if (value > 4000) {
        return 4;
    } else {
        return -1; // または、適切なデフォルト値を返す
    }
}
  int BLUE  = lgfx::color888(100, 255, 0);
  int YELLOW = lgfx::color888(10, 50, 150);
void keyTest(){

      display.fillRoundRect(10,190,20,20,10,YELLOW);
      display.fillRoundRect(35,190,20,20,10,YELLOW);
      display.fillRoundRect(60,190,20,20,10,YELLOW);
      display.fillRoundRect(85,190,20,20,10,YELLOW);

      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
             if(mapAnalogRead(sensorValue)==1){
              display.fillRoundRect(10,190,20,20,10,BLUE);
  }
  else   if(mapAnalogRead(sensorValue)==2){
              display.fillRoundRect(35,190,20,20,10,BLUE); 
  }
  else   if(mapAnalogRead(sensorValue)==3){
             display.fillRoundRect(60,190,20,20,10,BLUE);
  }
  else   if(mapAnalogRead(sensorValue)==4){
             display.fillRoundRect(85,190,20,20,10,BLUE);
  }
}

void setup() {
  Serial.begin(115200);
  IrSender.begin(IR_SEND_PIN); 
    display.init();
    display.fillScreen(TFT_BLACK);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {delay(100);display.print(".");}
  display.println("Connected to WiFi");
for(int i=0; i<240; i++){display.drawGradientLine( i, 0, i, 240,lgfx::color888(20, 150, 90), lgfx::color888(70, 220, 100));}//   上,下
  // NTPサーバに接続して時刻を取得
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  display.setColorDepth(24); 
  
  for(int i=0; i<5; i++){showTime();delay(200);}
  display.setCursor(10,190);
  display.print("Click a key.." );
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
    while (true) {//ロック画面（クリックまで待つ）
        mappedValue = mapAnalogRead(sensorValue);
        if (mappedValue == 1 || mappedValue == 2 || mappedValue == 3 || mappedValue == 4) {break;}
        delay(80); 
        sensorValue = analogRead(34);
    }
    draw_home();
}


int homeCursor = 0;
int app=0;//0:home,
String homeApps[] = {
        "Lighting Remote",//1
        "Alarm Clock",//2
        "Reading",//3
        "Settings"//4
};
//水色、紫、黄色
  #define LightingRemoteColor lgfx::color888(100, 70, 150)//深緑
  #define AlarmClockColor           lgfx::color888(150, 150, 0)//水色
  #define ReadingColor                  lgfx::color888(70,100,150)//紫
  #define  SettingsColor                 lgfx::color888(200,180,170)//深緑


void loop() {
  for(;;){
    if(app==0){
    showTime();
    delay(5);
    //keyTest();
    homeMenu();
    delay(50);
    }
    else if(app==1){app_LightingRemote();}
    else if(app==2){app_AlarmClock();}
    else if(app==3){app_Reading();}
    else if(app==4){app_Settings();}
    }

}
void homeMenu(){
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
      display.fillRoundRect(10,190,20,20,3,YELLOW);
      display.fillRoundRect(35,190,20,20,3,YELLOW);
      display.fillRoundRect(60,190,20,20,3,YELLOW);
      display.fillRoundRect(85,190,20,20,3,YELLOW);
      display.fillRoundRect(25*homeCursor+10,190,20,20,3,BLUE);
      display.setTextSize(1);
      display.setCursor(65,160);
      display.print(homeApps[homeCursor]);
      delay(30);
    if(mappedValue==1 && homeCursor>0 ){homeCursor--;}//left
    if(mappedValue==2){//OK
        app=homeCursor+1;  
        if(app==1){display.setColor(LightingRemoteColor);for(int i=0; i<25; i++){display.fillCircle(25*homeCursor+20,200,i*14);}draw_LightingRemote();}
        else if(app==2){display.setColor(AlarmClockColor);for(int i=0; i<25; i++){display.fillCircle(25*homeCursor+20,200,i*14);}draw_AlarmClock();}
        else if(app==3){display.setColor(ReadingColor);for(int i=0; i<20; i++){display.fillCircle(25*homeCursor+20,200,i*14);}draw_Reading();}
        else if(app==4){display.setColor(SettingsColor);for(int i=0; i<20; i++){display.fillCircle(25*homeCursor+20,200,i*14);}draw_Settings();}
    }
    if(mappedValue==3 && homeCursor<3){homeCursor++;}//right
}
void draw_home(){
  for(int i=0; i<240; i++){display.drawGradientLine( i, 0, i, 240,lgfx::color888(110, 0, 55), lgfx::color888(250, 0, 80));}//   上,下
  display.fillRoundRect(40,60,160,120,15,lgfx::color888(120, 110, 110));
  display.setTextColor(lgfx::color888(0, 0, 0),lgfx::color888(120, 110, 110));
}

String LightingRemoteKeys[]{"K1","K2","K3"};
String LightingRemoteFuncs[]{"ON/OFF","Night","Brighter"};
void draw_LightingRemote(){
  for(int i=0; i<240; i++){display.drawGradientLine( i, 0, i, 240,lgfx::color888(80, 50, 120), lgfx::color888(100, 70, 150));}//   上,下
  for(int i=0; i<3; i++){    
    display.fillRoundRect(10, 62*i+40, 70, 56, 3,lgfx::color888(170, 90, 20));//左側のBOX
    display.fillRoundRect(85,62*i+40,140,56,3,lgfx::color888(150, 70, 10));//右側のBOX
    display.setTextColor(lgfx::color888(0, 0, 0),lgfx::color888(170, 90, 20));

    display.setTextSize(2);//以下がKey
    display.setCursor(30,60+65*i);
    display.print(LightingRemoteKeys[i]);

    display.setCursor(120,60+65*i);//以下がFuncs
    display.setTextColor(lgfx::color888(0, 0, 0),lgfx::color888(150, 70, 10));
    display.print(LightingRemoteFuncs[i]);
    }
}
void draw_AlarmClock(){display.setCursor(30,100);display.print("AlarmClock Building..");}
void draw_Reading(){display.setCursor(30,100);display.print("Reading Building..");}
void draw_Settings(){display.setCursor(30,100);display.print("Settings Building..");}

void app_LightingRemote(){
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
        if(mappedValue==4){app=0;draw_home();}
        else if(mappedValue==0){IrSender.sendOnkyo(0x1275, 0x203, 0);delay(1000);}
        else if(mappedValue==1){IrSender.sendOnkyo(0x1275, 0x207, 0);delay(1000);}
        else if(mappedValue==2){IrSender.sendOnkyo(0x1275, 0x208, 0);delay(1000);}
        delay(30);
}
void app_AlarmClock(){
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
        if(mappedValue==4){app=0;draw_home();}
        delay(30);
}
void app_Reading(){
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
        if(mappedValue==4){app=0;draw_home();}
        delay(30);
}
void app_Settings(){
      int sensorValue = analogRead(34); 
      int mappedValue = mapAnalogRead(sensorValue);
        if(mappedValue==4){app=0;draw_home();}
        delay(30);
}

void showTime(){
//飾り。BOX
display.fillRoundRect(40,60,160,120,15,lgfx::color888(120, 110, 110));
display.setTextColor(lgfx::color888(0, 0, 0),lgfx::color888(120, 110, 110));
    // 現在のUNIX時刻を取得  
  time_t now;
  time(&now);
  // 時刻をフォーマットして表示
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char timeStr[50];

  display.setTextSize(2);
  strftime(timeStr, sizeof(timeStr), "%m/%d ", &timeinfo);//日付
  display.drawString(timeStr, 60,140);

  display.setTextSize(4);
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);//時刻
  display.drawString(timeStr, 65,100);

  display.setTextSize(2);
  int dayOfWeek = timeinfo.tm_wday;
  display.drawString(" "+getDayOfWeekString(dayOfWeek),125,140);//曜日
}

// 曜日を文字列に変換する関数
String getDayOfWeekString(int dayOfWeek) {
  switch (dayOfWeek) {
    case 0:
      return "Sun";
    case 1:
      return "Mon";
    case 2:
      return "Tue";
    case 3:
      return "Wed";
    case 4:
      return "Thu";
    case 5:
      return "Fri";
    case 6:
      return "Sat";
    default:
      return "Unknown";
  }
}
