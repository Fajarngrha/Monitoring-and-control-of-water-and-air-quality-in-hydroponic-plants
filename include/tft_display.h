#include <TFT_eSPI.h>
#include <SPI.h>

#include <bottle_icon.h>
#include <temperature_icon.h>
#include <water_icon.h>
#include <lux_icon.h>
#include <setting_icon.h>
#include <x_camp_logo.h>
#include <air_icon.h>
#include <humidity_icon.h>
#include <temp_icon.h>
#include <air_pressure_icon.h>
#include <droplet_icon.h>
#include <weather_icon.h>
#include <plus_icon.h>
#include <minus_icon.h>
#include <next_icon.h>
#include <previous_icon.h>

#define HITAM 0x0000
#define PUTIH 0xFFFF
#define BIRU 0x0176
#define HIJAU 0x0673
#define MERAH 0xF800
#define ORANGE 0xE4A3
#define CYAN 0x07FF

#define MIN 0xE920
#define LOWS 0xFC00
#define MEDIUM 0xF660
#define NORMAL 0xB6C2
#define HIGHS 0x8EE0
#define MAX 0x4DA3

#define TFT_CS 15

byte display;
uint16_t x, y;

float phMin = 6;
float phMax = 8;
int tdsMin = 950;
int tdsMax = 1500;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite txtJam = TFT_eSprite(&tft);
TFT_eSprite txtTanggal = TFT_eSprite(&tft);
TFT_eSprite txtSuhuUdara = TFT_eSprite(&tft);
TFT_eSprite txtKelembaban = TFT_eSprite(&tft);
TFT_eSprite txtTekananUdara = TFT_eSprite(&tft);
TFT_eSprite txtSuhuAir = TFT_eSprite(&tft);
TFT_eSprite txtPH = TFT_eSprite(&tft);
TFT_eSprite txtTDS = TFT_eSprite(&tft);
TFT_eSprite txtIntensitasCahaya = TFT_eSprite(&tft);
TFT_eSprite txtKecepatanAngin = TFT_eSprite(&tft);
TFT_eSprite txtPHmin = TFT_eSprite(&tft);
TFT_eSprite txtPHmax = TFT_eSprite(&tft);
TFT_eSprite txtTDSmin = TFT_eSprite(&tft);
TFT_eSprite txtTDSmax = TFT_eSprite(&tft);
// TFT_eSprite phupBar = TFT_eSprite(&tft);

void TFTinit()
{
  uint16_t calData[5] = {223, 3677, 256, 3448, 7};

  tft.init();
  tft.setRotation(1);
  tft.setTouch(calData);
  tft.setSwapBytes(true);
  tft.setTextDatum(4);

  txtJam.setTextDatum(4);
  txtTanggal.setTextDatum(4);
  txtSuhuUdara.setTextDatum(4);
  txtKelembaban.setTextDatum(4);
  txtTekananUdara.setTextDatum(4);
  txtSuhuAir.setTextDatum(4);
  txtPH.setTextDatum(4);
  txtTDS.setTextDatum(4);
  txtIntensitasCahaya.setTextDatum(4);
  txtKecepatanAngin.setTextDatum(4);
  txtPHmin.setTextDatum(4);
  txtPHmax.setTextDatum(4);
  txtTDSmin.setTextDatum(4);
  txtTDSmax.setTextDatum(4);
}

void waterQualityDisplay()
{
  tft.fillRect(0, 0, 205, 50, BIRU);
  tft.fillRect(275, 0, 205, 50, PUTIH);
  tft.fillRect(0, 50, 205, 210, BIRU);
  tft.fillRect(205, 120, 70, 140, BIRU);
  tft.fillRect(275, 50, 205, 210, BIRU);
  tft.fillSmoothCircle(51, 87, 25, HIJAU, HIJAU);
  tft.fillSmoothCircle(51, 212, 25, ORANGE, ORANGE);

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(PUTIH);
  tft.drawString("WATER QUALITY", 205 / 2, 22);
  tft.drawString("pH", 51, 85);
  tft.drawString("TDS", 51, 210);
  tft.drawString("o", 173 + (13 / 2), 130);
  tft.setTextColor(HITAM);
  tft.drawString("AIR QUALITY", 313 + (129 / 2), 22);

  tft.setFreeFont(&Inter_Bold15pt7b);
  tft.setTextColor(PUTIH);
  tft.drawString("-.-", 125, 85);
  tft.drawString("--.-    C", 100 + (109 / 2), 148);
  tft.drawString("----      ppm", 182, 210);

  tft.pushImage(215, 14, 50, 50, lux_icon_1);
  tft.pushImage(26, 125, 50, 50, temperature_icon);
  tft.pushImage(290, 80, 150, 150, water_icon);
  // tft.pushImage(14, 267, 45, 45, bottle_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);
}

void airQualityDisplay()
{
  tft.fillRect(0, 0, 205, 50, PUTIH);
  tft.fillRect(275, 0, 205, 50, BIRU);
  tft.fillRect(0, 50, 205, 210, BIRU);
  tft.fillRect(205, 120, 70, 140, BIRU);
  tft.fillRect(275, 50, 205, 210, BIRU);

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(PUTIH);
  tft.drawString("AIR QUALITY", 313 + (129 / 2), 22);
  tft.drawString("o", 348 + (13 / 2), 130);
  tft.setTextColor(HITAM);
  tft.drawString("WATER QUALITY", 205 / 2, 22);

  tft.setFreeFont(&Inter_Bold15pt7b);
  tft.setTextColor(PUTIH);
  tft.drawString("--   %", 342, 85);
  tft.drawString("--.-     C", 274 + (103 / 2), 148);
  tft.drawString("---    hpa", 315, 210);

  tft.pushImage(42, 80, 150, 150, air_icon);
  tft.pushImage(404, 62, 50, 50, humidity_icon);
  tft.pushImage(404, 125, 50, 50, temp_icon);
  tft.pushImage(394, 177, 70, 70, air_pressure_icon);
  // tft.pushImage(14, 267, 45, 45, bottle_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);
}

void weatherInfo()
{
  tft.fillRect(0, 0, 480, 50, PUTIH);
  tft.fillRect(0, 50, 480, 210, BIRU);
  tft.fillSmoothCircle(241, 74, 60, BIRU, BIRU);

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(HITAM);
  tft.drawString("AIR QUALITY", 313 + (129 / 2), 22);
  tft.drawString("WATER QUALITY", 205 / 2, 22);
  tft.setTextColor(PUTIH);
  tft.drawString("Intensitas Cahaya", 119, 176);
  tft.drawString("Kecepatan Angin", 358, 176);

  tft.setFreeFont(&Inter_Bold15pt7b);
  tft.setTextColor(PUTIH);
  tft.drawString("-----    lx", 121, 214);
  tft.drawString("-.-   m/s", 361, 214);

  tft.pushImage(190, 50, 100, 100, weather_icon);

  // tft.pushImage(14, 267, 45, 45, bottle_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);
}

void setPH()
{
  tft.fillRect(0, 0, 480, 50, PUTIH);
  tft.fillRect(0, 50, 480, 210, BIRU);
  tft.fillRect(240, 260, 240, 60, HIJAU);
  tft.fillRoundRect(33, 150, 180, 50, 25, PUTIH);
  tft.fillRoundRect(268, 150, 180, 50, 25, PUTIH);
  tft.fillRoundRect(271, 265, 180, 50, 25, HITAM);

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(HITAM);
  tft.drawString("pH CONTROL", 238, 25);
  tft.drawString(String(phMin), 123, 173);
  tft.drawString(String(phMax), 358, 173);
  tft.setTextColor(PUTIH);
  tft.drawString("Set pH range", 174 + (127/2), 107);
  tft.drawString("Min", 104 + (37/2), 137);
  tft.drawString("Max", 358, 137);
  tft.setTextColor(HIJAU);
  tft.drawString("OK", 361, 293);

  tft.pushImage(430, 5, 40, 40, next_icon);
  tft.pushImage(37, 152, 45, 45, minus_icon);
  tft.pushImage(271, 152, 45, 45, minus_icon);
  tft.pushImage(165, 152, 45, 45, plus_icon);
  tft.pushImage(400, 152, 45, 45, plus_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);
}

void setTDS()
{
  tft.fillRect(0, 0, 480, 50, PUTIH);
  tft.fillRect(0, 50, 480, 210, BIRU);
  tft.fillRect(240, 260, 240, 60, HIJAU);
  tft.fillRoundRect(33, 150, 180, 50, 25, PUTIH);
  tft.fillRoundRect(268, 150, 180, 50, 25, PUTIH);
  tft.fillRoundRect(271, 265, 180, 50, 25, HITAM);

  tft.setFreeFont(&Inter_Bold10pt7b);
  tft.setTextColor(HITAM);
  tft.drawString("TDS CONTROL", 238, 25);
  tft.drawString(String(tdsMin), 123, 173);
  tft.drawString(String(tdsMax), 358, 173);
  tft.setTextColor(PUTIH);
  tft.drawString("Set TDS range", 174 + (127/2), 107);
  tft.drawString("Min", 104 + (37/2), 137);
  tft.drawString("Max", 358, 137);
  tft.setTextColor(HIJAU);
  tft.drawString("OK", 361, 293);

  tft.pushImage(10, 5, 40, 40, previous_icon);
  tft.pushImage(37, 152, 45, 45, minus_icon);
  tft.pushImage(271, 152, 45, 45, minus_icon);
  tft.pushImage(165, 152, 45, 45, plus_icon);
  tft.pushImage(400, 152, 45, 45, plus_icon);

  tft.drawFastHLine(479, 319, 1, HITAM);
}

// void phUpLevel(int32_t tinggi, int32_t level, uint32_t warna)
// {
//   phupBar.createSprite(50, 120);
//   phupBar.fillSprite(HITAM);
//   phupBar.fillRect(0, tinggi, 50, level, warna);
//   phupBar.pushSprite(50, 93);
//   phupBar.deleteSprite();
//   tft.drawFastHLine(479, 319, 1, HITAM);
// }

// void waterLevelDisplay()
// {
//   tft.fillRect(0, 0, 480, 50, PUTIH);
//   tft.fillRect(0, 50, 480, 210, BIRU);

//   tft.setFreeFont(&Inter_Bold10pt7b);
//   tft.setTextColor(PUTIH);
//   tft.drawString("pH Up", 76, 70);
//   tft.drawString("pH Down", 141 + (89 / 2), 70);
//   tft.drawString("Nutrisi A", 255 + (85 / 2), 70);
//   tft.drawString("Nutrisi B", 408, 70);
//   tft.setTextColor(HITAM);
//   tft.drawString("WATER LEVEL MONITORING", 240, 25);

//   tft.fillRect(50, 93, 50, 120, HITAM);
//   // tft.fillRect(50, 193, 50, 20, MIN);
//   // tft.fillRect(50, 173, 50, 20, LOWS);
//   // tft.fillRect(50, 153, 50, 20, MEDIUM);
//   // tft.fillRect(50, 133, 50, 20, NORMAL);
//   // tft.fillRect(50, 113, 50, 20, HIGHS);
//   // tft.fillRect(50, 93, 50, 20, MAX);

//   tft.fillRect(161, 93, 50, 120, HITAM);
//   tft.fillRect(161, 193, 50, 20, MIN);
//   tft.fillRect(161, 173, 50, 20, LOWS);
//   tft.fillRect(161, 153, 50, 20, MEDIUM);
//   tft.fillRect(161, 133, 50, 20, NORMAL);
//   tft.fillRect(161, 113, 50, 20, HIGHS);
//   tft.fillRect(161, 93, 50, 20, MAX);

//   tft.fillRect(272, 93, 50, 120, HITAM);
//   tft.fillRect(272, 193, 50, 20, MIN);
//   tft.fillRect(272, 173, 50, 20, LOWS);
//   tft.fillRect(272, 153, 50, 20, MEDIUM);
//   tft.fillRect(272, 133, 50, 20, NORMAL);
//   tft.fillRect(272, 113, 50, 20, HIGHS);
//   tft.fillRect(272, 93, 50, 20, MAX);

//   tft.fillRect(383, 93, 50, 120, HITAM);
//   tft.fillRect(383, 193, 50, 20, MIN);
//   tft.fillRect(383, 173, 50, 20, LOWS);
//   tft.fillRect(383, 153, 50, 20, MEDIUM);
//   tft.fillRect(383, 133, 50, 20, NORMAL);
//   tft.fillRect(383, 113, 50, 20, HIGHS);
//   tft.fillRect(383, 93, 50, 20, MAX);

//   tft.setFreeFont(&Inter_Bold13pt7b);
//   tft.setTextColor(PUTIH);
//   tft.drawString("100 %", 76, 234);
//   tft.drawString("100 %", 141 + (89 / 2), 234);
//   tft.drawString("100 %", 255 + (85 / 2), 234);
//   tft.drawString("100 %", 408, 234);

//   tft.pushImage(14, 267, 45, 45, droplet_icon);

//   tft.drawFastHLine(479, 319, 1, HITAM);
// }

// // void phDownLevel(int32_t tinggi, int32_t level, uint32_t warna);
// // void nutrisiALevel(int32_t tinggi, int32_t level, uint32_t warna);
// // void nutrisiBLevel(int32_t tinggi, int32_t level, uint32_t warna);