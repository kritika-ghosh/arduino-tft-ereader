# Arduino Touch E-Reader 📖

A fully functional, offline digital book reader built with an **Arduino Uno** and a **2.4" TFT Touch Shield**. This project features a custom menu system, smart word-wrap text rendering, and a persistent save system that remembers your reading progress.

## 🚀 Key Features
* **Dynamic Library:** Automatically scans the SD card root for `.txt` files and displays them in a scrollable menu (supports up to 10 books).
* **Smart Word Wrap:** A custom algorithm that buffers words to ensure they aren't split across lines, providing a professional reading experience.
* **Persistent Bookmarks:** Automatically creates `.SV` files on your SD card to save the exact byte position where you left off.
* **Touch-Based Navigation:** * **Menu Mode:** Tap the bottom purple bar to cycle through books; tap the top header to open the selected story.
    * **Reading Mode:** Tap the bottom area to flip to the next page; tap the top purple bar to save your progress and exit.
* **Eye-Comfort UI:** Designed with a "Purple & Dark" theme for high contrast and reduced eye strain.

## 🛠 Hardware Architecture
* **MCU:** Arduino Uno R3
* **Display:** 2.4-inch TFT LCD Shield (Parallel Interface)
* **Touch:** Integrated Resistive Touch Panel
* **Storage:** MicroSD Card (formatted to FAT32)
* **Power:** 9V Battery (Portable) or USB Input

## 📦 Required Libraries
Ensure you have the following installed via the Arduino Library Manager:
1. `SdFat` - High-speed SD card management.
2. `MCUFRIEND_kbv` - The primary display driver.
3. `Adafruit_GFX` - Core graphics and text rendering.
4. `TouchScreen` - Resistive touch coordinate handling.

## 📂 Installation & Setup
1. **Prepare SD Card:** Format your MicroSD card to **FAT32**. 
2. **Add Books:** Drag and drop `.txt` files into the root directory. Keep filenames under 13 characters for 8.3 format compatibility.
3. **Assemble:** Plug the TFT Shield directly onto the Arduino Uno.
4. **Upload:** Flash the `eReader.ino` sketch to your Arduino.

## 💻 Technical Implementation
### The "Shared Pin" Solution
Because the TFT Shield shares pins between the LCD and the Touch Panel, the code re-initializes the pin modes every time a touch is detected:
```cpp
TSPoint p = ts.getPoint();
pinMode(XM, OUTPUT); pinMode(YP, OUTPUT);
pinMode(XP, OUTPUT); pinMode(YM, OUTPUT);

```

### Bookmark Logic

Reading progress is saved by capturing the `uint32_t` file position of the first character on the current page. This position is stored in a hidden `.SV` file. When a book is reopened, the code uses `bookFile.seek(pageStartPos)` to jump straight back to that location.

## 🤝 Contributing

As a Computer Science student project, this is open for improvements! Future enhancements could include:

* Support for custom fonts.
* Brightness control for the backlight.
* Folder support for larger libraries.

---

**Author:** Kritika Ghosh
