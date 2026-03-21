#pragma once
#ifndef SCREEN_SUBSYSTEM_H
#define SCREEN_SUBSYSTEM_H

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include "build_config.h"
#if MF_ENABLE_SCREEN_LOG
#include <Fonts/FreeSans9pt7b.h>
#endif
#include "log.h"

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410
#define PALETTEDEPTH 0
#define BUFFPIXEL      10

#if defined(ESP32)
    #define SD_CS   5
    #define SD_SCK  18
    #define SD_DI   23
    #define SD_DO   19
    #define LCD_RD  2
    #define LCD_WR  4
    #define LCD_RS 15
    #define LCD_CS 33
    #define LCD_RST 32
    #define LCD_D0 12
    #define LCD_D1 13
    #define LCD_D2 26
    #define LCD_D3 25
    #define LCD_D4 17
    #define LCD_D5 16
    #define LCD_D6 27
    #define LCD_D7 14
#else
    #define SD_CS     10
#endif
#define MAXLINES 17
#define MAXWIDTH 25
#define MAXHEIGHT 18

MCUFRIEND_kbv tft;
char namebuf[32] = "/reel";
File root;
uint8_t pathlen;

#if MF_ENABLE_SCREEN_LOG
uint8_t lineCounter = 0;
char textScreen[MAXLINES+1][MAXWIDTH+1];
bool showPrepared = false;
#endif

uint16_t read16(File& f);
uint32_t read32(File& f);
uint8_t showBMP(char *nm, int16_t x, int16_t y);
#if MF_ENABLE_SCREEN_LOG
void showPrepare(uint8_t sz, const GFXfont *f, uint16_t color);
void showmsgXYPrepared(int16_t x, int16_t y, const char *msg);
void showmsgXY(int16_t x, int16_t y, uint8_t sz, const GFXfont *f, const char *msg, uint16_t color);
#endif

void setupSD(void) {
    LOG_S(DEBUG, __FILE__, __LINE__, "setupSD()");
    if (!SD.begin(SD_CS)) {
        Serial.print(F("cannot start SD"));
        while (1);
    }
    root = SD.open(namebuf);
    pathlen = strlen(namebuf);
}

void setupScreen(void) {
    LOG_S(DEBUG, __FILE__, __LINE__, "setupScreen()");
    uint16_t ID = tft.readID();
    if (ID == 0x0D3D3) ID = 0x9481;
    char tempBuffer[30];
    sprintf(tempBuffer, "LCD ID:0x%x", ID);
    LOG_S(INFO, __FILE__, __LINE__, tempBuffer);
    tft.begin(ID);
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE, BLACK);
}

void cleanScr() {
    tft.fillScreen(BLACK);
#if MF_ENABLE_SCREEN_LOG
    lineCounter = 0;
#endif
}

#if MF_ENABLE_SCREEN_LOG
void printText(char *text) {
    char textBuffer[MAXWIDTH+1];

    if(strlen(text) > MAXWIDTH) {
        memcpy(textBuffer, text, MAXWIDTH);
        textBuffer[MAXWIDTH] = '\0';
    } else {
        strcpy(textBuffer, text);
    }

    if(!showPrepared) {
        showPrepare(1, &FreeSans9pt7b, WHITE);
        showPrepared = true;
    }

    if(lineCounter >= MAXLINES) {
        cleanScr();
        for(uint8_t i = 0; i < MAXLINES - 1; i++) {
            strcpy(textScreen[i], textScreen[i+1]);
            showmsgXYPrepared(4, (i+1)*MAXHEIGHT, textScreen[i]);
        }
        lineCounter = MAXLINES - 1;
        strcpy(textScreen[lineCounter], textBuffer);
        showmsgXYPrepared(4, (lineCounter+1)*MAXHEIGHT, textScreen[lineCounter]);
        lineCounter = MAXLINES;
    } else {
        strcpy(textScreen[lineCounter], textBuffer);
        showmsgXYPrepared(4, (lineCounter+1)*MAXHEIGHT, textScreen[lineCounter]);
        lineCounter++;
    }
}

void showmsgXY(int16_t x, int16_t y, uint8_t sz, const GFXfont *f, const char *msg, uint16_t color)
{
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(sz);
    tft.print(msg);
}

void showPrepare(uint8_t sz, const GFXfont *f, uint16_t color) {
    tft.setFont(f);
    tft.setTextColor(color);
    tft.setTextSize(sz);
}

void showmsgXYPrepared(int16_t x, int16_t y, const char *msg) {
    tft.setCursor(x, y);
    tft.print(msg);
}

void LOG_D(level lvl, const char *stringFrom, int lineNumber, const char *text) {
    if(!_debug && lvl == DEBUG)
        return;
    char outputBuffer[40] = "\0";
    static const char* const prefixes[] = {"[D] ", "[I] ", "[W] ", "[E] ", "[F] "};
    if(lvl <= FATAL) {
        strcpy(outputBuffer, prefixes[lvl]);
    } else {
        strcpy(outputBuffer, "[ ] ");
    }
    strncat(outputBuffer, text, sizeof(outputBuffer) - 5);
    (void)stringFrom;
    (void)lineNumber;
    printText(outputBuffer);
}

#endif

uint16_t read16(File& f) {
    uint16_t result;
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint8_t showBMP(char *nm, int16_t x, int16_t y)
{
    File bmpFile;
    int16_t bmpWidth, bmpHeight;
    uint8_t bmpDepth;
    uint32_t bmpImageoffset;
    uint32_t rowSize;
    uint8_t sdbuffer[3 * BUFFPIXEL];
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL];
    boolean flip = true;
    int16_t w, h, row, col;
    int16_t lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL;
    int16_t buffidx;
    uint32_t pos;
    boolean is565 = false;
    uint16_t bmpID;
    uint16_t n;
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;

    bmpFile = SD.open(nm);
    bmpID = read16(bmpFile);
    (void) read32(bmpFile);
    (void) read32(bmpFile);
    bmpImageoffset = read32(bmpFile);
    (void) read32(bmpFile);
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);
    bmpDepth = read16(bmpFile);
    pos = read32(bmpFile);
    if (bmpID != 0x4D42) ret = 2;
    else if (n != 1) ret = 3;
    else if (pos != 0 && pos != 3) ret = 4;
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5;
    else {
        bool first = true;
        is565 = (pos == 3);
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())
            w = tft.width() - x;
        if ((y + h) >= tft.height())
            h = tft.height() - y;

        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) {
            uint8_t r, g, b, *sdptr;
            int16_t lcdidx, lcdleft;
            if (flip)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) {
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer);
            }

            for (col = 0; col < w; ) {
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) {
                    uint16_t color;
                    if (buffidx >= sizeof(sdbuffer)) {
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0;
                        r = 0;
                    }
                    switch (bmpDepth) {
                        case 32:
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (bmpDepth == 32) buffidx++;
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                    }
                    lcdbuffer[lcdidx] = color;
                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }
        }
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1);
        ret = 0;
    }
    bmpFile.close();
    return ret;
}

void runReel(void) {
    uint8_t ret;
    uint32_t start;
    char *nm = namebuf + pathlen;
    *nm++ = '/';
    uint16_t maxCounter = 0;

    File f = root.openNextFile();
    if(!f)
        root.rewindDirectory();

    f = root.openNextFile();
    if(!f) {
        LOG_D(INFO, __FILE__, __LINE__, "No BMPs(a)");
        return;
    }

    strcpy(nm, (char *)f.name());
    strlwr(nm);
    LOG_D(INFO, __FILE__, __LINE__, "Srch imgs...");
    while((strstr(nm, ".bmp") == NULL) || (!f) ) {
        f = root.openNextFile();
        if (!f) {
            root.rewindDirectory();
        } else {
            strcpy(nm, (char *)f.name());
            strlwr(nm);
            break;
        }

        if (maxCounter >= 1024) {
            LOG_D(INFO, __FILE__, __LINE__, "No BMPs(b)");
            return;
        } else {
            maxCounter++;
        }
    }

    if(!f) {
        LOG_D(INFO, __FILE__, __LINE__, "No BMPs(c)");
        return;
    }

    f.close();
    tft.fillScreen(0);
    start = millis();
    cleanScr();
    LOG_D(INFO, __FILE__, __LINE__, "Rendering:");
    LOG_D(INFO, __FILE__, __LINE__, namebuf);
    delay(1000);
    cleanScr();
    ret = showBMP(namebuf, 5, 5);
    delay(3000);
    switch (ret) {
        case 0: {
            char buf[30];
            cleanScr();
            sprintf(buf, "Done %dms", (int)(millis() - start));
            LOG_D(INFO, __FILE__, __LINE__, buf);
            delay(1000);
            break;
        }
        case 1: LOG_D(INFO, __FILE__, __LINE__, "bad pos"); break;
        case 2: LOG_D(INFO, __FILE__, __LINE__, "bad ID"); break;
        case 3: LOG_D(INFO, __FILE__, __LINE__, "bad planes"); break;
        case 4: LOG_D(INFO, __FILE__, __LINE__, "bad fmt"); break;
        case 5: LOG_D(INFO, __FILE__, __LINE__, "bad pltt"); break;
        default: LOG_D(INFO, __FILE__, __LINE__, "unknown"); break;
    }
}

#endif
