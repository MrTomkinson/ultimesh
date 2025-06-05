import curses
import meshtastic.serial_interface
from pubsub import pub
import time
import os
import psutil
import datetime
import argparse
import subprocess
import meshtastic_utils
import zlib
import data_packet_pb2
import textwrap

local_dir = None

def update_progress_box(telemetry_win, to_node, mode, chunk_num, total_chunks):
    """Update the telemetry window with chunk progress, ensuring node ID stays visible."""
    telemetry_win.clear()
    telemetry_win.border()
    
    telemetry_win.addstr(1, 2, f"🎯 To: {to_node}")  
    telemetry_win.addstr(2, 2, f"📦 {mode}: Chunk {chunk_num + 1}/{total_chunks}")  

    telemetry_win.refresh()

def main(stdscr, args):
    global local_dir
    if local_dir is None:
        local_dir = os.getcwd() or "/"

    curses.curs_set(0)  
    stdscr.clear()
    stdscr.refresh()

    max_y, max_x = stdscr.getmaxyx()

    telemetry_height = max_y // 4  
    terminal_height = max_y - telemetry_height
    terminal_width = max_x // 2
    telemetry_width = max_x // 2  

    remote_win = curses.newwin(terminal_height, terminal_width, 0, 0)
    local_win = curses.newwin(terminal_height, terminal_width, 0, terminal_width)
    telemetry_win = curses.newwin(telemetry_height, telemetry_width, terminal_height, 0)
    system_win = curses.newwin(telemetry_height, telemetry_width, terminal_height, telemetry_width)

    remote_win.scrollok(True)
    local_win.scrollok(True)

    interface = meshtastic.serial_interface.SerialInterface(args.port)
    sequence_number = 0
    remote_dir = "~"
    local_dir = os.getcwd() or "/"

    remote_input_buffer = ""
    local_input_buffer = ""

    received_output = {}

    def draw_borders():
        """Redraw window borders."""
        remote_win.border()
        local_win.border()
        telemetry_win.border()
        system_win.border()

        remote_win.refresh()
        local_win.refresh()
        telemetry_win.refresh()
        system_win.refresh()

    def clear_command_line(input_win, current_dir, input_buffer):
        input_win.move(input_win.getmaxyx()[0] - 2, 2)
        input_win.clrtoeol()
        input_win.addstr(f"{remote_dir}$ {input_buffer}")
        input_win.refresh()

    def execute_local_command(command):
        """Execute local commands, handling `cd` properly."""
        global local_dir
        if command.startswith("cd "):
            new_dir = command[3:].strip()
            try:
                os.chdir(new_dir)
                local_dir = os.getcwd() or "/"
            except Exception as e:
                return f"Error: {str(e)}\n"
            return ""
        else:
            try:
                result = subprocess.run(command, shell=True, capture_output=True, text=True, cwd=local_dir)
                return result.stdout + result.stderr
            except Exception as e:
                return f"Error: {str(e)}\n"

    def update_system_info():
        """Update system stats in the right telemetry window."""
        system_win.clear()
        system_win.border()

        cpu_usage = psutil.cpu_percent()
        mem = psutil.virtual_memory()
        time_now = datetime.datetime.now().strftime("%H:%M:%S")
        date_now = datetime.datetime.now().strftime("%Y-%m-%d")

        system_win.addstr(1, 2, f"📅 Date: {date_now}")
        system_win.addstr(2, 2, f"⏰ Time: {time_now}")
        system_win.addstr(3, 2, f"💾 Memory: {mem.percent}% used")
        system_win.addstr(4, 2, f"⚡ CPU: {cpu_usage}% used")

        system_win.refresh()

    def onReceive(packet, interface):
        """Handle incoming packets and process responses with smart wrapping."""
        if 'decoded' not in packet:
            return  

        if packet['decoded']['portnum'] == 'PRIVATE_APP':
            data_packet = data_packet_pb2.DataPacket()
            data_packet.ParseFromString(packet['decoded']['payload'])

            if data_packet.packet_type == data_packet_pb2.DataPacket.DATA:
                try:
                    seq_num = data_packet.sequence_number
                    chunk_num = data_packet.chunk_number
                    total_chunks = data_packet.total_chunks

                    update_progress_box(telemetry_win, args.dest, "Receiving", chunk_num, total_chunks)

                    if seq_num not in received_output:
                        received_output[seq_num] = {}

                    received_output[seq_num][chunk_num] = data_packet.payload

                    if len(received_output[seq_num]) == total_chunks:
                        ordered_chunks = [received_output[seq_num][i] for i in range(total_chunks)]
                        compressed_output = b''.join(ordered_chunks)
                        output_bytes = zlib.decompress(compressed_output)
                        payload = output_bytes.decode('utf-8', errors='ignore')

                        max_width = remote_win.getmaxyx()[1] - 2  
                        for line in payload.split("\n"):  
                            if len(line) > max_width:  
                                wrapped_lines = textwrap.wrap(line, max_width)
                                for wrapped_line in wrapped_lines:
                                    remote_win.addstr(wrapped_line + "\n")
                            else:
                                remote_win.addstr(line + "\n")

                        remote_win.refresh()
                        del received_output[seq_num]  
                except Exception as e:
                    remote_win.addstr(f"Decompression error: {e}\n")
                    remote_win.refresh()

            update_telemetry(packet, telemetry_win)

    pub.subscribe(onReceive, 'meshtastic.receive')

    def update_telemetry(packet, telemetry_win):
        """Update telemetry information on the UI."""
        telemetry_win.border()

        telemetry_win.addstr(3, 2, f"📶 Signal: {packet.get('rxSnr', 'N/A')}")
        telemetry_win.addstr(4, 2, f"🔁 Hops: {packet.get('hopLimit', 'N/A')}")

        telemetry_win.refresh()

    draw_borders()
    update_system_info()

    while True:
        key = stdscr.getch()
        if key == curses.KEY_LEFT:
            current_window = "remote"
        elif key == curses.KEY_RIGHT:
            current_window = "local"

        if current_window == "remote":
            input_win = remote_win
            input_buffer = remote_input_buffer
            current_dir = remote_dir
        else:
            input_win = local_win
            input_buffer = local_input_buffer
            current_dir = local_dir

        if key == 10:  
            command = input_buffer
            input_win.addstr(f"\n{current_dir}$ {command}\n")
            input_win.refresh()

            if current_window == "remote":
                meshtastic_utils.send_command(interface, args.dest, command, sequence_number)
                update_progress_box(telemetry_win, args.dest, "Sending", 0, 1)  
                sequence_number += 1
            else:
                output = execute_local_command(command)
                input_win.addstr(output + "\n")
                input_win.refresh()

            input_buffer = ""
            clear_command_line(input_win, current_dir, input_buffer)

        elif key in [curses.KEY_BACKSPACE, 127]:  
            if input_buffer:
                input_buffer = input_buffer[:-1]
            clear_command_line(input_win, current_dir, input_buffer)

        elif 32 <= key <= 126:  
            input_buffer += chr(key)
            clear_command_line(input_win, current_dir, input_buffer)

        update_system_info()

        if current_window == "remote":
            remote_input_buffer = input_buffer
        else:
            local_input_buffer = input_buffer

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Meshtastic TUI messaging.")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Serial port to use.")
    parser.add_argument("--dest", required=True, type=str, help="Destination node ID (hexadecimal string).")
    parser.add_argument("--debug", action="store_true", help="Enable debug output.")
    args = parser.parse_args()

    try:
        curses.wrapper(lambda stdscr: main(stdscr, args))
    except KeyboardInterrupt:
        print("TUI mode terminated by user.")
