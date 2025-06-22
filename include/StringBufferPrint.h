#pragma once
#include <Print.h>
#include <Arduino.h>

class StringBufferPrint : public Print {
public:
    StringBufferPrint() : buffer("") {}

    virtual size_t write(uint8_t c) override {
        buffer += (char)c;
        return 1;
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override {
        this->buffer += String((const char*)buffer).substring(0, size);
        return size;
    }

    String getString() const {
        return buffer;
    }

    void clear() {
        buffer = "";
    }

private:
    String buffer;
};
