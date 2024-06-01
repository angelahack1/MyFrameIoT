#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <FreeDefaultFonts.h>

#if defined(ESP32)
    #define SD_CS   5
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

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410
#define NAMEMATCH ""
#define PALETTEDEPTH 0
#define BMPIMAGEOFFSET 54
#define BUFFPIXEL      20

uint16_t read16(File& f);
uint32_t read32(File& f);
uint8_t showBMP(char *nm, int x, int y);
void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg, uint16_t color);
void cleanScr();
void printTextAt(char *text, int numberLine);
void wifiScan();

MCUFRIEND_kbv tft;
char namebuf[32] = "/";
File root;
int pathlen;
int lineCounter = 0;
int lineHeight = 25;

void setup() {
    uint16_t ID;
    Serial.begin(9600);
    Serial.print("Show BMP files on TFT with ID:0x");
    ID = tft.readID();
    Serial.println(ID, HEX);
    if (ID == 0x0D3D3) ID = 0x9481;
    tft.begin(ID);
    tft.fillScreen(0x001F);
    tft.setTextColor(0xFFFF, 0x0000);
    bool good = SD.begin(SD_CS);
    if (!good) {
        Serial.print(F("cannot start SD"));
        while (1);
    }
    root = SD.open(namebuf);
    pathlen = strlen(namebuf);

    wifiScan();
}

void loop() {
    char *nm = namebuf + pathlen;
    File f = root.openNextFile();
    uint8_t ret;
    uint32_t start;
    if (f != NULL) {
#ifdef USE_SDFAT
        f.getName(nm, 32 - pathlen);
#else
        strcpy(nm, (char *)f.name());
#endif
        f.close();
        strlwr(nm);
        if (strstr(nm, ".bmp") != NULL && strstr(nm, NAMEMATCH) != NULL) {
            Serial.print(namebuf);
            Serial.print(F(" - "));
            tft.fillScreen(0);
            start = millis();
            ret = showBMP(namebuf, 5, 5);
            switch (ret) {
                case 0:
                    Serial.print(millis() - start);
                    Serial.println(F("ms"));
                    break;
                case 1:
                    Serial.println(F("bad position"));
                    break;
                case 2:
                    Serial.println(F("bad BMP ID"));
                    break;
                case 3:
                    Serial.println(F("wrong number of planes"));
                    break;
                case 4:
                    Serial.println(F("unsupported BMP format"));
                    break;
                case 5:
                    Serial.println(F("unsupported palette"));
                    break;
                default:
                    Serial.println(F("unknown"));
                    break;
            }
        }
    }
    else root.rewindDirectory();

    delay(1000);
    cleanScr();
    for(int i = 1; i <= 12; i++) {
        lineCounter = i;
        char buffer1[4];
        char buffer2[20] = "Line counter: ";
        itoa(lineCounter, buffer1, 10);
        strcat(buffer2, buffer1);
        printTextAt(buffer2, lineCounter);
        delay(100);
    }
    delay(2000);
}

void wifiScan() {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    
    Serial.println("");
    WiFi.begin("AiXKare-4G", "1164Louder!#");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    if(WiFi.isConnected()) {
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>WiFi Connected!!!!!<<<<<<<<<<<<<<<<<<<<<<<<<<");
    } else {
        Serial.println("WiFi Not connected XC");
    }
}

uint16_t read16(File& f) {
    uint16_t result;         // read little-endian
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read((uint8_t*)&result, sizeof(result));
    return result;
}

void logDebug(char text) {
    Serial.print(text);
}

void cleanScr() {
    tft.fillScreen(BLACK);
}

void printTextAt(char *text, int numberLine) {
    showmsgXY(5, numberLine*lineHeight, 1, &FreeSans12pt7b, text, WHITE);
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg, uint16_t color)
{
    int16_t x1, y1;
    uint16_t wid, ht;
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(sz);
    tft.print(msg);
}

uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;
    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bmpFile.seek(bmpImageoffset - (4<<bmpDepth)); //54 for regular, diff for colorsimportant
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 32:
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (bmpDepth == 32) buffidx++; //ignore ALPHA
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
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}
