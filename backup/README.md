This my first ever time programming, Be gentle.
BBMesh - Meshtastic Terminal Interface 🛰️
(still in alpha testing)
BBMesh is a terminal-based interface for sending and receiving text commands over Meshtastic devices. 
It includes a TUI (Text User Interface) for easier interaction and supports real-time progress tracking for chunked data transmission.

🚀 Features
*BUGS, quirks and unique operation.
    
    🎨 TUI Mode for Easy Interaction
    📡 Send & Receive Commands over a Meshtastic network
    📊 Real-Time Progress Updates for data transmission
    🔀 Adaptive Chunking for optimized packet delivery
    📂 File Transfer Support (Upcoming)
    
    
🛠️ Installation
1️⃣ Clone the Repository

git clone https://github.com/MrTomkinson/BBMesh.git

cd BBMesh

2️⃣ Set Up a Virtual Environment (Recommended)

python3 -m venv bbmesh_env .

source bbmesh_env/bin/activate  # On Windows use: bbmesh_env\Scripts\activate

3️⃣ Install Dependencies

pip install -r requirements.txt

🖥️ Usage
📌 Start the Meshtastic Receiver

Run termrecv.py to start listening for incoming messages:

python3 termrecv.py --port /dev/ttyUSB#

📌 Start the Sender in TUI Mode

python3 termsend_tui.py --port /dev/ttyUSB# --dest '!nodeid your sending to'

    Use Left-Right arrow keys to switch between local and remote terminals
    remote session left, local right.
    Type most terminal commands like ls, cd, pwd, apt install etc.

⚙️ Configuration

BBMesh is currently set:

    Chunk Size: 192 bytes (allows for metadata to fit within 200 limit.)
    send packet time 1 second, set higher if low signal.
    Auto-Compression: Enabled
    Packet Acknowledgments: Implemented

To customize settings, modify termrecv.py and termsend_tui.py.

🔧 Troubleshooting

 Blank screen on TUI start, press left-right arrows a couple of times. (Known bug) 
 Cursor wont always show directory. Type cd or pwd to show current directory.

 Permission Denied on Serial Port
  Run:
    sudo usermod -a -G dialout $USER
  Then restart your system.

🛠️ Future Improvements

    ✅ File Transfer Support
    ✅ Auto-Detect Available Ports
    ✅ Enhanced Error Handling
    ✅ Improved UI Design

📜 License

This project is open-source under the MIT License.
🙌 Contributing
Meshtastic.

Feel free to open issues, submit pull requests, or suggest improvements!
📞 Contact
    GitHub: @MrTomkinson
  
