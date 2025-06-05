import argparse
import subprocess
import meshtastic.serial_interface
from pubsub import pub
import time
import os
import datetime
import zlib
import data_packet_pb2

CHUNK_SIZE = 192  # Optimized chunk size
current_remote_dir = "/"  # Track the remote directory

def main():
    parser = argparse.ArgumentParser(description="Meshtastic raw data messaging (Text Mode).")
    parser.add_argument("--port", default="/dev/ttyUSB1", help="Serial port to use.")
    parser.add_argument("--compression", type=int, default=9, help="Zlib compression level (1-9).")
    args = parser.parse_args()

    try:
        interface = meshtastic.serial_interface.SerialInterface(args.port)
    except Exception as e:
        print(f"termrecv: Error initializing Meshtastic interface: {e}")
        exit(1)

    def execute_command(command):
        """Execute a command and send buffered output."""
        global current_remote_dir  

        try:
            start_time = datetime.datetime.now()
            print(f"termrecv: Executing command '{command}' in '{current_remote_dir}' at {start_time}")

            if command.startswith("cd "):  
                new_dir = command[3:].strip()
                try:
                    os.chdir(new_dir)
                    current_remote_dir = os.getcwd()  
                    print(f"termrecv: Changed directory to '{current_remote_dir}'")
                    return f"REMOTE_DIR: {current_remote_dir}".encode('utf-8')  
                except Exception as e:
                    return f"Error: {str(e)}".encode('utf-8')

            process = subprocess.Popen(command, shell=True, cwd=current_remote_dir,
                                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

            output_buffer = []  
            last_flush_time = time.time()
            accumulated_size = 0

            for line in iter(process.stdout.readline, ''):
                output_buffer.append(line)
                accumulated_size += len(line.encode('utf-8'))

                if (time.time() - last_flush_time) > 2 or accumulated_size >= CHUNK_SIZE:  
                    yield ''.join(output_buffer).encode('utf-8')  
                    output_buffer.clear()
                    last_flush_time = time.time()
                    accumulated_size = 0

            process.stdout.close()
            process.wait()  

            if output_buffer:
                yield ''.join(output_buffer).encode('utf-8')

            end_time = datetime.datetime.now()
            print(f"termrecv: Command '{command}' completed at {end_time}, duration: {end_time - start_time}")

        except Exception as e:
            yield str(e).encode('utf-8')

    def send_buffered_output(interface, packet, sequence_number, output_bytes):
        """Compress and send buffered output."""
        try:
            compressed_output = zlib.compress(output_bytes, level=args.compression)
            output_chunks = [compressed_output[i:i + CHUNK_SIZE] for i in range(0, len(compressed_output), CHUNK_SIZE)]
            total_chunks = len(output_chunks)

            for i, chunk in enumerate(output_chunks):
                response_packet = data_packet_pb2.DataPacket()
                response_packet.sequence_number = sequence_number
                response_packet.chunk_number = i
                response_packet.total_chunks = total_chunks
                response_packet.payload = chunk
                response_packet.packet_type = data_packet_pb2.DataPacket.DATA
                serialized_response = response_packet.SerializeToString()

                print(f"termrecv: Sending chunk {i + 1}/{total_chunks}...")
                interface.sendData(serialized_response, portNum='PRIVATE_APP', destinationId=packet['from'])
                time.sleep(0.5)  

        except Exception as e:
            print(f"termrecv: Error sending chunk: {e}")

    def onReceive(packet, interface):
        """Handles incoming packets efficiently."""
        print(f"termrecv: Received packet: {packet}")
        try:
            if 'decoded' in packet and packet['decoded']['portnum'] == 'PRIVATE_APP':
                data_packet = data_packet_pb2.DataPacket()
                data_packet.ParseFromString(packet['decoded']['payload'])

                if data_packet.packet_type == data_packet_pb2.DataPacket.DATA:
                    try:
                        command_bytes = zlib.decompress(data_packet.payload)
                        command = command_bytes.decode('utf-8')
                        print(f"termrecv: Received command: '{command}' (Packet {data_packet.sequence_number})")
                    except Exception as e:
                        print(f"termrecv: Error with zlib decompression: {e}")
                        return

                    total_chunks_estimate = "?"  
                    chunk_count = 0  
                    for output_bytes in execute_command(command):
                        chunk_count += 1
                        print(f"termrecv: Sending chunk {chunk_count}/{total_chunks_estimate}...")
                        send_buffered_output(interface, packet, data_packet.sequence_number, output_bytes)

        except Exception as e:
            print(f"termrecv: Error processing packet: {e}")

    pub.subscribe(onReceive, 'meshtastic.receive')

    while True:
        time.sleep(1)

if __name__ == "__main__":
    main()
