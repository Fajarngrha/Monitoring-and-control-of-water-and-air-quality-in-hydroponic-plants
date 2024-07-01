#include <Arduino.h>
#include <uRTCLib.h>
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>

#include <tft_display.h>

#define PIN_RELAY_1 26
#define PIN_RELAY_2 27
#define PIN_RELAY_3 32
#define PIN_RELAY_4 33

uRTCLib rtc(0x68);

unsigned long previoustxtJam = 0;
unsigned long interval = 1000;
unsigned long previousSendData = 0;
unsigned long intervalSendData = 10000;
unsigned long previousDummyData = 0;
unsigned long intervalDummyData = 60000;

float number = 0;
long suhuAirDummy, tdsDummy;
float suhuAirFloat;

int8_t indexOfA, indexOfB, indexOfC, indexOfD, indexOfE,
    indexOfF, indexOfG, indexOfH;

char c;

String dataIn, jam, menit, detik, tanggal, bulan, tahun, dataMessage,
    intensitasCahaya, suhuUdara, kelembaban, tekananUdara, suhuAir, pH, TDS,
    kecepatanAngin;

bool isNetworkAvailable = false;
bool isDeviceConnectedToServer = false;

bool pompa1 = false;
bool pompa2 = false;
bool pompa3 = false;
bool pompa4 = false;

String username = "crowd";
String password = "12345678";
String IPaddress;

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

/* ----- Parameter Thingsboard ----- */
constexpr char TOKEN[] PROGMEM = "1HnCbZeCscgYjX7JozDi";
constexpr char THINGSBOARD_SERVER[] PROGMEM = "thingsboard.cloud";
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 1883;

DynamicJsonDocument doc(1024);
StaticJsonDocument<96> action;

void receiveDataFromMain()
{
  while (Serial2.available() > 0)
  {
    c = Serial2.read();
    if (c == '\n')
    {
      break;
    }
    else
    {
      dataIn += c;
    }
  }

  if (c == '\n')
  {
    indexOfA = dataIn.indexOf("A");
    indexOfB = dataIn.indexOf("B");
    indexOfC = dataIn.indexOf("C");
    indexOfD = dataIn.indexOf("D");
    indexOfE = dataIn.indexOf("E");
    indexOfF = dataIn.indexOf("F");
    indexOfG = dataIn.indexOf("G");
    indexOfH = dataIn.indexOf("H");

    intensitasCahaya = dataIn.substring(0, indexOfA);
    suhuUdara = dataIn.substring(indexOfA + 1, indexOfB);
    kelembaban = dataIn.substring(indexOfB + 1, indexOfC);
    tekananUdara = dataIn.substring(indexOfC + 1, indexOfD);
    suhuAir = dataIn.substring(indexOfD + 1, indexOfE);
    pH = dataIn.substring(indexOfE + 1, indexOfF);
    TDS = dataIn.substring(indexOfF + 1, indexOfG);
    kecepatanAngin = dataIn.substring(indexOfG + 1, indexOfH);
  }

  c = 0;
  dataIn = "";
}

/* ------ Fungsi Reconnecting ke Thingsboard ------ */
void tbReconnect()
{
  Serial.println(F("Connecting to Thingsboard ... "));
  if (tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
  {
    isDeviceConnectedToServer = true;
    Serial.println(F("Thingsboard connected"));
  }
  else
  {
    isDeviceConnectedToServer = false;
    Serial.println(F("Thingsboard connection failed"));
  }
}

/** ----- Publish data ke server ----- **/
void publishData()
{
  if (isNetworkAvailable && isDeviceConnectedToServer)
  {
    Serial.println(F("Sending data to Thingsboard"));
    tb.sendTelemetryString("suhuair", String(suhuAirFloat).c_str());
    tb.sendTelemetryString("ph", pH.c_str());
    tb.sendTelemetryString("tds", String(tdsDummy).c_str());
    tb.sendTelemetryString("kelembaban", String(68).c_str());
    tb.sendTelemetryString("suhuudara", suhuUdara.c_str());
    tb.sendTelemetryString("tekananudara", tekananUdara.c_str());
    tb.sendTelemetryString("kecepatanangin", kecepatanAngin.c_str());
    tb.sendTelemetryString("lux", intensitasCahaya.c_str());
    tb.sendTelemetryBool("pompa1", pompa1);
    tb.sendTelemetryBool("pompa2", pompa2);
    tb.sendTelemetryBool("pompa3", pompa3);
    tb.sendTelemetryBool("pompa4", pompa4);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);
  URTCLIB_WIRE.begin();

  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  pinMode(PIN_RELAY_3, OUTPUT);
  pinMode(PIN_RELAY_4, OUTPUT);

  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_RELAY_2, HIGH);
  digitalWrite(PIN_RELAY_3, HIGH);
  digitalWrite(PIN_RELAY_4, HIGH);

  suhuAirDummy = random(280, 300);
  suhuAirFloat = suhuAirDummy / 10.0;
  tdsDummy = random(400, 650);


  WiFi.begin(username.c_str(), password.c_str());
  String dot = ".";
  for (int i = 0; i < 57; i++)
  {
    tft.setCursor(15, 35);
    dot += ".";
    tft.print(dot);
    delay(100);

    if (WiFi.status() == WL_CONNECTED)
    {
      IPaddress = WiFi.localIP().toString();
      isNetworkAvailable = true;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      isNetworkAvailable = false;
    }
    Serial.println(IPaddress);
  }

  // rtc.set(0, 48, 6, 1, 12, 3, 23);
  TFTinit();

  tft.fillScreen(PUTIH);
  tft.fillRect(0, 260, 480, 60, HITAM);
  tft.fillRect(0, 260, 410, 60, HIJAU);
  tft.fillRect(205, 50, 70, 70, BIRU);
  tft.fillSmoothRoundRect(205, 0, 70, 80, 50, PUTIH, PUTIH);
  tft.pushImage(215, 14, 50, 50, lux_icon_1);

  waterQualityDisplay();

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(HITAM);
  tft.drawString("--:--:--", 325, 275);
  tft.drawString("--/--/--", 325, 303);

  tft.pushImage(55, 260, 137, 55, x_camp_logo);
  tft.pushImage(423, 267, 45, 45, setting_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);

  display = 0;
}

void loop()
{
  

  if (display == 0)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 275 && x < 420 && y > 0 && y < 50)
      {
        airQualityDisplay();

        display = 1;
      }
    }
  }

  if (display == 1)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 0 && x < 205 && y > 0 && y < 50)
      {
        waterQualityDisplay();

        display = 0;
      }
    }
  }

  if (display == 0 || display == 1)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 215 && x < 265 && y > 14 && y < 64)
      {
        weatherInfo();

        display = 2;
      }
    }
  }

  if (display == 2)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 0 && x < 205 && y > 0 && y < 50)
      {
        tft.fillRect(0, 50, 480, 210, BIRU);
        tft.fillRect(0, 0, 480, 50, PUTIH);
        tft.fillSmoothRoundRect(205, 0, 70, 80, 50, PUTIH, PUTIH);
        tft.pushImage(215, 14, 50, 50, lux_icon_1);

        waterQualityDisplay();

        tft.drawFastHLine(479, 319, 1, HITAM);

        display = 0;
      }

      if (x > 275 && x < 420 && y > 0 && y < 50)
      {
        tft.fillRect(0, 50, 480, 210, BIRU);
        tft.fillRect(0, 0, 480, 50, PUTIH);
        tft.fillSmoothRoundRect(205, 0, 70, 80, 50, PUTIH, PUTIH);
        tft.pushImage(215, 14, 50, 50, lux_icon_1);

        airQualityDisplay();

        tft.drawFastHLine(479, 319, 1, HITAM);

        display = 1;
      }
    }
  }

  if (display == 0 || display == 1 || display == 2)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 410 && x < 480 && y > 260 && y < 320)
      {
        setPH();

        display = 3;
      }
    }
  }

  if (display == 3)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 430 && x < 480 && y > 5 && y < 45)
      {
        setTDS();

        display = 4;
      }

      if (x > 37 && x < 82 && y > 152 && y < 197)
      {
        phMin -= 0.1;

        txtPHmin.createSprite(83, 45);
        txtPHmin.fillSprite(PUTIH);
        txtPHmin.setTextColor(HITAM);
        txtPHmin.setFreeFont(&Inter_Bold10pt7b);
        txtPHmin.drawString(String(phMin), 83 / 2, 20);
        txtPHmin.pushSprite(82, 152);
        txtPHmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 165 && x < 210 && y > 152 && y < 197)
      {
        phMin += 0.1;

        txtPHmin.createSprite(83, 45);
        txtPHmin.fillSprite(PUTIH);
        txtPHmin.setTextColor(HITAM);
        txtPHmin.setFreeFont(&Inter_Bold10pt7b);
        txtPHmin.drawString(String(phMin), 83 / 2, 20);
        txtPHmin.pushSprite(82, 152);
        txtPHmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 271 && x < 336 && y > 152 && y < 197)
      {
        phMax -= 0.1;

        txtPHmin.createSprite(83, 45);
        txtPHmin.fillSprite(PUTIH);
        txtPHmin.setTextColor(HITAM);
        txtPHmin.setFreeFont(&Inter_Bold10pt7b);
        txtPHmin.drawString(String(phMax), 83 / 2, 20);
        txtPHmin.pushSprite(316, 152);
        txtPHmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 400 && x < 445 && y > 152 && y < 197)
      {
        phMax += 0.1;

        txtPHmin.createSprite(83, 45);
        txtPHmin.fillSprite(PUTIH);
        txtPHmin.setTextColor(HITAM);
        txtPHmin.setFreeFont(&Inter_Bold10pt7b);
        txtPHmin.drawString(String(phMax), 83 / 2, 20);
        txtPHmin.pushSprite(316, 152);
        txtPHmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }
    }
  }

  if (display == 4)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 10 && x < 50 && y > 5 && y < 45)
      {
        setPH();

        display = 3;
      }

      if (x > 37 && x < 82 && y > 152 && y < 197)
      {
        tdsMin -= 50;

        txtTDSmin.createSprite(83, 45);
        txtTDSmin.fillSprite(PUTIH);
        txtTDSmin.setTextColor(HITAM);
        txtTDSmin.setFreeFont(&Inter_Bold10pt7b);
        txtTDSmin.drawString(String(tdsMin), 83 / 2, (45 / 2) - 2);
        txtTDSmin.pushSprite(82, 152);
        txtTDSmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 165 && x < 210 && y > 152 && y < 197)
      {
        tdsMin += 50;

        txtTDSmin.createSprite(83, 45);
        txtTDSmin.fillSprite(PUTIH);
        txtTDSmin.setTextColor(HITAM);
        txtTDSmin.setFreeFont(&Inter_Bold10pt7b);
        txtTDSmin.drawString(String(tdsMin), 83 / 2, 20);
        txtTDSmin.pushSprite(82, 152);
        txtTDSmin.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 271 && x < 336 && y > 152 && y < 197)
      {
        tdsMax -= 50;

        txtTDSmax.createSprite(83, 45);
        txtTDSmax.fillSprite(PUTIH);
        txtTDSmax.setTextColor(HITAM);
        txtTDSmax.setFreeFont(&Inter_Bold10pt7b);
        txtTDSmax.drawString(String(tdsMax), 83 / 2, 20);
        txtTDSmax.pushSprite(316, 152);
        txtTDSmax.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }

      if (x > 400 && x < 445 && y > 152 && y < 197)
      {
        tdsMax += 50;

        txtTDSmax.createSprite(83, 45);
        txtTDSmax.fillSprite(PUTIH);
        txtTDSmax.setTextColor(HITAM);
        txtTDSmax.setFreeFont(&Inter_Bold10pt7b);
        txtTDSmax.drawString(String(tdsMax), 83 / 2, 20);
        txtTDSmax.pushSprite(316, 152);
        txtTDSmax.deleteSprite();

        tft.drawFastHLine(479, 319, 1, HITAM);
      }
    }
  }

  if (display == 3 || display == 4)
  {
    if (tft.getTouch(&x, &y))
    {
      if (x > 271 && x < 451 && y > 265 && y < 315)
      {
        tft.fillScreen(PUTIH);
        tft.fillRect(0, 260, 480, 60, HITAM);
        tft.fillRect(0, 260, 410, 60, HIJAU);
        tft.fillRect(205, 50, 70, 70, BIRU);
        tft.fillSmoothRoundRect(205, 0, 70, 80, 50, PUTIH, PUTIH);
        tft.pushImage(215, 14, 50, 50, lux_icon_1);

        waterQualityDisplay();

        tft.setFreeFont(&Inter_Bold10pt7b);
        tft.setTextColor(HITAM);
        tft.drawString("--:--:--", 325, 275);
        tft.drawString("--/--/--", 325, 303);

        tft.pushImage(55, 260, 137, 55, x_camp_logo);
        tft.pushImage(423, 267, 45, 45, setting_icon);

        tft.drawFastHLine(479, 319, 1, HITAM);

        display = 0;
      }
    }
  }

  unsigned start = millis();
  if (start - previoustxtJam >= interval)
  {
    rtc.refresh();
    receiveDataFromMain();

    jam = String(rtc.hour());
    if (jam.length() < 2)
    {
      jam = "0" + jam;
    }

    menit = String(rtc.minute());
    if (menit.length() < 2)
    {
      menit = "0" + menit;
    }

    detik = String(rtc.second());
    if (detik.length() < 2)
    {
      detik = "0" + detik;
    }

    tanggal = String(rtc.day());
    if (tanggal.length() < 2)
    {
      tanggal = "0" + tanggal;
    }

    bulan = String(rtc.month());
    if (bulan.length() < 2)
    {
      bulan = "0" + bulan;
    }

    tahun = String(rtc.year());

    if (display == 0)
    {
      txtPH.createSprite(50, 36);
      txtPH.fillSprite(BIRU);
      txtPH.setTextColor(PUTIH);
      txtPH.setFreeFont(&Inter_Bold15pt7b);
      txtPH.drawString(pH, 25, 16);
      txtPH.pushSprite(100, 69);
      txtPH.deleteSprite();

      txtSuhuAir.createSprite(70, 36);
      txtSuhuAir.fillSprite(BIRU);
      txtSuhuAir.setTextColor(PUTIH);
      txtSuhuAir.setFreeFont(&Inter_Bold15pt7b);
      txtSuhuAir.drawString(String(suhuAirFloat), 35, 16);
      txtSuhuAir.pushSprite(100, 132);
      txtSuhuAir.deleteSprite();

      txtTDS.createSprite(85, 36);
      txtTDS.fillSprite(BIRU);
      txtTDS.setTextColor(PUTIH);
      txtTDS.setFreeFont(&Inter_Bold15pt7b);
      txtTDS.drawString(String(tdsDummy), 85 / 2, 16);
      txtTDS.pushSprite(100, 194);
      txtTDS.deleteSprite();
    }

    if (display == 1)
    {
      txtKelembaban.createSprite(46, 36);
      txtKelembaban.fillSprite(BIRU);
      txtKelembaban.setTextColor(PUTIH);
      txtKelembaban.setFreeFont(&Inter_Bold15pt7b);
      txtKelembaban.drawString(kelembaban, 23, 16);
      txtKelembaban.pushSprite(301, 69);
      txtKelembaban.deleteSprite();

      txtSuhuUdara.createSprite(68, 36);
      txtSuhuUdara.fillSprite(BIRU);
      txtSuhuUdara.setTextColor(PUTIH);
      txtSuhuUdara.setFreeFont(&Inter_Bold15pt7b);
      txtSuhuUdara.drawString(suhuUdara, 34, 16);
      txtSuhuUdara.pushSprite(274, 132);
      txtSuhuUdara.deleteSprite();

      txtTekananUdara.createSprite(75, 36);
      txtTekananUdara.fillSprite(BIRU);
      txtTekananUdara.setTextColor(PUTIH);
      txtTekananUdara.setFreeFont(&Inter_Bold15pt7b);
      txtTekananUdara.drawString(tekananUdara, 75 / 2, 16);
      txtTekananUdara.pushSprite(241, 194);
      txtTekananUdara.deleteSprite();
    }

    if (display == 2)
    {
      txtIntensitasCahaya.createSprite(100, 36);
      txtIntensitasCahaya.fillSprite(BIRU);
      txtIntensitasCahaya.setTextColor(PUTIH);
      txtIntensitasCahaya.setFreeFont(&Inter_Bold15pt7b);
      txtIntensitasCahaya.drawString(intensitasCahaya, 50, 16);
      txtIntensitasCahaya.pushSprite(38, 196);
      txtIntensitasCahaya.deleteSprite();

      txtKecepatanAngin.createSprite(50, 36);
      txtKecepatanAngin.fillSprite(BIRU);
      txtKecepatanAngin.setTextColor(PUTIH);
      txtKecepatanAngin.setFreeFont(&Inter_Bold15pt7b);
      txtKecepatanAngin.drawString(kecepatanAngin, 25, 16);
      txtKecepatanAngin.pushSprite(302, 196);
      txtKecepatanAngin.deleteSprite();
    }

    if (display == 0 || display == 1 || display == 2)
    {
      txtJam.createSprite(95, 24);
      txtJam.fillSprite(HIJAU);
      txtJam.setTextColor(HITAM);
      txtJam.setFreeFont(&Inter_Bold10pt7b);
      txtJam.drawString(jam + ":" + menit + ":" + detik, 95 / 2, 10);
      txtJam.pushSprite(278, 265);
      txtJam.deleteSprite();

      txtTanggal.createSprite(96, 24);
      txtTanggal.fillSprite(HIJAU);
      txtTanggal.setTextColor(HITAM);
      txtTanggal.setFreeFont(&Inter_Bold10pt7b);
      txtTanggal.drawString(tanggal + "/" + bulan + "/" + tahun, 48, 10);
      txtTanggal.pushSprite(277, 291);
      txtTanggal.deleteSprite();

      tft.drawFastHLine(479, 319, 1, HITAM);
    }

    Serial.print("Intensitas Cahaya: ");
    Serial.print(intensitasCahaya);
    Serial.println(" lx");

    Serial.print("Suhu = ");
    Serial.print(suhuUdara);
    Serial.println(" *C");

    Serial.print("Kelembaban = ");
    Serial.print(kelembaban);
    Serial.println(" %");

    Serial.print("Tekanan Udara = ");
    Serial.print(tekananUdara);
    Serial.println(" hPa");

    Serial.println();
    previoustxtJam = start;
  }

  if (start - previousSendData >= intervalSendData)
  {
    publishData();
    previousSendData = start;
  }

  if (start - previousDummyData >= intervalDummyData)
  {
    suhuAirDummy = random(280, 300);
    suhuAirFloat = suhuAirDummy / 10.0;
    tdsDummy = random(400, 650);
    previousDummyData = start;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    IPaddress = WiFi.localIP().toString();
    isNetworkAvailable = true;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    isNetworkAvailable = false;
  }

  if (!tb.connected())
  {
    tbReconnect();
  }
  tb.loop();
}