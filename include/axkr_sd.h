#ifndef AXKR_H
#define AXKR_H

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "log.h"

#define SD_CS   5   //8-R-D-U
#define SD_SCK  18  //9-R-D-U
#define SD_DI   23  //1-R-U-D
#define SD_DO   19  //6-R-U-D

File root;
int pathlen;
char namebuf[64] = "/media";
static File tempFile;


void setupSD(void) {
    LOG_S(DEBUG, __FILE__, __LINE__, "setupSD()...");
    char generalBuffer[64];

    bool good = SD.begin(SD_CS);
    if (!good) {
        Serial.print(F("cannot start SD"));
        while (1);
    }
    root = SD.open(namebuf);
    pathlen = strlen(namebuf);
    if(root) {
        strcpy(generalBuffer, "Root file: ");
        strcat(generalBuffer, root.name());
        LOG_S(INFO, __FILE__, __LINE__, generalBuffer);
    }
    LOG_S(INFO, __FILE__, __LINE__, "SD card initialized");
    LOG_S(DEBUG, __FILE__, __LINE__, "...setupSD()");
}

File openFileFromSD(char *filename) {
    File file = SD.open(filename);
    return file;
}

void closeFile(File paramFile) {
    paramFile.close();
}

File* searchForNext(char *extension) { //We make sure to find a file with the provided extension
    LOG_S(DEBUG, __FILE__, __LINE__, "searchForNext()...");
    char *nm = namebuf + pathlen;
    *nm = '/';
    nm++;
    int maxCounter = 0;
    char generalBuffer[128];

    tempFile = root.openNextFile();
    if(tempFile == NULL) {
        LOG_S(DEBUG, __FILE__, __LINE__, "Rewinding...");
        root.rewindDirectory();
        tempFile = root.openNextFile();
        LOG_S(DEBUG, __FILE__, __LINE__, "...Rewinded");
    }
    
    if(tempFile == NULL) {
        strcpy(generalBuffer, "No ");
        strcat(generalBuffer, extension);
        strcat(generalBuffer, "'s found(b)");
        LOG_S(INFO, __FILE__, __LINE__, generalBuffer);
        LOG_S(DEBUG, __FILE__, __LINE__, "...searchForNext()");
        return NULL;
    }

    strcpy(nm, (char *)tempFile.name());
    strlwr(nm);
    strcpy(generalBuffer, "Starting with file: ");
    strcat(generalBuffer, tempFile.name());
    strcat(generalBuffer, ", and looking for: ");
    strcat(generalBuffer, extension);
    strcat(generalBuffer, "'s...");
    LOG_S(INFO, __FILE__, __LINE__, generalBuffer);
    maxCounter = 0;
    while((strstr(nm, extension) == NULL) || (tempFile == NULL) ) {
        if(tempFile != NULL) {
            tempFile.close();
        }
            
        tempFile = root.openNextFile();
        if (tempFile == NULL) {
            root.rewindDirectory();
        } else {
            strcpy(nm, (char *)tempFile.name());
            strlwr(nm);
        }

        if ((maxCounter >= 1024) && (strstr(nm, extension) == NULL)) {
            strcpy(generalBuffer, "No ");
            strcat(generalBuffer, extension);
            strcat(generalBuffer, "'s found(b)");
            LOG_S(INFO, __FILE__, __LINE__, generalBuffer);
            LOG_S(DEBUG, __FILE__, __LINE__, "...searchForNext()");
            return NULL;
        } else {
            maxCounter++;
        }
    }

    if( (tempFile == NULL) || (strstr(nm, extension) == NULL)) {
        if(tempFile != NULL) {
            tempFile.close();
        }
        strcpy(generalBuffer, "No ");
        strcat(generalBuffer, extension);
        strcat(generalBuffer, "'s found(c)");
        LOG_S(DEBUG, __FILE__, __LINE__, "...searchForNext()");
        return NULL;
    }

    if(strstr(nm, extension) == NULL) {
        if(tempFile != NULL) {
            tempFile.close();
        }
        strcpy(generalBuffer, "No ");
        strcat(generalBuffer, extension);
        strcat(generalBuffer, "'s found(d)");
        LOG_S(DEBUG, __FILE__, __LINE__, "...searchForNext()");
        return NULL;
    }

    LOG_S(DEBUG, __FILE__, __LINE__, "...searchForNext()");
    return &tempFile;
}

void readFromFile2Serial(File paramFile) {
    if (paramFile) {
        while (paramFile.available()) {
            Serial.write(paramFile.read());
        }
    }
}
#endif