{
    "entities" : [
        {
            "id": "mcu",
            "type": "Atmega32",
            "firmware": "../firmware/bin/charliev2.elf",
            "frequency": 16000000
        },
        {
            "id": "rtc",
            "type": "Ds1307",
            "i2c_address": 104,
            "nvram_file": "ds1307_nvram.bin"
        },
        {
            "id": "sd_card",
            "type": "SdCard",
            "image": "fsimage.bin",
            "capacity": 268435456,
            "connect": {
                "SS": { "device": "mcu", "pin": "B1" }
            }
        },
        {
            "id": "enc28j60",
            "type": "Enc28J60",
            "connect": {
                "RESET": { "device": "mcu", "pin": "B3" },
                "SS": { "device": "mcu", "pin": "B4" }
            }
        },
        {
            "id": "virtual_net",
            "type": "VirtualNetwork",
            "devices": ["enc28j60"]
        },
        {
            "id": "mock_plant_hum_sensor",
            "type": "VoltageSource",
            "value": 0.0,
            "connect": {
                "OUT": { "device": "mcu", "pin": "A0" }
            }
        },
        {
            "id": "mock_plant_ilum_sensor",
            "type": "VoltageSource",
            "value": 0.0,
            "connect": {
                "OUT": { "device": "mcu", "pin": "A1" }
            }
        },
        {
            "type": "I2cBus",
            "devices": ["mcu", "rtc"]
        },
        {
            "type": "SpiBus",
            "devices": ["mcu", "sd_card", "enc28j60"]
        },
        {
            "id": "debug_led",
            "type": "SimpleLed",
            "x": 16,
            "y": 16,
            "size": 16,
            "color": "#ff0000",
            "caption": "Debug LED",
            "connect": {
                "IN": { "device": "mcu", "pin": "D7" }
            }
        },
        {
            "id": "debug_btn",
            "type": "SimplePushButton",
            "up_value": "Z",
            "down_value": 0,
            "x": 16,
            "y": 40,
            "size": 16,
            "caption": "Debug button",
            "connect": {
                "OUT": { "device": "mcu", "pin": "D6" }
            }
        },
        {
            "id": "pc_indicator",
            "type": "PCIndicator",
            "x": 160,
            "y": 18,
            "size": 14,
            "mcu": "mcu"
        },
        {
            "id": "console",
            "type": "RS232Console",
            "x": 16,
            "y": 64,
            "columns": 80,
            "rows": 25,
            "font_size": 10,
            "rs232_peer": "mcu"
        },
        {
            "id": "dashboard",
            "type": "Dashboard",
            "background": "charlie_dashboard_bkgd.png",
            "font": "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
            "monoFont": "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
            "widgets": ["debug_led", "debug_btn", "pc_indicator", "console"]
        }
    ]
}
