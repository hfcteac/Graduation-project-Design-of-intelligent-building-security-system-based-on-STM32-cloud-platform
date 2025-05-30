// 调用wifi库
#define BLINKER_WIFI // 定义通过wifi网络连接
#include <ArduinoJson.h>
#include <Blinker.h> //调用Blinker库
#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h> //调用dht11驱动库
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EEPROM.h>
// 点灯科技宏定义开始
#define TEXTE_1 "tex-1"    ////文字组件1
#define TEXTE_2 "tex-2"    ////文字组件2
#define TEXTE_3 "tex-3"    ////文字组件3
#define TEXTE_4 "tex-4"    ////文字组件4
#define TEXTE_5 "tex-5"    ////文字组件5
#define TEXTE_6 "tex-sol"  ////文字组件6
#define TEXTE_7 "tex-dor"  ////文字组件7
#define Slider_2 "TempKey" /// 滑动组件
// 点灯科技宏定义结束
// 串口接收变量开始
String xdata;
char *leftover, *leftover2;
long number, numslp;
// 串口接收变量结束

//*********开始关于公众号接收信息
String uid = "79ddbee061cb4009bd6ed9f743426bba";       // 用户私钥，巴法云控制台获取
String type = "1";                                     // 1表示是预警消息，2表示设备提醒消息
String device1 = "走廊一号";                           // 设备名称
String msg1 = "检测到走廊一区域着火，立即前去查看";    // 发送的消息
String device2 = "走廊二号";                           // 设备名称
String msg2 = "检测到走廊二区域着火，立即前去查看";    // 发送的消息
String device3 = "走廊三号";                           // 设备名称
String msg3 = "检测到走廊三区域着火，立即前去查看";    // 发送的消息
String device4 = "办公一区";                           // 设备名称
String msg4 = "检测到办公一区着火，立即前去查看";      // 发送的消息
String device5 = "办公二区";                           // 设备名称
String msg5 = "检测到办公二区着火，立即前去查看";      // 发送的消息
String device6 = "太阳能异常";                         // 设备名称
String msg6 = "太阳能发电异常，请注意";                // 发送的消息
int delaytime = 0;                                     // 为了防止被设备“骚扰”，可设置贤者时间，单位是秒，如果设置了该值，在该时间内不会发消息到微信，设置为0立即推送。
String ApiUrl = "http://api.bemfa.com/api/wechat/v1/"; // 默认 api 网址
///*********结束关于公众号接收信息

//---------------修改此处""内的信息---------------------------------------------
char auth[] = "0556d2b28c44";        // 设备秘钥
const char ssid[] = "lkddx516";      // WiFi名
const char pass[] = "34dianxie1989"; // WiFi密码
//==========目标服务器网址和端口==============//
const char *host = "116.62.81.138"; // api.seniverse.com
const uint16_t port = 80;
//===============地区设置===================//
String City = "lishan";              // 城市
String My_Key = "S-kAm0lDUxT-meOe_"; // 禁止泄露
//=================变量=====================//
typedef struct
{
    int Hour;
    int Minute;
    int Second;
    int Year;
    int Month;
    int Day;
} STime; // 时间日期结构体
typedef struct
{
    int zuigaowendu;
    int zuidiwendu;
    int shidu;
    int tianqitubiao;
    int fengjiaodu;
    int dengji;

} tianqixinxi; // 高温最低温和湿度的结构体

tianqixinxi day1, day2;
STime dTime, hTime;
int OnTime = -1;           // 计数显示变量  10s时间 5s今明后天天气
bool DatFlag = true;       // 处理接收json数据的标志位
unsigned long getTime = 0; // 获取网络天气和时间  5s请求一次
String inputString = "";   // 接收到的数据
// 请求URL
String url = "/v3/weather/daily.json?key=" + My_Key + "&location=" + City + "&language=zh-Hans&unit=c&start=0&days=3";
// 请求数据
String urlDat = "key=" + My_Key + "&location=" + City + "&language=zh-Hans&unit=c&start=0&days=3";

// 定义时间和读取
int ones, tens;

HTTPClient http;

int led1 = 0;
int led2 = 0;
int led3 = 0;
int led4 = 0;
long int ddkjcontrol = 0;
int kongtiao = 26;
double solar_read;
int Tm = 0; // 发公众号计时

// 点灯科技组件对象开始
BlinkerButton Button1("btn-1");     // 1
BlinkerButton Button2("btn-2");     // 2
BlinkerButton Button3("btn-4");     // l1
BlinkerButton Button4("btn-5");     // l2.3-4自动移位，变成4-5
BlinkerButton Button7("btn-open");  ////开启all
BlinkerButton Button8("btn-close"); /// 关闭all
BlinkerButton Button9("btn-opn");   // 远程开门
BlinkerSlider Slider2(Slider_2);    // 空调温度
BlinkerText Text1(TEXTE_1);         // 区域
BlinkerText Text2(TEXTE_2);         // 区域
BlinkerText Text3(TEXTE_3);         // 区域
BlinkerText Text4(TEXTE_4);         // 区域
BlinkerText Text5(TEXTE_5);         // 区域
BlinkerText Text6(TEXTE_6);         // 太阳能
BlinkerText Text7(TEXTE_7);         // 开锁
BlinkerText Text8("slpnum");         // 太阳能显示
BlinkerNumber SOLAR("solar");
// 点灯科技组件对象结束

// 按下按键即会执行该函数
void button1_callback(const String &state) // 走廊1灯
{
    led1++;
    if (led1 > 1)
        led1 = 0;
    if (led1 == 1)
        ddkjcontrol += 10;
    else if (led1 == 0)
        ddkjcontrol -= 10;
}

void button2_callback(const String &state) // 走廊2灯
{
    led2++;
    if (led2 > 1)
        led2 = 0;
    if (led2 == 1)
        ddkjcontrol += 100;
    else if (led2 == 0)
        ddkjcontrol -= 100;
}

void button3_callback(const String &state) // 办公1灯
{
    led3++;
    if (led3 > 1)
        led3 = 0;
    if (led3 == 1)
        ddkjcontrol += 1000;
    else if (led3 == 0)
        ddkjcontrol -= 1000;
}

void button4_callback(const String &state) // 办公2灯
{
    led4++;
    if (led4 > 1)
        led4 = 0;
    if (led4 == 1)
        ddkjcontrol += 10000;
    else if (led4 == 0)
        ddkjcontrol -= 10000;
}

void button7_callback(const String &state) // 开启全部
{
    ddkjcontrol = 11110;
}

void button8_callback(const String &state) // 关闭全部
{
    ddkjcontrol = 00000;
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;
}

void button9_callback(const String &state) // 远程开门（还没写好）
{
    // 在这写开门代码*******************************************************************************************
    digitalWrite(D2, LOW);
    delay(1000);
    digitalWrite(D2, HIGH);
}

void slider2_callback(int32_t val) // 中央空调设定回调函数
{
    kongtiao = val;
}
void heartbeat() // 太阳能发电量数据上传
{
    SOLAR.print(solar_read);
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        continue;
    Blinker.delay(100);
    pinMode(D4, OUTPUT);   // 设置引脚为输出模式
    digitalWrite(D4, LOW); // 设置引脚为低电平
    Blinker.delay(2000);    // 等待 200 毫秒

    digitalWrite(D4, HIGH); // 设置引脚为高电平
    
    pinMode(D2, OUTPUT);   // 设置输出D2为APP控制
    digitalWrite(D2, HIGH); // 设置引脚为高电平，当按下触发时候是低电平
    pinMode(D1, INPUT);   // 设置输出D2为APP控制,当低电平来时，系统认为开门
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
//    while (WiFi.status() != WL_CONNECTED)
//    {
//        Blinker.delay(500);
//    }

    // 初始化blinker
    Blinker.begin(auth, ssid, pass);
    // 按键后执行button1_callback函数
    Button1.attach(button1_callback);
    // 按键后执行button2_callback函数
    Button2.attach(button2_callback);
    // 按键后执行button3_callback函数
    Button3.attach(button3_callback);
    // 按键后执行button4_callback函数
    Button4.attach(button4_callback);
    // 按键后执行button7_callback函数
    Button7.attach(button7_callback);
    // 按键后执行button8_callback函数
    Button8.attach(button8_callback);
    // 按键后执行button9_callback函数
    Button9.attach(button9_callback);
    Slider2.attach(slider2_callback);
    Blinker.attachHeartbeat(heartbeat);
    GET_Weather(); // 获取天气
    DateHandle();
}

void loop()
{
    xdata = receive();
    if (xdata != "")
    {
        char *ReceiveBuff = new char[xdata.length() + 1];
        strcpy(ReceiveBuff, xdata.c_str());
        number = strtoul(ReceiveBuff, &leftover, 10);
        numslp = strtoul(leftover, &leftover2, 10);

        if (number % 10 == 1) // xxxxx11111(1)
        {
            Text1.print("走廊1区着火");
        }
        else
            Text1.print("走廊1区安全");
        if (number / 10 % 10 == 1)
        {
            Text2.print("走廊2区着火"); // xxxxx1111(1)1
        }
        else
            Text2.print("走廊2区安全");
        if (number / 100 % 10 == 1)
        {
            Text3.print("走廊3区着火"); // xxxxx111(1)11
        }
        else
            Text3.print("走廊3区安全");
        if (number / 1000 % 10 == 1)
        {
            Text4.print("办公1区着火"); // xxxxx11(1)111
        }
        else
            Text4.print("办公1区安全");
        if (number / 10000 % 10 == 1)
        {
            Text5.print("办公2区着火"); // xxxxx1(1)1111
        }
        else
            Text5.print("办公2区安全");
        if (number / 100000 % 10 == 1)
        {
            Text6.print("太阳能异常"); // xxxx(1)11111
        }
        else
            Text6.print("太阳能正常");
        solar_read = numslp*1.0/10000;                        ////////太阳能数据上传////////////////////////////////////////////////////////////////////////////
        if (Tm++==8) // 10s
        {
        Tm=0; // 这个负责发公众号
            if (number % 10 == 1)
            {
                doHttpStick1();
            }
            if (number / 10 % 10 == 1)
            {
                doHttpStick2();
            }
            if (number / 100 % 10 == 1)
            {
                doHttpStick3();
            }
            if (number / 1000 % 10 == 1)
            {
                doHttpStick4();
            }
            if (number / 10000 % 10 == 1)
            {
                doHttpStick5();
            }
            if (number / 100000 % 10 == 1)
            {
                doHttpStick6();
            }
        }
        delete[] ReceiveBuff;
    }
    digitalWrite(D4, Blinker.connected()); // 设置引脚为低电平    
    tens = (dTime.Year / 10) % 10; // 获取十位
    ones = dTime.Year % 10; // 获取个位
    if (getTime++==15) // 10s
    {
    getTime=0;
    GET_Weather(); // 获取天气
    }
    DateHandle();  // json处理
    Serial.printf("%09d %d %d %02d%02d%02d%02d %03d%02d %03d%02d %d %d %d %d %d %d %d%d/%02d/%02d-%02d:%02d \r\n", ddkjcontrol, kongtiao,dTime.Hour,day1.tianqitubiao,day1.shidu,day2.tianqitubiao,day2.shidu,day1.fengjiaodu, day1.dengji,day2.fengjiaodu, day2.dengji,day1.zuidiwendu, day1.zuigaowendu,day2.zuidiwendu, day2.zuigaowendu,WiFi.status(),Blinker.connected(), tens, ones, dTime.Month, dTime.Day, dTime.Hour, dTime.Minute);
    //              CTR AC EC 1D1S2D2S FX  DJFX  DJ 1d 1g 2d 2g time
    //              控制 空调 环保 今天天气湿度明天天气湿度 1风向等级2风向等级 1低温 1高温 2低温 2高温 wifi状态 点灯状态 时间。
        //  Text7.print("门开了");//用于开门远程显示备用
    if(digitalRead(D1)==LOW)//远程开门显示
    {
    Text7.print("门开了");
    }
    else
    {
    Text7.print("已关门");
    }
    Text8.print(solar_read);
    Blinker.delay(500);
    Blinker.run();
}

//******微信消息推送函数********//

void doHttpStick1()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device1 + "&msg=" + msg1;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}

void doHttpStick2()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device2 + "&msg=" + msg2;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}

void doHttpStick3()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device3 + "&msg=" + msg3;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}

void doHttpStick4()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device4 + "&msg=" + msg4;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}

void doHttpStick5()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device5 + "&msg=" + msg5;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}

void doHttpStick6()
{ // 微信消息推送函数
    String postData;
    // Post Data
    postData = "uid=" + uid + "&type=" + type + "&time=" + delaytime + "&device=" + device6 + "&msg=" + msg6;
    http.begin(client, ApiUrl);                                          // Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header
    int httpCode = http.POST(postData);                                  // Send the request
    String payload = http.getString();                                   // Get the response payload
    http.end();                                                          // Close connection
}
//=======================================================================
String receive()
{
    String data = "";
    while (Serial.available())
    {
        char c = (char)Serial.read();
        data += c;
        delay(2); // 给予接收数据的时间
    }
    return data;
}
//=====================串口接收数据串================================
/**************************************************
 * 函数名称：GET_Weather
 * 函数功能：http访问获取天气数据
 * 参数说明：无
 **************************************************/
void GET_Weather()
{
        // Serial.print("connecting to ");
        if (!client.connect(host, port))
        {
            //Serial.println("服务器连接失败");
            return;
        }
        // Serial.print("Requesting URL: ");
        //  发送请求报文
        client.print(String("GET ") + url + " HTTP/1.1\r\n" + // 请求行  请求方法 ＋ 请求地址 + 协议版本
                     "Host: " + host + "\r\n" +               // 请求头部
                     "Connection: close\r\n" +                // 处理完成后断开连接
                     "\r\n" +                                 // 空行
                     urlDat);                                 // 请求数据
        delay(100);
        while (client.available()) // 接收数据
        {
            String line = client.readStringUntil('\r');
            inputString += line;
        }
        // Serial.println(inputString);
        client.stop(); // 断开与服务器连接以节约资源
        DatFlag = true;
        // Serial.println(inputString);
}
/**************************************************
 * 函数名称：DateHandle
 * 函数功能：将获取到的数据进行处理
 * 参数说明：无
 **************************************************/
void DateHandle()
{
    if (DatFlag)
    {
        DatFlag = false;
        int t = inputString.indexOf("Date:"); // 找时间
        int m = inputString.lastIndexOf("GMT");
        String inputTime = inputString.substring(t, m + 1); // 把含有时间的数据取出进行处理
        int miao = inputTime.lastIndexOf(":");
        hTime.Hour = (inputTime.substring(miao - 5, miao - 3)).toInt();
        hTime.Minute = (inputTime.substring(miao - 2, miao)).toInt();
        hTime.Second = (inputTime.substring(miao + 1, miao + 3)).toInt();
        // Serial.println(inputTime);
        int jsonBeginAt = inputString.indexOf("{"); // 判断json数据完整性
        int jsonEndAt = inputString.lastIndexOf("}");
        if (jsonBeginAt != -1 && jsonEndAt != -1)
        {
            // 净化json数据
            inputString = inputString.substring(jsonBeginAt, jsonEndAt + 1); // 取得一个完整的JSON字符串
            processMessage();                                                // 对数据进行处理
            // Serial.println(inputString);
            inputString = "";
        }
    }
}
/**************************************************
 * 函数名称：processMessage
 * 函数功能：将json数据取出
 * 参数说明：无
 **************************************************/
void processMessage()
{
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 3 * JSON_OBJECT_SIZE(13) + 760;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, inputString);
    JsonObject results_0 = doc["results"][0];
    JsonObject results_0_location = results_0["location"];
    const char *results_0_location_name = results_0_location["name"]; // "宁波"
    JsonArray results_0_daily = results_0["daily"];
    JsonObject results_0_daily_0 = results_0_daily[0];
    const char *results_0_daily_0_date = results_0_daily_0["date"];                                   // "2019-12-21"==========================今天日期
    const char *results_0_daily_0_text_day = results_0_daily_0["text_day"];                           // "中雨"=========================白天天气
    const char *results_0_daily_0_code_day = results_0_daily_0["code_day"];                           // "14"==========================图标编号
    const char *results_0_daily_0_text_night = results_0_daily_0["text_night"];                       // "中雨"=====================晚上天气
    const char *results_0_daily_0_code_night = results_0_daily_0["code_night"];                       // "14"=======================图标编号
    const char *results_0_daily_0_high = results_0_daily_0["high"];                                   // "11"===================================最高气温
    const char *results_0_daily_0_low = results_0_daily_0["low"];                                     // "9"======================================最低气温
    const char *results_0_daily_0_wind_direction = results_0_daily_0["wind_direction"];               // "东北"==============风向
    const char *results_0_daily_0_wind_direction_degree = results_0_daily_0["wind_direction_degree"]; // "45"==度数
    const char *results_0_daily_0_wind_speed = results_0_daily_0["wind_speed"];                       // "25.20"=====================速度
    const char *results_0_daily_0_wind_scale = results_0_daily_0["wind_scale"];                       // "4"=========================风标
    const char *results_0_daily_0_humidity = results_0_daily_0["humidity"];                           // "94"============================湿度

    JsonObject results_0_daily_1 = results_0_daily[1];
    const char *results_0_daily_1_date = results_0_daily_1["date"];                                   // "2019-12-22"
    const char *results_0_daily_1_text_day = results_0_daily_1["text_day"];                           // "小雨"
    const char *results_0_daily_1_code_day = results_0_daily_1["code_day"];                           // "13"
    const char *results_0_daily_1_text_night = results_0_daily_1["text_night"];                       // "阴"
    const char *results_0_daily_1_code_night = results_0_daily_1["code_night"];                       // "9"
    const char *results_0_daily_1_high = results_0_daily_1["high"];                                   // "12"
    const char *results_0_daily_1_low = results_0_daily_1["low"];                                     // "8"
    const char *results_0_daily_1_precip = results_0_daily_1["precip"];                               // ""
    const char *results_0_daily_1_wind_direction = results_0_daily_1["wind_direction"];               // "西"
    const char *results_0_daily_1_wind_direction_degree = results_0_daily_1["wind_direction_degree"]; // "270"
    const char *results_0_daily_1_wind_speed = results_0_daily_1["wind_speed"];                       // "25.20"
    const char *results_0_daily_1_wind_scale = results_0_daily_1["wind_scale"];                       // "4"
    const char *results_0_daily_1_humidity = results_0_daily_1["humidity"];                           // "91"

    const char *results_0_last_update = results_0["last_update"]; // "2019-12-21T17:23:52+08:00"

    String riqi = results_0_last_update; // 将日期取出处理
    int nian = riqi.lastIndexOf("T");
    hTime.Year = (riqi.substring(nian - 10, nian - 6)).toInt();
    hTime.Month = (riqi.substring(nian - 5, nian - 3)).toInt();
    hTime.Day = (riqi.substring(nian - 2, nian)).toInt();

    OnTime++; // 显示标志位 0,1,2显示时间 3显示今天天气 4显示明天天气 5显示后天天气
    OnTime %= 5;
    dTime.Hour = hTime.Hour + 8;
    dTime.Minute = hTime.Minute;
    dTime.Second = hTime.Second;
    dTime.Year = hTime.Year;
    dTime.Month = hTime.Month;
    dTime.Day = hTime.Day;

    day1.tianqitubiao = atoi(results_0_daily_0_code_day); // 获取今天天气信息
    day1.zuigaowendu = atoi(results_0_daily_0_high);
    day1.zuidiwendu = atoi(results_0_daily_0_low);
    day1.shidu = atoi(results_0_daily_0_humidity);
    day1.fengjiaodu = atoi(results_0_daily_0_wind_direction_degree);
    day1.dengji = atoi(results_0_daily_0_wind_scale);

    day2.tianqitubiao = atoi(results_0_daily_1_code_day);
    day2.zuigaowendu = atoi(results_0_daily_1_high);
    day2.zuidiwendu = atoi(results_0_daily_1_low);
    day2.shidu = atoi(results_0_daily_1_humidity);
    day2.fengjiaodu = atoi(results_0_daily_1_wind_direction_degree);
    day2.dengji = atoi(results_0_daily_1_wind_scale);
}
