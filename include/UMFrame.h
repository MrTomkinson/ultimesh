/**
 * @file UMFrame.h
 * @brief Defines the UMFrame structure used for all packet transmissions in the mesh network.
 *
 * The UMFrame class represents a serialized, chunkable, and typed message used to communicate
 * across the network using LoRa or other transport mediums. Frames can represent shell commands,
 * responses, file transfers, and more.
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include <cstdint>

/**
 * UMFrame: Universal Message Frame
 * Used for transmitting structured messages across nodes.
 */
class UMFrame {
public:
    // 🔁 Frame types for different message protocols
    enum FrameType {
        DATA,         // General data
        ACK,          // Acknowledgement for received packet
        CONTROL,      // Control message
        END,          // End of transmission signal
        BROADCAST,    // Broadcast to all nodes
        DIRECT_MSG,   // Peer-to-peer message
        SSH,          // Remote SSH-style session command
        SHELL,        // Shell-style command for local exec
        RESP,         // A response to a command
        TRANSFER,     // File or large object transfer
        CHUNK_ACK,    // Per-chunk acknowledgment
        CMD           // 🔧 Future use: remote meta/command packets
    };

    FrameType type;

    std::vector<uint8_t> from;      // Node ID of sender
    std::vector<uint8_t> to;        // Node ID of recipient
    int chunkNumber;                // If part of a chunked message
    int totalChunks;                // Total number of chunks
    std::vector<uint8_t> filename;  // Optional: file or label
    std::vector<uint8_t> payload;   // The actual compressed data

    UMFrame();

    // Convert this frame into a byte vector (compressed or encoded)
    bool encode(std::vector<uint8_t>& output) const;

    // Parse an incoming byte buffer back into a UMFrame
    static bool parse(const uint8_t* data, size_t length, UMFrame& out);

    // Convert type enum to printable char
    static char typeToChar(FrameType t);

    // Parse char type from encoded frame into enum
    static FrameType parseType(char c);
};
