ESP8266 GPIO16 TCP Example Project
========

Example project for controlling 16 GPIO pins (via Microchip MCP23S17 SPI Expansion Chip) over Wifi using TCP packets.

Connect your ESP8266 Pins up to the MCP23S17 according to the below table

| ESP GPIO # |  SPI Function | MCP23S17    |
|------------|---------------|-------------|
| GPIO12     | MISO (DIN)    | Pin 14 (SO) |
| GPIO13     | MOSI (DOUT)   | Pin 13 (SI) |
| GPIO14     | CLOCK         | Pin 12 (SCK)|
| GPIO15     | CS / SS       | Pin 11 (CS) |

Connect A0, A1, and A2 pins on the MCP23S17 to ground (Pins 15/16/17)
Connect RESET on the MCP23S17 to VCC (+3.3V) using a 1k/4.7k/10k pullup resistor
Leave INTA and INTB unconnected on the MCP23S17

You can define the SSID and SSID_PASSWORD for your wireless AP to connect the ESP8266 to your local network at the top of the user_main.c file.

You can define the starting text of the SoftAP_SSID also. The remainder of the SSID will be the ESP8266 SoftAP MAC address. 
The SoftAP_SSID_PASSWORD can also be set along with the encryption mode (AP_AUTH).

The SoftAP uses IP 192.168.5.1 for the ESP8266, and gives out DHCP addresses for connecting devices from 192.168.5.2 to 192.168.5.100

TCP Listener
========
The ESP8266 listens on port 33333 for a TCP packet. The packet format is a simple 2 Byte packet.

Send "0xFF 0xFF" to turn on all outputs. 

Structure of the 16bit packet is:

A7|A6|A5|A4|A3|A2|A1|A0|B7|B6|B5|B4|B3|B2|B1|B0

Which correspont to the GPIO pins on the MCP23S17.

Using netcat you can do the following command to send a packet:

echo -ne '\x00\xFF' | nc 192.168.5.1 33333

Makefile
========
The makefile is setup for using the esp-open-sdk installed in /opt/esp-open-sdk

Modify the makefile to match your install setup or use your own Makefile from a working project.