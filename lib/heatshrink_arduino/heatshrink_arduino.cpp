#include "heatshrink_arduino.h"
#include <vector>

String compressData(const String& input) {
    heatshrink_encoder encoder;
    heatshrink_encoder_reset(&encoder);

    // Make mutable buffer
    std::vector<uint8_t> inputVec(input.begin(), input.end());
    size_t input_size = inputVec.size();

    uint8_t comp_out[256];
    size_t comp_size = 0;

    size_t sunk = 0;
    while (sunk < input_size) {
        size_t amt_sunk;
        heatshrink_encoder_sink(&encoder, inputVec.data() + sunk, input_size - sunk, &amt_sunk);  // ✅ NO const issue
        sunk += amt_sunk;

        HSE_poll_res pres;
        size_t poll_size;
        do {
            pres = heatshrink_encoder_poll(&encoder, comp_out + comp_size, sizeof(comp_out) - comp_size, &poll_size);
            comp_size += poll_size;
        } while (pres == HSER_POLL_MORE);
    }

    heatshrink_encoder_finish(&encoder);

    HSE_poll_res pres;
    size_t poll_size;
    do {
        pres = heatshrink_encoder_poll(&encoder, comp_out + comp_size, sizeof(comp_out) - comp_size, &poll_size);
        comp_size += poll_size;
    } while (pres == HSER_POLL_MORE);

    return String((const char*)comp_out, comp_size);
}

String decompressData(const String& input) {
    heatshrink_decoder decoder;
    heatshrink_decoder_reset(&decoder);

    std::vector<uint8_t> inputVec(input.begin(), input.end());
    size_t input_size = inputVec.size();

    uint8_t out_buf[256];
    String result = "";

    size_t sunk = 0;
    while (sunk < input_size) {
        size_t amt_sunk;
        heatshrink_decoder_sink(&decoder, inputVec.data() + sunk, input_size - sunk, &amt_sunk);  // ✅ NO const issue
        sunk += amt_sunk;

        HSD_poll_res pres;
        size_t poll_size;
        do {
            pres = heatshrink_decoder_poll(&decoder, out_buf, sizeof(out_buf), &poll_size);
            result += String((const char*)out_buf, poll_size);
        } while (pres == HSDR_POLL_MORE);
    }

    heatshrink_decoder_finish(&decoder);

    HSD_poll_res pres;
    size_t poll_size;
    do {
        pres = heatshrink_decoder_poll(&decoder, out_buf, sizeof(out_buf), &poll_size);
        result += String((const char*)out_buf, poll_size);
    } while (pres == HSDR_POLL_MORE);

    return result;
}
