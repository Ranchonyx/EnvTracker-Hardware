/* CODE BEGIN Header */
/*
  ******************************************************************************
  * @file           : WetterstationEmpfaengerV2.ino
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Erik Lauter.
  * All rights reserved.  
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************

  IMPORTANT INFO
  This code is written without structure, why? because I hate my future self.
  The display is a BGR that uses RGB565. That means all colors are fucked up.
  Before graphics use the following:   "tft.setSwapBytes(true);"
  But this doesn't work if you color text or shapes. Swap the color RED with BLUE!!!
  I could have built a helper function, but I didn't.
*/
/* CODE END Header */

#include <TFT_eSPI.h>  
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPping.h>
#include "Bitmap.h"

// TFT Display Setup
TFT_eSPI tft = TFT_eSPI(135, 240); // Custom resolution

// XCVRSerial
HardwareSerial LoRaSerial(2);
#define TX2_PIN 25 // TX2 Pin
#define RX2_PIN 26 // RX2 Pin

// WLAN settings
char* ssid = "Erii and Nagis Network";
char* password = "Nagii & Erii";
WiFiClient client;

// Server settings
char* serverIP = "128.140.14.67";
int serverPort = 8787;

// Serial number of the station
String stationID = "e7ce4d5e-b6f9-11ef-b475-d0ad089b010b";

// Button Pins
const int pinYesNo = 35;
const int pinNextPrev = 0;

// Page control
int currentPage = 0;
const int totalPages = 10;
bool offlineMode = false;

// Sensor data
float temp1 = 0.0, temp2 = 0.0, temp3 = 0.0;
float humidity1 = 0.0, humidity2 = 0.0;
float air_pressure = 0.0;
float pm1p0 = 0.0, pm2p5 = 0.0, pm4p0 = 0.0, pm10 = 0.0;
float voc = 0.0, nox = 0.0;
float alt = 0.0;
float ch1_voltage = 0.0, ch1_current = 0.0;
float ch2_voltage = 0.0, ch2_current = 0.0;
float ch3_voltage = 0.0, ch3_current = 0.0;


String lastData = "";

// Buffer für Graph-Daten
const int graphSize = 100; // Anzahl der gespeicherten Werte
float tempBuffer[graphSize] = {0};
float humidityBuffer[graphSize] = {0};
float airPressureBuffer[graphSize] = {0};
float pmBuffer[4][graphSize] = {0}; // Für PM1.0, PM2.5, PM4.0, PM10
float vocBuffer[graphSize] = {0};
float noxBuffer[graphSize] = {0};
float inaVoltageBuffer[3][graphSize] = {{0}}; // Für Spannungen von INA3221-Kanälen
float inaCurrentBuffer[3][graphSize] = {{0}}; // Für Ströme von INA3221-Kanälen
int graphIndex = 0;

String LoRaInfoBuffer = "";  // Globaler Puffer für Empfangsinfo

// Einstellungen für Datenbereiche (min, max) und Nullpunkt (Offset) pro Graph
struct GraphSettings {
    float minValue;  // Minimaler Wert des Datenbereichs
    float maxValue;  // Maximaler Wert des Datenbereichs
    int zeroLine;    // Position der Null-Linie in Pixeln
};

GraphSettings graphSettings[] = {
    {-40.0, 50.0, 120},     // Temperatur: Min -40, Max 50, Nullpunkt unten
    {0.0, 100.0, 120},      // Relative Feuchtigkeit: Min 0, Max 100, Nullpunkt unten
    {950.0, 1050.0, 120},   // Luftdruck: Min 950, Max 1050, Nullpunkt unten
    {0.0, 1000.0, 120},     // PM-Werte: Min 0, Max 1000, Nullpunkt unten
    {0.0, 500.0, 120},      // VOC: Min 0, Max 500, Nullpunkt unten
    {0.0, 500.0, 120},      // NOx: Min 0, Max 500, Nullpunkt unten
    {0.0, 30.0, 120},        // Spannung 1: Min 0, Max 30 V, Nullpunkt unten
    {0.0, 5.0, 120}         // Spannung 3: Min 0, Max 5 V, Nullpunkt unten
};




int scaleToGraph(float value, GraphSettings settings, int graphHeight) {
    // Skaliert den Wert in den Pixelbereich (120 = unterer Rand, 20 = oberer Rand)
    int pixelValue = map(value, settings.minValue, settings.maxValue, settings.zeroLine, settings.zeroLine - graphHeight);
    return constrain(pixelValue, 20, 120); // Begrenze auf Displaybereich
}

void drawGraph(float* buffer, int bufferSize, GraphSettings settings, uint16_t color) {
    for (int i = 1; i < bufferSize; i++) {
        int x1 = 10 + (i - 1) * 2;
        int y1 = scaleToGraph(buffer[(graphIndex + i - 1) % bufferSize], settings, 100);
        int x2 = 10 + i * 2;
        int y2 = scaleToGraph(buffer[(graphIndex + i) % bufferSize], settings, 100);
        tft.drawLine(x1, y1, x2, y2, color);
    }
}

void setupLoRa() {
    // Initialisierung des LoRa E5-Moduls
    LoRaSerial.println("AT+MODE=TEST"); // Wechsel in den Testmodus (P2P)
    delay(500);
    LoRaSerial.println("AT+TEST=RFCFG,868,SF12,125,12,15,14,ON,OFF,OFF"); // Frequenz und Parameter einstellen
    delay(500);
    LoRaSerial.println("AT+TEST=RXLRPKT"); // Empfang aktivieren
    delay(500);

    if (LoRaSerial.available()) {
        String response = LoRaSerial.readString();
        Serial.println("LoRa init response: " + response);
    } else {
        Serial.println("Fehler bei der Initialisierung des LoRa-Moduls.");
    }
}

///////////////////////////////////////////////////////////////// Parse Data /////////////////////////////////////////////////////////////////

String cleanHexData(const String& rawData) {


    // Überprüfen, ob der String das erwartete Präfix enthält
    int startIndex = rawData.indexOf("\"");  // Start des Hex-Datenbereichs
    int endIndex = rawData.lastIndexOf("\""); // Ende des Hex-Datenbereichs

    if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
        // Extrahiere den Hex-Datenbereich
        String hexData = rawData.substring(startIndex + 1, endIndex);
//        Serial.println("HexAfterCleanHexData: " + hexData);
        return hexData;
    } else {
    LoRaInfoBuffer = rawData;  // Empfangsinfo in den globalen Puffer schreiben
//    Serial.println("Empfangsinfo: " + rawData);  // Ausgabe der Empfangsinfo
        return ""; // Gib einen leeren String zurück, falls das Format fehlerhaft ist
    }
}

void ConvertTextToHex(const char *text, char *hex_buffer, size_t buffer_size) {
    size_t length = strlen(text);
    for (size_t i = 0; i < length && (i * 2) < buffer_size - 1; i++) {
        snprintf(&hex_buffer[i * 2], 3, "%02X", (unsigned char)text[i]);
    }
}

void ConvertHexToText(const char *hex, char *text_buffer, size_t buffer_size) {
    size_t hex_length = strlen(hex);
    size_t text_length = hex_length / 2;

    if (text_length >= buffer_size) {
        text_length = buffer_size - 1;
    }

    for (size_t i = 0; i < text_length; i++) {
        char byte_string[3] = { hex[i * 2], hex[i * 2 + 1], '\0' };
        text_buffer[i] = (char)strtol(byte_string, nullptr, 16);
    }
    text_buffer[text_length] = '\0';
}

void handleLoRaData() {
    if (LoRaSerial.available()) {
        // Empfange die Rohdaten
        String rawHexData = LoRaSerial.readStringUntil('\n');
//        Serial.println("Empfangene Hex-Daten: " + rawHexData);

        // Bereinige die Rohdaten
        String cleanedHexData = cleanHexData(rawHexData);
        if (cleanedHexData.isEmpty()) {
            Serial.println("Keine gültigen Hex-Daten gefunden.");
            return;
        }

        // Konvertiere Hex-Daten in lesbaren Text
        char text_buffer[256] = {0}; // Puffer für den konvertierten Text
        ConvertHexToText(cleanedHexData.c_str(), text_buffer, sizeof(text_buffer));
//        Serial.println("Formatierte Daten (ASCII): " + String(text_buffer));

        // Analysiere die konvertierten Daten
        parseData(String(text_buffer));
    }
}



void parseData(const String& data) {
//    printf("Incoming data: %s\n", data.c_str()); // Debug

    // Find sensor data
    int hp20xIndex = data.indexOf("HP20x:");
    int sen55Index = data.indexOf("SEN55:");
    int sht45Index = data.indexOf("SHT45:");
    int ina3221Index = data.indexOf("INA3221:");

    if (hp20xIndex != -1) {
        int nextIndex = data.indexOf(";", hp20xIndex);
        String hp20xData = data.substring(hp20xIndex + 6, nextIndex);

        // Extract HP20x data
        sscanf(hp20xData.c_str(), "%f,%f,%f", &temp3, &air_pressure, &alt);
//        printf("HP20x -> Temp3: %.2f, Air pressure: %.2f, Alt: %.2f\n", temp3, air_pressure, alt);
    }

    if (sen55Index != -1) {
        int nextIndex = data.indexOf(";", sen55Index);
        String sen55Data = data.substring(sen55Index + 6, nextIndex);

        // Extract SEN55 data
        sscanf(sen55Data.c_str(), "%f,%f,%f,%f,%f,%f,%f,%f",
               &pm1p0, &pm2p5, &pm4p0, &pm10, &humidity2, &temp2, &voc, &nox);
//        printf("SEN55 -> PM1.0: %.2f, PM2.5: %.2f, PM4.0: %.2f, PM10: %.2f, Temp2: %.2f, Humidity2: %.2f, VOC: %.2f, NOx: %.2f\n",pm1p0, pm2p5, pm4p0, pm10, temp2, humidity2, voc, nox);
    }

    if (sht45Index != -1) {
        int nextIndex = data.indexOf(";", sht45Index);
        String sht45Data = data.substring(sht45Index + 6, nextIndex);

        // Extract SHT45 data
        sscanf(sht45Data.c_str(), "%f,%f", &temp1, &humidity1);
//        printf("SHT45 -> Temp1: %.2f, Humidity1: %.2f\n", temp1, humidity1);
    }

    if (ina3221Index != -1) {
        int nextIndex = data.indexOf(";", ina3221Index);
        String ina3221Data = data.substring(ina3221Index + 8, nextIndex);

        // Extract INA3221 data
        sscanf(ina3221Data.c_str(), "%f,%f,%f,%f,%f,%f",
               &ch1_voltage, &ch1_current, &ch2_voltage, &ch2_current, &ch3_voltage, &ch3_current);
//        printf("INA3221 -> CH1 Voltage: %.3f V, CH1 Current: %.3f A\n", ch1_voltage, ch1_current);
//        printf("INA3221 -> CH2 Voltage: %.3f V, CH2 Current: %.3f A\n", ch2_voltage, ch2_current);
//        printf("INA3221 -> CH3 Voltage: %.3f V, CH3 Current: %.3f A\n", ch3_voltage, ch3_current);
    }
/*
    // Debug output of the final values
    printf("Final values:\n");
    printf("Temp1: %.2f, Temp2: %.2f, Temp3: %.2f\n", temp1, temp2, temp3);
    printf("Humidity1: %.2f, Humidity2: %.2f\n", humidity1, humidity2);
    printf("Air Pressure: %.2f\n", air_pressure);
    printf("PM1.0: %.2f, PM2.5: %.2f, PM4.0: %.2f, PM10: %.2f\n", pm1p0, pm2p5, pm4p0, pm10);
    printf("VOC: %.2f, NOx: %.2f\n", voc, nox);
    printf("INA3221 -> CH1 Voltage: %.3f V, CH1 Current: %.3f A\n", ch1_voltage, ch1_current);
    printf("INA3221 -> CH2 Voltage: %.3f V, CH2 Current: %.3f A\n", ch2_voltage, ch2_current);
    printf("INA3221 -> CH3 Voltage: %.3f V, CH3 Current: %.3f A\n", ch3_voltage, ch3_current);
*/    
}
 
///////////////////////////////////////////////////////////////// Pages /////////////////////////////////////////////////////////////////

void displayPage() {

#define PM1_COLOR    TFT_RED
#define PM2_COLOR    TFT_BLUE
#define PM4_COLOR    TFT_GREEN
#define PM10_COLOR   TFT_ORANGE

  tft.fillScreen(0x3165);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  if (offlineMode) {
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_RED, 0x3165); 
    tft.setTextSize(1);
    tft.drawString("Offline", 190, tft.height() - 125);
    tft.setTextColor(TFT_WHITE, 0x3165); 
  }

  int y = 0;
  if (currentPage == 0) {
    // Page 1
    float avgTemp = (temp1 + temp2 + temp3) / 3;
    float avghumidity = (humidity1 + humidity2) / 2;
    y += 10;
    tft.drawString("Main info:", 0, y);
    y += 20;
    tft.setTextSize(2);
    tft.drawString("Temp: " + String(avgTemp, 1) + " C", 0, y);
    y += 20;
    tft.drawString("RH:   " + String(avghumidity, 1) + " %", 0, y);
    y += 20;
    tft.drawString("AirP: " + String(air_pressure, 1) + " hPa", 0, y);
    tft.setTextSize(1);
  } else if (currentPage == 1) {
    // Page 2
    y = 10;
    tft.drawString("Air quality:", 0, y);
    y += 10;
    tft.setTextSize(2);
    tft.drawString("PM1.0: " + String(pm1p0, 1), 0, y);
    y += 20;
    tft.drawString("PM2.5: " + String(pm2p5, 1), 0, y);
    y += 20;
    tft.drawString("PM4.0: " + String(pm4p0, 1), 0, y);
    y += 20;
    tft.drawString("PM10:  " + String(pm10, 1), 0, y);
    y += 20;
    tft.drawString("VOC:   " + String(voc, 1), 0, y);
    y += 20;
    tft.drawString("NOx:   " + String(nox, 1), 0, y);
    tft.setTextSize(1);
  } else if (currentPage == 2) {
    // Page 3
    y += 10;
    tft.drawString("All info:", 0, y);
    y += 10;
    tft.setTextSize(1);
    tft.drawString("Temp1: " + String(temp1, 1), 0, y);
    y += 10;
    tft.drawString("Temp2: " + String(temp2, 1), 0, y);
    y += 10;
    tft.drawString("Temp3: " + String(temp3, 1), 0, y);
    y += 10;
    tft.drawString("RH1:   " + String(humidity1, 1), 0, y);
    y += 10;
    tft.drawString("RH2:   " + String(humidity2, 1), 0, y);
    y += 10;
    tft.drawString("AirP:  " + String(air_pressure, 1), 0, y);
    y += 10;
    tft.drawString("PM1.0: " + String(pm1p0, 1), 0, y);
    y += 10;
    tft.drawString("PM2.5: " + String(pm2p5, 1), 0, y);
    y += 10;
    tft.drawString("PM4.0: " + String(pm4p0, 1), 0, y);
    y += 10;
    tft.drawString("PM10:  " + String(pm10, 1), 0, y);
    y = 10;
    y += 10;
    tft.drawString("VOC:   " + String(voc, 1), 100, y);
    y += 10;
    tft.drawString("NOx:   " + String(nox, 1), 100, y);
    y += 10;
    tft.drawString("Voltage1: " + String(ch1_voltage, 1), 100, y);
    y += 10;
    tft.drawString("Current1: " + String(ch1_current, 1), 100, y);
    y += 10;
    tft.drawString("Voltage2: " + String(ch2_voltage, 1), 100, y);
    y += 10;
    tft.drawString("Current2:  " + String(ch2_current, 1), 100, y);
    y += 10;
    tft.drawString("Voltage3:   " + String(ch3_voltage, 1), 100, y);
    y += 10;
    tft.drawString("Current3:   " + String(ch3_current, 1), 100, y);
  
  } else if (currentPage == 3) {
    // Temperatur
    tft.drawString("50", 225, 25);
    tft.drawString("42", 225, 35);
    tft.drawString("25", 225, 45);
    tft.drawString("25", 225, 55);
    tft.drawString("17", 225, 65);
    tft.drawString("9", 225, 75);
    tft.drawString("1", 230, 85);
    tft.drawString("-7", 220, 95);
    tft.drawString("-15", 220, 105);
    tft.drawString("-23", 220, 115);
    tft.drawString("-31", 220, 125);
    tft.drawString("Temperature (C):", 10, 10);
//    tft.drawLine(10, 79, 230, 79, TFT_WHITE); // X-Achse
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);  // Y-Achse
    drawGraph(tempBuffer, graphSize, graphSettings[0], TFT_RED);
  } else if (currentPage == 4) {
    // Relative Feuchtigkeit
    tft.drawString("100", 220, 25);
    tft.drawString("90", 225, 35);
    tft.drawString("80", 225, 45);
    tft.drawString("70", 225, 55);
    tft.drawString("60", 225, 65);
    tft.drawString("50", 225, 75);
    tft.drawString("40", 225, 85);
    tft.drawString("30", 225, 95);
    tft.drawString("20", 225, 105);
    tft.drawString("10", 225, 115);
    tft.drawString("0", 230, 125);
    tft.drawString("Humidity (RH%):", 10, 10);
//    tft.drawLine(10, 120, 230, 120, TFT_WHITE);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(humidityBuffer, graphSize, graphSettings[1], TFT_BLUE);
  } else if (currentPage == 5) {
    // Luftdruck
    tft.drawString("1050", 215, 25);
    tft.drawString("1040", 215, 35);
    tft.drawString("1030", 215, 45);
    tft.drawString("1020", 215, 55);
    tft.drawString("1010", 215, 65);
    tft.drawString("1000", 215, 75);
    tft.drawString("990", 220, 85);
    tft.drawString("980", 220, 95);
    tft.drawString("970", 220, 105);
    tft.drawString("960", 220, 115);
    tft.drawString("950", 220, 125);
    tft.drawString("Air Pressure (hPa):", 10, 10);
//    tft.drawLine(10, 120, 230, 120, TFT_WHITE);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(airPressureBuffer, graphSize, graphSettings[2], TFT_GREEN);
  } else if (currentPage == 6) {
    // PM-Werte
    tft.drawString("1k", 225, 25);
    tft.drawString("900", 220, 35);
    tft.drawString("800", 220, 45);
    tft.drawString("700", 220, 55);
    tft.drawString("600", 220, 65);
    tft.drawString("500", 220, 75);
    tft.drawString("400", 220, 85);
    tft.drawString("300", 220, 95);
    tft.drawString("200", 220, 105);
    tft.drawString("100", 220, 115);
    tft.drawString("0", 230, 125);
    tft.drawString("(\u00b5g/m\u00b3)", 140, 10);
//    tft.drawLine(10, 120, 230, 120, TFT_WHITE);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
const char* pmLabels[] = {"PM1.0", "PM2.5", "PM4.0", "PM10"};
uint16_t pmColors[] = {TFT_RED, TFT_BLUE, TFT_GREEN, TFT_ORANGE,};

for (int i = 0; i < 4; i++) {
    tft.setTextColor(pmColors[i]);
    tft.drawString(pmLabels[i], 10 + i * 33, 10);
    drawGraph(pmBuffer[i], graphSize, graphSettings[3], pmColors[i]);
}
  tft.setTextColor(TFT_WHITE, 0x3165); 
  } else if (currentPage == 7) {
    // VOC
    tft.fillRect(10, 20, 220, 33, 0x6000);  //GREY-RED
    tft.fillRect(10, 53, 220, 33, 0x6320);  //GREY-YELLOW
    tft.fillRect(10, 86, 220, 33, 0x0320); //GREY-GREEN
    tft.drawString("500", 220, 25);
    tft.drawString("450", 220, 35);
    tft.drawString("400", 220, 45);
    tft.drawString("350", 220, 55);
    tft.drawString("300", 220, 65);
    tft.drawString("250", 220, 75);
    tft.drawString("200", 220, 85);
    tft.drawString("150", 220, 95);
    tft.drawString("100", 220, 105);
    tft.drawString("50", 225, 115);
    tft.drawString("0", 230, 125);
    tft.drawString("VOC-Index:", 10, 10);
//    tft.drawLine(10, 120, 230, 120, TFT_WHITE);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(vocBuffer, graphSize, graphSettings[4], TFT_PINK);
  } else if (currentPage == 8) {
    // NOx
    tft.fillRect(10, 20, 220, 33, 0x6000);  //GREY-RED
    tft.fillRect(10, 53, 220, 33, 0x6320);  //GREY-YELLOW
    tft.fillRect(10, 86, 220, 33, 0x0320); //GREY-GREEN
    tft.drawString("500", 220, 25);
    tft.drawString("450", 220, 35);
    tft.drawString("400", 220, 45);
    tft.drawString("350", 220, 55);
    tft.drawString("300", 220, 65);
    tft.drawString("250", 220, 75);
    tft.drawString("200", 220, 85);
    tft.drawString("150", 220, 95);
    tft.drawString("100", 220, 105);
    tft.drawString("50", 225, 115);
    tft.drawString("0", 230, 125);
    tft.drawString("NOx-Index:", 10, 10);
//    tft.drawLine(10, 120, 230, 120, TFT_WHITE);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(noxBuffer, graphSize, graphSettings[5], TFT_YELLOW);
  
  } else if (currentPage == 9) {
    // Spannung 1
    tft.drawString("Voltage 1 (V):", 10, 10);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(inaVoltageBuffer[0], graphSize, graphSettings[6], TFT_BLUE);

  } else if (currentPage == 10) {
    // Spannung 3
    tft.drawString("Voltage 3 (V):", 10, 10);
    tft.drawLine(10, 20, 10, 120, TFT_WHITE);
    drawGraph(inaVoltageBuffer[2], graphSize, graphSettings[7], TFT_GREEN);

  } else if (currentPage == 11) {
    // LoRa Empfangsinfo
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(1);
    tft.drawString("LoRa Info:", 0, 0);

    tft.setTextColor(TFT_GREEN);
    tft.drawString("Empfangsinfo:", 0, 20);
    tft.drawString(LoRaInfoBuffer, 0, 40);  // Zeigt die komplette Empfangsinfo aus dem Puffer

//    char text_buffer[256] = {0};
//    ConvertHexToText(cleanHexData(LoRaInfoBuffer).c_str(), text_buffer, sizeof(text_buffer));

//    tft.setTextColor(TFT_WHITE);
//    tft.drawString("Decoded Text:", 0, 60);
//    tft.drawString(String(text_buffer), 0, 80);  // Zeigt den dekodierten Text an
  }
}

///////////////////////////////////////////////////////////////// Offline Mode /////////////////////////////////////////////////////////////////

void askOfflineMode() {
  tft.fillScreen(0x3165);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  tft.drawString("Offline mode?", tft.width() / 2, tft.height() / 2);
//  tft.drawString("NO / YES ", tft.width() / 2, tft.height() / 2 + 20);
// NEW
tft.setTextColor(TFT_RED, 0x3165); 
tft.drawString("NO", tft.width() / 2 - 30, tft.height() / 2 + 20);
tft.setTextColor(TFT_GREEN, 0x3165); 
tft.drawString("YES", tft.width() / 2 + 10, tft.height() / 2 + 20);
//NEW END
  Serial.println("Offline mode?");
  Serial.println("Y/N");
  while (true) {
    if (digitalRead(pinYesNo) == LOW || (Serial.available() > 0 && Serial.read() == 'Y')) {
      offlineMode = true;
      tft.fillScreen(0x3165);
      tft.drawString("offline mode", tft.width() / 2, tft.height() / 2);
      Serial.println("Offline mode activated");
      delay(500);
      return;
    }
    if (digitalRead(pinNextPrev) == LOW || (Serial.available() > 0 && Serial.read() == 'N')) {
      offlineMode = false;
      tft.fillScreen(0x3165);
      tft.drawString("reconnecting", tft.width() / 2, tft.height() / 2);
      Serial.println("Retry to connect");
      delay(500);
      setup();
      return;
    }
  }
}

///////////////////////////////////////////////////////////////// TCP /////////////////////////////////////////////////////////////////

bool SendTCPData(const String& serverIP, int serverPort, const String& stationID, const String& data) {
  // Verbindungsaufbau zum Server
  if (!client.connect(serverIP.c_str(), serverPort)) {
    Serial.println("Error connecting to server");
    return false;
  }

  Serial.println("Connected to server!");

  // HELO-Phase
  client.printf("HELO %s\n", stationID.c_str());
  String response = client.readStringUntil('\n');
  Serial.printf("Server response: %s \n", response.c_str());

  if (response != "ACK") {
    Serial.println("Error at HELO-ACK");
    client.stop();
    return false;
  }

 Serial.println("ACK");

  // Datenübertragung
  client.printf("SIZE %u\n", data.length());
  if (client.readStringUntil('\n') != "ACK") {
    Serial.println("Error at SIZE-ACK");
    client.stop();
    return false;
  }

  Serial.println("Transmiting...");

  client.printf("DATA %s\n", data.c_str());
  if (client.readStringUntil('\n') != "ACK") {
    Serial.println("Error during data transfer");
    client.stop();
    return false;
  }

  Serial.println("Transmit Done");

  // Abschlussphase
  client.println("FINI\n");
  Serial.println("Data transferred successfully");
  client.stop();
  return true;
}


///////////////////////////////////////////////////////////////// Console /////////////////////////////////////////////////////////////////

void handleConsoleCommands() {
  static String inputBuffer = ""; // Buffer for input

  // Check if data is available on Serial
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    if (receivedChar == '\n') { 
      inputBuffer.trim();

      if (inputBuffer.equalsIgnoreCase("HELP")) {
        Serial.println("Available commands:");
        Serial.println("  HELP - Show this help message");
        Serial.println("  SHOW SETUP - Show's Current Setup");
        Serial.println("  SETUP WIFI - Set WiFi SSID and password");
        Serial.println("  SETUP SERVER - Set server IP and port");
        Serial.println("  SETUP STATION - Set station ID");
      } else if (inputBuffer.equalsIgnoreCase("SHOW SETUP")) {
        Serial.println("Current Setup:");
        Serial.printf("Station ID: %s\n", stationID);
        Serial.printf("WIFI SSID: %s\n", ssid);
        Serial.print("WIFI Password: ");
        for (size_t i = 0; i < strlen(password); i++) {
          Serial.print('*');
        }
        Serial.println();
        if (!offlineMode) {
          Serial.printf("Server IP: %s\n", serverIP);
          Serial.printf("Server Port: %d\n", serverPort);
        } else {
          Serial.println("Server IP: N/A Offline");
          Serial.println("Server Port: N/A Offline");
        }
      } else if (inputBuffer.equalsIgnoreCase("SETUP WIFI")) {
        Serial.println("Enter WiFi SSID:");
        String newSSID = waitForInput();
        Serial.println("Enter WiFi password:");
        String newPassword = waitForInput();

        ssid = strdup(newSSID.c_str());
        password = strdup(newPassword.c_str());
        Serial.println("WiFi settings updated. Reboot to apply changes.");
      } else if (inputBuffer.equalsIgnoreCase("SETUP SERVER")) {
        Serial.println("Enter server IP:");
        String newIP = waitForInput();
        Serial.println("Enter server port:");
        String newPort = waitForInput();

        serverIP = strdup(newIP.c_str());
        serverPort = newPort.toInt();
        Serial.println("Server settings updated. Reboot to apply changes.");
      } else if (inputBuffer.equalsIgnoreCase("SETUP STATION")) {
        Serial.println("Enter station ID:");
        String newStationID = waitForInput();
        stationID = strdup(newStationID.c_str());
        Serial.println("Station ID updated. Reboot to apply changes.");
      } else {
        Serial.println("Unknown command. Type 'HELP' for a list of commands.");
      }

      inputBuffer = ""; // Reset buffer
    } else {
      inputBuffer += receivedChar; 
    }
  }
}

// Wait for input
String waitForInput() {
  String input = "";
  while (true) {
    while (Serial.available() > 0) {
      char receivedChar = Serial.read();
      if (receivedChar == '\n') {
        input.trim();
        return input;
      } else {
        input += receivedChar;
      }
    }
    delay(10); 
  }
}

///////////////////////////////////////////////////////////////// Setup /////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  LoRaSerial.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN); // LoRa UART konfigurieren
  
  // Configure button pins
  pinMode(pinYesNo, INPUT_PULLUP);
  pinMode(pinNextPrev, INPUT_PULLUP);

  // Turn on backlight
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // TFT display initialization
  tft.init();
  tft.setRotation(1); // Landscape 1 Portrait 0
  tft.fillScreen(0x3165);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 135, logo_bitmap);  
  delay(1000);
  tft.setTextColor(TFT_WHITE, 0x3165); 
  tft.setTextDatum(MC_DATUM); // Center text
  tft.setTextSize(1);
  tft.drawString("Connecting...", tft.width() / 2, tft.height() - 8); 

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  delay(50);
  setupLoRa(); // LoRa-Modul initialisieren

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed");
    delay(500);
    askOfflineMode();
    return;
  }

  // Perform a ping test to the server
  if (Ping.ping(serverIP)) {
//    Serial.printf("Server reachable! Response time: min=%dms, avg=%.2fms, max=%dms\n",Ping.minTime(), Ping.averageTime(), Ping.maxTime());
  } else {
    Serial.println("Server not reachable. Switching to offline mode.");
    delay(500);
    askOfflineMode();
  }


}

///////////////////////////////////////////////////////////////// Main /////////////////////////////////////////////////////////////////

void loop() {
  // Konsolenbefehle abfangen und ausführen
  handleConsoleCommands();

  // LoRa-Daten verarbeiten
  if (LoRaSerial.available() > 0) {
    // Empfange die Rohdaten
    String rawHexData = LoRaSerial.readStringUntil('\n');

    // Bereinige die Hex-Daten
    String cleanedHexData = cleanHexData(rawHexData);
    if (cleanedHexData.isEmpty()) {
      Serial.println("Keine gültigen Hex-Daten gefunden.");
      return;
    }

    // Speichere die Empfangsinfo für die Anzeige
    lastData = "Empfangsinfo: " + rawHexData;

    // Konvertiere Hex in lesbaren Text
    char text_buffer[256] = {0}; // Puffer für ASCII-Daten
    ConvertHexToText(cleanedHexData.c_str(), text_buffer, sizeof(text_buffer));
    String formattedData = String(text_buffer);

    // Daten analysieren und verarbeiten
    parseData(formattedData);

    // Neue Werte in den Buffer speichern
    tempBuffer[graphIndex] = (temp1 + temp2 + temp3) / 3;
    humidityBuffer[graphIndex] = (humidity1 + humidity2) / 2;
    airPressureBuffer[graphIndex] = air_pressure;
    pmBuffer[0][graphIndex] = pm1p0;
    pmBuffer[1][graphIndex] = pm2p5;
    pmBuffer[2][graphIndex] = pm4p0;
    pmBuffer[3][graphIndex] = pm10;
    vocBuffer[graphIndex] = voc;
    noxBuffer[graphIndex] = nox;
    inaVoltageBuffer[0][graphIndex] = ch1_voltage;
    inaCurrentBuffer[0][graphIndex] = ch1_current;
    inaVoltageBuffer[1][graphIndex] = ch2_voltage;
    inaCurrentBuffer[1][graphIndex] = ch2_current;
    inaVoltageBuffer[2][graphIndex] = ch3_voltage;
    inaCurrentBuffer[2][graphIndex] = ch3_current;

    // Zyklischen Index aktualisieren
    graphIndex = (graphIndex + 1) % graphSize;

    // Display aktualisieren
    displayPage();

    // Daten über das Netzwerk senden (nur im Online-Modus)
    if (!offlineMode && !SendTCPData(serverIP, serverPort, stationID, formattedData)) {
      Serial.println("Fehler beim Senden der Daten");
    }
  }

  // Seitensteuerung (Buttons)
  if (digitalRead(pinYesNo) == LOW) {
    currentPage = (currentPage + 1) % (totalPages + 3); // Inklusive neuer Seiten
    displayPage();
    delay(200);
  }

  if (digitalRead(pinNextPrev) == LOW) {
    currentPage = (currentPage - 1 + totalPages + 3) % (totalPages + 3);
    displayPage();
    delay(200);
  }
}

