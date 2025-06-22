#include "compression_utils.h"
#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include "common_utils.h"

std::vector<String> splitString(const String& input, char delimiter);


std::vector<uint8_t> compressData(const std::vector<uint8_t>& input) {
    const size_t BUF_SIZE = 128;
    heatshrink_encoder* hse = heatshrink_encoder_alloc(8, 4); // window_sz2=8, lookahead_sz2=4

    if (!hse) return {};

    std::vector<uint8_t> output;
    uint8_t buf[BUF_SIZE];

    size_t sunk = 0;
    HSE_sink_res sres = heatshrink_encoder_sink(hse, const_cast<uint8_t*>(input.data()), input.size(), &sunk);
    if (sres != HSER_SINK_OK) {
        heatshrink_encoder_free(hse);
        return {};
    }

    HSE_finish_res fres = heatshrink_encoder_finish(hse);
    if (fres != HSER_FINISH_DONE && fres != HSER_FINISH_MORE) {
        heatshrink_encoder_free(hse);
        return {};
    }

    HSE_poll_res pres;
    size_t polled = 0;
    do {
        pres = heatshrink_encoder_poll(hse, buf, BUF_SIZE, &polled);
        if (pres < 0) break;
        output.insert(output.end(), buf, buf + polled);
    } while (polled > 0);

    heatshrink_encoder_free(hse);
    return output;
}

std::vector<uint8_t> decompressData(const uint8_t* input, size_t input_size) {
    const size_t BUF_SIZE = 128;
    heatshrink_decoder* hsd = heatshrink_decoder_alloc(128, 8, 4); // input buffer, win, lookahead

    if (!hsd) return {};

    std::vector<uint8_t> output;
    uint8_t buf[BUF_SIZE];

    size_t sunk = 0;
    HSD_sink_res sres = heatshrink_decoder_sink(hsd, const_cast<uint8_t*>(input), input_size, &sunk);
    if (sres != HSDR_SINK_OK) {
        heatshrink_decoder_free(hsd);
        return {};
    }

    HSD_finish_res fres = heatshrink_decoder_finish(hsd);
    if (fres != HSDR_FINISH_DONE && fres != HSDR_FINISH_MORE) {
        heatshrink_decoder_free(hsd);
        return {};
    }

    HSD_poll_res pres;
    size_t polled = 0;
    do {
        pres = heatshrink_decoder_poll(hsd, buf, BUF_SIZE, &polled);
        if (pres < 0) break;
        output.insert(output.end(), buf, buf + polled);
    } while (polled > 0);

    heatshrink_decoder_free(hsd);
    return output;
}

// Overload to accept std::vector input and forward it to the pointer version
std::vector<uint8_t> decompressData(const std::vector<uint8_t>& input) {
    return decompressData(reinterpret_cast<const uint8_t*>(input.data()), input.size());
}
