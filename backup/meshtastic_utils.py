#import meshtastic.serial_interface
#from pubsub import pub
import time
import data_packet_pb2
import zlib

received_output_chunks = {}
sent_chunks = {}

def onReceive(packet, interface, console_win=None, debug=False):
    if debug:
        print(f"Packet Received: {packet}")
    try:
        if 'decoded' in packet:
            if packet['decoded']['portnum'] == 'PRIVATE_APP':
                data_packet = data_packet_pb2.DataPacket()
                data_packet.ParseFromString(packet['decoded']['payload'])

                if data_packet.packet_type == data_packet_pb2.DataPacket.DATA:
                    if debug:
                        print(f"Data packet received: Seq={data_packet.sequence_number}, Chunk={data_packet.chunk_number}/{data_packet.total_chunks}")
                        print(f"Received Chunks: {received_output_chunks[data_packet.sequence_number].keys()}")
                    if data_packet.sequence_number not in received_output_chunks:
                        received_output_chunks[data_packet.sequence_number] = {}
                    received_output_chunks[data_packet.sequence_number][data_packet.chunk_number] = data_packet.payload
                    if len(received_output_chunks[data_packet.sequence_number]) == data_packet.total_chunks:
                        ordered_chunks = [received_output_chunks[data_packet.sequence_number][i] for i in range(data_packet.total_chunks)]
                        full_output = b''.join(ordered_chunks)
                        try:
                            full_output = zlib.decompress(full_output).decode('utf-8', errors='ignore')
                            if console_win:
                                console_win.addstr(console_win.getyx()[0] + 1, 1, f"{full_output}\n")
                                console_win.refresh()
                            else:
                                print(f"{full_output}\n")
                           # del received_output_chunks[data_packet.sequence_number]
                        except Exception as e:
                            if console_win:
                                console_win.addstr(console_win.getyx()[0] + 1, 1, f"Decompression Error: {e}\n")
                                console_win.refresh()
                            else:
                                print(f"Decompression Error: {e}\n")
                elif data_packet.packet_type == data_packet_pb2.DataPacket.NACK:
                    if debug:
                        print(f"Received NACK: Seq={data_packet.sequence_number}, Chunk={data_packet.chunk_number}")
                    if data_packet.sequence_number in sent_chunks and data_packet.chunk_number in sent_chunks[data_packet.sequence_number]:
                        # ... (NACK handling logic)
                        pass
            elif packet['decoded']['portnum'] == 'TELEMETRY_APP':
                # Ignore telemetry messages
                if debug:
                    print ("Telemetry Message Ignored")

    except Exception as e:
        print(f"Error in onReceive: {e}")

def send_command(interface, dest, command, sequence_number, console_win=None):
    print(f"Sending command: {command}, Seq={sequence_number}")
    data_packet = data_packet_pb2.DataPacket()
    data_packet.packet_type = data_packet_pb2.DataPacket.DATA
    data_packet.sequence_number = sequence_number
    data_packet.chunk_number = 0
    data_packet.total_chunks = 1

    compressed_command = zlib.compress(command.encode('utf-8'))
    chunk_size = 192  # Adjusted chunk size

    chunks = [compressed_command[i:i + chunk_size] for i in range(0, len(compressed_command), chunk_size)]
    data_packet.total_chunks = len(chunks)
    sent_chunks[sequence_number] = {}

    for chunk_number, chunk in enumerate(chunks):
        data_packet.chunk_number = chunk_number
        data_packet.payload = chunk
        interface.sendData(data_packet.SerializeToString(), dest, portNum=256) # Use the value of PRIVATE_APP
        print(f"Sent chunk: Seq={sequence_number}, Chunk={chunk_number}/{data_packet.total_chunks}")
        sent_chunks[sequence_number][chunk_number] = chunk
        time.sleep(.5)  # Added delay

    print(f"Sent command: '{command}' (Packet {sequence_number}) to node {dest}")