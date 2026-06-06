#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

const int chipSelect = 10;
const int analogButtonPin = A5; 

SdFat sd;
File bookFile;

char fileNames[12][13]; 
int fileCount = 0;
int selectedIndex = 0;
bool inMenu = true;
uint32_t pageStartPos = 0; 

// Backward tracking history array stack
uint32_t pageHistory[25];
int historyIndex = 0;

#define BTN_NONE  0
#define BTN_MENU  1
#define BTN_PREV  2
#define BTN_NEXT  3

// 16-bit True Dark Academia Color Palette
#define BLACK            0x0000
#define WHITE            0xFFFF
#define ACADEMIA_BROWN   0x7142  // Rich chocolate mahogany leather
#define SPINE_SHADOW     0x3040  // Dark contrast shadow
#define ANTIQUE_GOLD     0xDE64  // Muted vintage gold filigree
#define AGED_PAGES       0xF60B  // Warm parchment tan paper edges
#define RIBBON_RED       0x9800  // Deep crimson bookmark ribbon

void setup() {
    uint16_t ID = tft.readID();
    if (ID == 0xD3D3 || ID == 0xFFFF || ID == 0x0000) ID = 0x9486; 
    tft.begin(ID);
    tft.setRotation(1); 
    
    if (!sd.begin(chipSelect, SD_SCK_MHZ(16))) {
        tft.fillScreen(BLACK);
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        tft.print(F("SD Error!"));
        while (1);
    }
    loadFileList();
    showMenu();
}

void loadFileList() {
    File root = sd.open("/");
    fileCount = 0;
    while (fileCount < 12) {
        File entry = root.openNextFile();
        if (!entry) break;
        
        char tempName[13];
        entry.getName(tempName, 13);
        
        char upperName[13];
        strcpy(upperName, tempName);
        for(int i=0; upperName[i]; i++) upperName[i] = toupper(upperName[i]);
        
        if (!entry.isDirectory() && strstr(upperName, ".TXT") && !strstr(upperName, ".SV")) {
            strcpy(fileNames[fileCount], tempName);
            fileCount++;
        }
        entry.close();
    }
}

int readPhysicalButtons() {
    int reading1 = analogRead(analogButtonPin);
    delayMicroseconds(50);
    int reading2 = analogRead(analogButtonPin);
    
    if (abs(reading1 - reading2) > 15) return BTN_NONE;
    int analogVal = (reading1 + reading2) / 2;
    
    if (analogVal > 950) return BTN_NONE;  
    if (analogVal < 100) return BTN_MENU;  
    if (analogVal > 450 && analogVal < 570) return BTN_PREV;  
    if (analogVal > 620 && analogVal < 740) return BTN_NEXT;  
    
    return BTN_NONE;
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
        f.println(historyIndex);
        for (int i = 0; i <= historyIndex; i++) {
            f.println(pageHistory[i]);
        }
        f.close();
    }
}

void openBook() {
    char sName[16];
    getSaveName(sName);
    bookFile = sd.open(fileNames[selectedIndex]);
    if (bookFile) {
        pageStartPos = 0;
        historyIndex = 0;
        pageHistory[0] = 0;
        
        if (sd.exists(sName)) {
            File f = sd.open(sName);
            if (f) {
                if (f.available()) {
                    int tempIndex = f.parseInt();
                    
                    if (tempIndex >= 0 && tempIndex < 25) {
                        historyIndex = tempIndex;
                        for (int i = 0; i <= historyIndex; i++) {
                            if (f.available()) {
                                pageHistory[i] = f.parseInt();
                            }
                        }
                    } else {
                        historyIndex = 0;
                        pageHistory[0] = 0;
                    }
                }
                f.close();
                pageStartPos = pageHistory[historyIndex];
                bookFile.seek(pageStartPos);
            }
        }
        inMenu = false;
    }
}

void updateSelection() {
    // Smoothly clear the center bounding box array zone to eliminate flicker
    tft.fillRect(60, 30, 200, 210, BLACK);

    int bookX = (tft.width() - 86) / 2; // Centers the book completely at X = 117
    int bookY = 45;

    // 1. Layer: Aged Paper Edge Extensions
    tft.fillRect(bookX + 80, bookY + 4, 6, 86, AGED_PAGES);  
    tft.fillRect(bookX + 4, bookY + 90, 82, 8, AGED_PAGES);  
    tft.drawFastHLine(bookX + 8, bookY + 93, 74, BLACK);
    tft.drawFastVLine(bookX + 83, bookY + 8, 80, BLACK);

    // 2. Layer: Crimson Bookmark Ribbon Drop
    tft.fillRect(bookX + 38, bookY + 96, 10, 14, RIBBON_RED);

    // 3. Layer: Main Leather Book Cover
    tft.fillRect(bookX, bookY, 80, 90, ACADEMIA_BROWN);

    // 4. Layer: Spine Compression Shadow
    tft.fillRect(bookX, bookY, 8, 90, SPINE_SHADOW);

    // 5. Layer: Antique Gold Linings
    tft.drawRect(bookX + 12, bookY + 6, 62, 78, ANTIQUE_GOLD);
    tft.fillRect(bookX + 14, bookY + 8, 4, 4, ANTIQUE_GOLD);    
    tft.fillRect(bookX + 68, bookY + 8, 4, 4, ANTIQUE_GOLD);    
    tft.fillRect(bookX + 14, bookY + 76, 4, 4, ANTIQUE_GOLD);   
    tft.fillRect(bookX + 68, bookY + 76, 4, 4, ANTIQUE_GOLD);   

    // 6. Layer: Center Antique Medallion Crest
    int cX = bookX + 43; 
    int cY = bookY + 45;
    tft.fillRect(cX - 1, cY - 16, 3, 32, ANTIQUE_GOLD); 
    tft.fillRect(cX - 16, cY - 1, 32, 3, ANTIQUE_GOLD); 
    tft.fillRect(cX - 5, cY - 7, 11, 15, ANTIQUE_GOLD); 
    tft.fillRect(cX - 2, cY - 4, 5, 9, ACADEMIA_BROWN);  
    tft.fillRect(cX - 10, cY - 10, 3, 3, ANTIQUE_GOLD);
    tft.fillRect(cX + 8, cY - 10, 3, 3, ANTIQUE_GOLD);
    tft.fillRect(cX - 10, cY + 8, 3, 3, ANTIQUE_GOLD);
    tft.fillRect(cX + 8, cY + 8, 3, 3, ANTIQUE_GOLD);

    // 7. Layer: Centered Title Text Display
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    int titleLength = strlen(fileNames[selectedIndex]);
    int centerTextX = (tft.width() - (titleLength * 12)) / 2;
    if (centerTextX < 10) centerTextX = 10; 
    
    tft.setCursor(centerTextX, 185);
    tft.print(fileNames[selectedIndex]);

    // 8. Layer: Library Volume Carousel Page Index Tracker
    tft.setTextSize(1);
    tft.setTextColor(ANTIQUE_GOLD);
    char itemTracker[12];
    sprintf(itemTracker, "%d / %d", selectedIndex + 1, fileCount);
    int centerTrackerX = (tft.width() - (strlen(itemTracker) * 6)) / 2;
    
    tft.setCursor(centerTrackerX, 215);
    tft.print(itemTracker);
}

void showMenu() {
    inMenu = true;
    tft.fillScreen(BLACK); // Generates the clean dark void landscape base
    updateSelection();
}

void waitForInput(bool reading) {
    while (true) {
        int physicalInput = readPhysicalButtons();
        if (physicalInput != BTN_NONE) {
            if (reading) {
                if (physicalInput == BTN_MENU) {
                    saveProgress(); 
                    bookFile.close();
                    showMenu();
                    delay(400); 
                    return;
                }
                if (physicalInput == BTN_NEXT) {
                    if (historyIndex < 24) {
                        historyIndex++;
                        pageHistory[historyIndex] = bookFile.position();
                    } else {
                        for (int i = 0; i < 24; i++) {
                            pageHistory[i] = pageHistory[i + 1];
                        }
                        pageHistory[24] = bookFile.position();
                    }
                    pageStartPos = bookFile.position(); 
                    delay(400); 
                    return;
                }
                if (physicalInput == BTN_PREV) {
                    if (historyIndex > 0) {
                        historyIndex--;
                        pageStartPos = pageHistory[historyIndex];
                        delay(400); 
                        return;
                    }
                }
            } else {
                if (physicalInput == BTN_NEXT || physicalInput == BTN_PREV) {
                    if (physicalInput == BTN_NEXT) {
                        selectedIndex = (selectedIndex + 1) % fileCount;
                    } else {
                        selectedIndex = (selectedIndex - 1 + fileCount) % fileCount;
                    }
                    updateSelection(); 
                    delay(400);
                }
                if (physicalInput == BTN_MENU) {
                    openBook();
                    delay(400);
                    return;
                }
            }
        }
    }
}

void loop() {
    if (inMenu) {
        waitForInput(false);
    } else {
        tft.fillScreen(BLACK);
        tft.setCursor(10, 20); 
        tft.setTextColor(WHITE);
        tft.setTextSize(2);
        
        bookFile.seek(pageStartPos);
        
        char currentWord[21];
        int wordIdx = 0;
        uint32_t wordStartPos = pageStartPos;

        while (bookFile.available()) {
            if (wordIdx == 0) {
                wordStartPos = bookFile.position();
            }
            
            char c = bookFile.read();
            
            if (c == ' ' || c == '\n' || c == '\r') {
                if (wordIdx > 0) {
                    currentWord[wordIdx] = '\0';
                    int wordWidth = strlen(currentWord) * 12;
                    
                    if (tft.getCursorX() + wordWidth > tft.width() - 10) tft.println();
                    
                    if (tft.getCursorY() + 25 > tft.height()) {
                        bookFile.seek(wordStartPos);
                        waitForInput(true);
                        return; 
                    }
                    tft.print(currentWord);
                    wordIdx = 0;
                }
                if (c == ' ') tft.print(F(" "));
                if (c == '\n') tft.println();
            } else if (wordIdx < 20) {
                currentWord[wordIdx++] = c;
            }
        }
        tft.setTextColor(ANTIQUE_GOLD);
        tft.println(F("\n\n--- THE END ---"));
        char sName[16];
        getSaveName(sName);
        sd.remove(sName);
        delay(3000);
        bookFile.close();
        showMenu();
    }
}
