/*
 * dot_config.h - Build configuration header a la .config
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _DOT_CONFIG_H
#define _DOT_CONFIG_H

// general
#define CFG_MAX_PLANTS               4

// scheduler
#define CFG_SCHED_SLOTS              8

// debug
#define CFG_DEBUG_ENABLED            1
#define CFG_DEBUG_LED_PORT           PORTD
#define CFG_DEBUG_LED_DDR            DDRD
#define CFG_DEBUG_LED_BIT            7
#define CFG_DEBUG_KEY_PORT           PIND
#define CFG_DEBUG_KEY_DDR            DDRD
#define CFG_DEBUG_KEY_BIT            6

// RS-232 interface
#define CFG_RS232_BAUD               38400
#define CFG_RS232_PARITY             0
#define CFG_RS232_STOP_BITS          1

// SPI interface
#define CFG_SPI_PORT                 PORTB
#define CFG_SPI_DDR                  DDRB
#define CFG_SPI_SS_BIT               4
#define CFG_SPI_MOSI_BIT             5
#define CFG_SPI_MISO_BIT             6
#define CFG_SPI_SCK_BIT              7
#define CFG_SPI_CLOCK_DIV            2

// I2C/TWI interface
#define CFG_I2C_BAUD                 100000

// DS1307 RTC
#define CFG_RTC_I2C_ADDR             0x68

// sensors
#define CFG_SENSOR_ADC_PRESCALER     7
#define CFG_SENSOR_ADC_SAMPLES       200
#define CFG_SENSOR_MUX_PORT          PORTC
#define CFG_SENSOR_MUX_DDR           DDRC
#define CFG_SENSOR_MUX_ENABLE_BIT    5
#define CFG_SENSOR_MUX_ADDR_LSB_BIT  6
#define CFG_SENSOR_MUX_WIDTH         2

#define CFG_SENSOR_HUMID_CHAN        0
#define CFG_SENSOR_CALIB_HUMID_MIN   766
#define CFG_SENSOR_CALIB_HUMID_MAX   128

#define CFG_SENSOR_ILUM_CHAN         1
#define CFG_SENSOR_CALIB_ILUM_MIN    766
#define CFG_SENSOR_CALIB_ILUM_MAX    133

// pump
#define CFG_PUMP_MUX_PORT            PORTC
#define CFG_PUMP_MUX_DDR             DDRC
#define CFG_PUMP_MUX_ENABLE_BIT      2
#define CFG_PUMP_MUX_ADDR_LSB_BIT    3
#define CFG_PUMP_MUX_WIDTH           2

// SD card
#define CFG_CARD_CS_PORT             PORTB
#define CFG_CARD_CS_DDR              DDRB
#define CFG_CARD_CS_BIT              1

// log file
#define CFG_LOG_FILE                 "data/log.bin"
#define CFG_LOG_TEXT_ACTIVATED       "Activated"
#define CFG_LOG_TEXT_RESTARTING      "Restarting"
#define CFG_LOG_TEXT_SHUTDOWN        "Shutdown"
#define CFG_LOG_TEXT_WATERED         "Watered "
#define CFG_LOG_TEXT_WATERING        "Watering "
#define CFG_LOG_TEXT_TIME_LOST       "RTC time lost"
#define CFG_LOG_TEXT_UNKNOWN         "?"

// records file
#define CFG_RECORDS_FILE             "data/records.bin"

// config file
#define CFG_CONFIG_FILE              "sys/config.bin"

// ENC28J60 network controller
#define CFG_E28J_PORT                PORTB
#define CFG_E28J_DDR                 DDRB
#define CFG_E28J_RESET_BIT           3
#define CFG_E28J_CS_BIT              4
// Note: buffer start addresses must be even and buffer end addresses
// must be odd.
// Default receive buffer of 5881B, i.e. ~4 full Ethernet frames
#define CFG_E28J_RECV_BUFFER_START   0x0000
#define CFG_E28J_RECV_BUFFER_END     0x16FD
// Default transmission buffer of 1536B, i.e. one full Ethernet frame
#define CFG_E28J_TRAN_BUFFER_START   0x1700
#define CFG_E28J_TRAN_BUFFER_END     0x1CFF
// Default out-of-band transmission buffer of 256B (for ARP etc.)
#define CFG_E28J_OOB_BUFFER_START    0x1D00
#define CFG_E28J_OOB_BUFFER_END      0x1DFF
// Default auxiliary buffer of 512B (for application use)
#define CFG_E28J_AUX_BUFFER_START    0x1E00
#define CFG_E28J_AUX_BUFFER_END      0x1FFF
#define CFG_E28J_MAX_FRAME_SIZE      1536
#define CFG_E28J_B2B_PACKET_GAP      0x15
#define CFG_E28J_INTER_PACKET_GAP    0x0c12

// network
#define CFG_NET_RECEIVE_INTERVAL     1

// ARP
#define CFG_NET_ARP_TABLE_ENTRIES    8

// DHCP
#define CFG_NET_DHCP_TIMEOUT_SEC     4

// TCP
#define CFG_NET_TCP_PORT_HTTP        80
#define CFG_NET_TCP_WINDOW_SIZE      1024
#define CFG_NET_TCP_TIMEOUT_SEC      3
#define CFG_NET_TCP_LAPSE_SEC        4
#define CFG_NET_TCP_MAX_FAILURES     4

// Web
#define CFG_NET_WEB_WWWROOT_DIR      "wwwroot"

#endif
