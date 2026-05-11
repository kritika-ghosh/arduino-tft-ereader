#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

MCUFRIEND_kbv tft;

const int XP = 8, XM = A2, YP = A3, YM = 9; 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

const int chipSelect = 10;
SdFat sd;
File bookFile;
char fileNames[10][13]; 
int fileCount = 0;
int selectedIndex = 0;
bool inMenu = true;
uint32_t pageStartPos = 0; // The byte position where the current page started

#define BLACK   0x0000
#define WHITE   0xFFFF
#define PURPLE  0x780F 

void setup() {
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3 || ID == 0xFFFF || ID == 0x0000) ID = 0x9486; 
    tft.begin(ID);
    tft.setRotation(1); 
    if (!sd.begin(chipSelect, SD_SCK_MHZ(16))) {
        tft.fillScreen(BLACK);
        tft.print(F("SD Error!"));
        while (1);
    }
    loadFileList();
    showMenu();
}

void loadFileList() {
    File root = sd.open("/");
    fileCount = 0;
    while (fileCount < 10) {
        File entry = root.openNextFile();
        if (!entry) break;
        entry.getName(fileNames[fileCount], 13);
        char upperName[13];
        strcpy(upperName, fileNames[fileCount]);
        for(int i=0; upperName[i]; i++) upperName[i] = toupper(upperName[i]);
        if (!entry.isDirectory() && strstr(upperName, ".TXT")) fileCount++;
        entry.close();
    }
}

void getSaveName(char* buf) {
    strcpy(buf, fileNames[selectedIndex]);
    strcat(buf, ".SV");
}

void saveProgress() {
    char sName[16];
    getSaveName(sName);
    sd.remove(sName);
    File f = sd.open(sName, FILE_WRITE);
    if (f) {
        f.println(pageStartPos); // Save the START of the current page
        f.close();
    }
}

void openBook() {
    char sName[16];
    getSaveName(sName);
    bookFile = sd.open(fileNames[selectedIndex]);
    if (bookFile) {
        pageStartPos = 0;
        if (sd.exists(sName)) {
            File f = sd.open(sName);
            if (f) {
                pageStartPos = f.readStringUntil('\n').toInt();
                f.close();
                bookFile.seek(pageStartPos);
            }
        }
        inMenu = false;
        tft.fillScreen(BLACK);
        tft.setCursor(10, 40);
        tft.setTextSize(2);
    }
}

void updateSelection() {
    for (int i = 0; i < fileCount; i++) {
        tft.setCursor(30, 65 + (i * 35)); 
        if (i == selectedIndex) {
            tft.setTextColor(BLACK, PURPLE); 
            tft.print(F(" > "));
        } else {
            tft.setTextColor(WHITE, BLACK);
            tft.print(F("   "));
        }
        tft.println(fileNames[i]);
    }
}

void showMenu() {
    inMenu = true;
    tft.fillScreen(BLACK);
    tft.fillRect(0, 0, tft.width(), 45, PURPLE);
    tft.setCursor(55, 15);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println(F("OPEN STORY"));
    updateSelection();
    tft.fillRect(0, tft.height()-45, tft.width(), 45, PURPLE);
    tft.setCursor(75, tft.height()-30);
    tft.println(F("NEXT BOOK"));
}

void waitForTouch(bool reading) {
    if (reading) {
        tft.fillRect(0, 0, tft.width(), 30, PURPLE);
        tft.setCursor(10, 8);
        tft.setTextColor(WHITE);
        tft.setTextSize(1);
        tft.print(F("TAP TOP TO SAVE & EXIT"));
        tft.fillRect(0, tft.height()-25, tft.width(), 25, BLACK);
        tft.drawFastHLine(0, tft.height()-25, tft.width(), PURPLE);
        tft.setCursor(tft.width()-110, tft.height()-18);
        tft.print(F("TAP BOTTOM FOR NEXT >"));
    }

    while (true) {
        TSPoint p = ts.getPoint();
        pinMode(XM, OUTPUT); pinMode(YP, OUTPUT);
        pinMode(XP, OUTPUT); pinMode(YM, OUTPUT);

        if (p.z > 450 && p.z < 1000) {
            int vPos = p.x; 
            if (reading) {
                if (vPos > 500) { 
                    saveProgress(); 
                    bookFile.close();
                    showMenu();
                    return;
                } else { 
                    // Record position for the NEW page before clearing
                    pageStartPos = bookFile.position(); 
                    tft.fillScreen(BLACK);
                    tft.setCursor(10, 40); 
                    tft.setTextColor(WHITE);
                    tft.setTextSize(2);
                    return;
                }
            } else {
                if (vPos > 500) { openBook(); return; } 
                else { selectedIndex = (selectedIndex + 1) % fileCount; updateSelection(); }
            }
            while (ts.getPoint().z > 150) { pinMode(XM, OUTPUT); pinMode(YP, OUTPUT); }
            delay(400); 
        }
    }
}

void loop() {
    if (inMenu) {
        waitForTouch(false);
    } else {
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        char currentWord[21];
        int wordIdx = 0;

        while (bookFile.available()) {
            char c = bookFile.read();
            if (c == ' ' || c == '\n' || c == '\r') {
                currentWord[wordIdx] = '\0';
                int wordWidth = strlen(currentWord) * 12;
                if (tft.getCursorX() + wordWidth > tft.width() - 10) tft.println();
                if (tft.getCursorY() + 55 > tft.height()) {
                    waitForTouch(true);
                    if (inMenu) return;
                }
                tft.print(currentWord);
                if (c == ' ') tft.print(F(" "));
                if (c == '\n') tft.println();
                wordIdx = 0;
            } else if (wordIdx < 20) {
                currentWord[wordIdx++] = c;
            }
        }
        tft.setTextColor(PURPLE);
        tft.println(F("\n\n--- THE END ---"));
        char sName[16];
        getSaveName(sName);
        sd.remove(sName);
        delay(3000);
        bookFile.close();
        showMenu();
    }
}
