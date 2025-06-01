#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "text_editor.h"

#define MAX_LINES 64
#define MAX_LINE_LENGTH 128      // Increased line limit
#define MAX_RENDER_WIDTH 80      // Truncate display

void deleteCurrentLine();

String lines[MAX_LINES];
int numLines = 0;

int cursorX = 0;
int cursorY = 0;
bool isDirty = false;

String currentFile = "";

void clearScreen() {
    Serial.print("\033[2J\033[H"); // ANSI clear + home
}

void renderEditor() {
    clearScreen();
    Serial.printf("ULTIMESH EDITOR — Editing: %s\n", currentFile.c_str());
    Serial.println("--------------------------------------------------");

    int viewTop = max(0, cursorY - 10);
    int viewBottom = min(viewTop + 20, MAX_LINES);

    for (int i = viewTop; i < viewBottom; i++) {
        String content = (i < numLines) ? lines[i] : "";
        content = content.substring(0, MAX_RENDER_WIDTH);  // Truncate

        if (i == cursorY)
            Serial.printf("> %2d: %s\n", i + 1, content.c_str());
        else
            Serial.printf("  %2d: %s\n", i + 1, content.c_str());
    }

    Serial.println("--------------------------------------------------");
    Serial.print("Ctrl+S: Save | Ctrl+A: Save As | Ctrl+N: New | Ctrl+D: Del Line | Ctrl+Q: Quit | Ctrl+I: Help\n");
    Serial.printf("Cursor: %d:%d | Modified: %s\n", cursorY + 1, cursorX + 1, isDirty ? "YES" : "NO");
}

void showHelp() {
    clearScreen();
    Serial.println("--- ULTIMESH EDITOR KEYS ---");
    Serial.println("Ctrl+S     Save current file");
    Serial.println("Ctrl+A     Save As (prompt for filename)");
    Serial.println("Ctrl+N     New file (prompt for name)");
    Serial.println("Ctrl+D     Delete current line");
    Serial.println("Ctrl+Q     Quit (asks to save if needed)");
    Serial.println("Arrow Keys Move cursor (left/right/up/down)");
    Serial.println("Enter      Insert new line");
    Serial.println("Backspace  Delete character before cursor");
    Serial.println("Ctrl+I     Show this help screen");
    Serial.println("----------------------------");
    Serial.println("Press any key to return...");
    while (!Serial.available());
    Serial.read();
    renderEditor();
}

void loadFile(const String &filename) {
    currentFile = filename;
    numLines = 0;
    cursorX = 0;
    cursorY = 0;
    isDirty = false;

    File file = SPIFFS.open(filename, "r");
    if (!file) {
        Serial.println("[!] File not found, starting new.");
        lines[0] = "";
        numLines = 1;
        return;
    }

    while (file.available() && numLines < MAX_LINES) {
        lines[numLines++] = file.readStringUntil('\n');
    }
    file.close();
}

void saveFile(const String &filename) {
    File file = SPIFFS.open(filename, "w");
    if (!file) {
        Serial.println("[!] Failed to save file.");
        return;
    }

    for (int i = 0; i < numLines; i++) {
        file.println(lines[i]);
    }

    file.close();
    currentFile = filename;
    isDirty = false;
    Serial.println("[✓] File saved.");
}

void promptFilenameAndSaveAs() {
    Serial.println("\nSave as filename: ");
    String newFile = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') break;
            newFile += c;
            Serial.print(c);
        }
    }
    saveFile("/" + newFile);
}

void promptFilenameAndNew() {
    Serial.println("\nNew filename: ");
    String newFile = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') break;
            newFile += c;
            Serial.print(c);
        }
    }
    for (int i = 0; i < MAX_LINES; i++) lines[i] = "";
    numLines = 1;
    cursorX = 0;
    cursorY = 0;
    isDirty = false;
    currentFile = "/" + newFile;
}

void insertChar(char c) {
    String &line = lines[cursorY];
    if (line.length() < MAX_LINE_LENGTH - 1) {
        line = line.substring(0, cursorX) + c + line.substring(cursorX);
        cursorX++;
        isDirty = true;
    }
}

void deleteChar() {
    String &line = lines[cursorY];

    if (cursorX > 0) {
        // Standard character delete
        line.remove(cursorX - 1, 1);
        cursorX--;
        isDirty = true;
    } else if (cursorY > 0) {
        // Merge with previous line
        int prevLen = lines[cursorY - 1].length();
        if (lines[cursorY].length() + prevLen < MAX_LINE_LENGTH) {
            lines[cursorY - 1] += lines[cursorY];
            deleteCurrentLine(); // This updates cursorY
            cursorY--;
            cursorX = prevLen;
            isDirty = true;
        }
    }
}

void handleDeleteKey() {
    String &line = lines[cursorY];
    if (line.isEmpty() && numLines > 1) {
        deleteCurrentLine();
        renderEditor(); // Immediate feedback
    }
}


void deleteCurrentLine() {
    if (numLines <= 1) {
        lines[0] = "";
        cursorX = 0;
        return;
    }

    for (int i = cursorY; i < numLines - 1; i++) {
        lines[i] = lines[i + 1];
    }

    lines[numLines - 1] = "";
    numLines--;

    if (cursorY >= numLines) {
        cursorY = numLines - 1;
    }

    cursorX = min(cursorX, (int)lines[cursorY].length());
    isDirty = true;
}


void insertNewLine() {
    if (numLines < MAX_LINES) {
        for (int i = numLines; i > cursorY + 1; i--) {
            lines[i] = lines[i - 1];
        }
        lines[cursorY + 1] = lines[cursorY].substring(cursorX);
        lines[cursorY].remove(cursorX);
        numLines++;
        cursorY++;
        cursorX = 0;
        isDirty = true;
    }
}

void moveCursorUp() {
    if (cursorY > 0) {
        cursorY--;
        cursorX = min(cursorX, (int)lines[cursorY].length());
    }
}

void moveCursorDown() {
    if (cursorY < numLines - 1) {
        cursorY++;
        cursorX = min(cursorX, (int)lines[cursorY].length());
    }
}

void moveCursorLeft() {
    if (cursorX > 0) {
        cursorX--;
    }
}

void moveCursorRight() {
    if (cursorX < lines[cursorY].length()) {
        cursorX++;
    }
}

void processEditorInput() {
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();

            if (c == 0x1B) { // ESC sequence
                while (!Serial.available());
                char next1 = Serial.read();
                if (next1 == '[') {
                    while (!Serial.available());
                    char next2 = Serial.read();
                    switch (next2) {
                        case 'A': moveCursorUp(); break;
                        case 'B': moveCursorDown(); break;
                        case 'C': moveCursorRight(); break;
                        case 'D': moveCursorLeft(); break;
                    }
                }
            }
            else if (c == 0x13) { // Ctrl+S
                saveFile(currentFile);
            }
            else if (c == 0x01) { // Ctrl+A
                promptFilenameAndSaveAs();
            }
            else if (c == 0x0E) { // Ctrl+N
                promptFilenameAndNew();
            }
            else if (c == 0x04) { // Ctrl+D = Delete Line
                deleteCurrentLine();
            }
            else if (c == 0x11) { // Ctrl+Q
                Serial.println("\nExit editor? Save first? (y/n)");
                while (!Serial.available());
                char confirm = Serial.read();
                if (confirm == 'y' || confirm == 'Y') {
                    saveFile(currentFile);
                }
                Serial.println("[✓] Editor closed.\n");
                return;
            }
            else if (c == 0x09) { // Ctrl+I = Help
                showHelp();
            }
            else if (c == '\n') {
                insertNewLine();
            }
            else if (c == 0x08) { // Backspace
    deleteChar();
}
else if (c == 127) { // DEL key
    handleDeleteKey();
}
            else if (c == '\t') {
                insertChar(' ');
                insertChar(' ');
            }
            else if (c >= 32 && c <= 126) {
                insertChar(c);
            }

            renderEditor();
        }
    }
}

void launchTextEditor(const String &filename) {
    loadFile(filename);
    renderEditor();
    processEditorInput();
}

