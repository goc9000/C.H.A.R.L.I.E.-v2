/*
 * php_impl.c - Dynamic pages layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "debug/debug.h"
#include "util/stringset.h"
#include "util/misc.h"
#include "proto/ip.h"
#include "proto/eth.h"
#include "proto/http.h"
#include "core/config.h"
#include "core/plants.h"
#include "model/query.h"
#include "model/log.h"
#include "model/records.h"
#include "time/time.h"
#include "time/dev/rtc_ds1307.h"
#include "storage/filesys.h"
#include "php_impl.h"
#include "charlie.h"

typedef void (*php_conv_fn_t)(char *, void *);
typedef bool (*php_routine_fn_t)(void);

#define PHP_TAB_PLANTS            1
#define PHP_TAB_NETWORK           2
#define PHP_TAB_ALERTS            3
#define PHP_TAB_TIME              4
#define PHP_TAB_RESET             5

#define PHP_CMD_APPLY             1
#define PHP_CMD_EXPORT            2
#define PHP_CMD_SAVE              1
#define PHP_CMD_RESTART           1
#define PHP_CMD_SHUTDOWN          2
#define PHP_CMD_PURGE_LOG         3
#define PHP_CMD_PURGE_RECORDS     4

#define PHP_NODE_STACK_SIZE       8
#define PHP_CALL_STACK_SIZE       4
#define PHP_MAX_CHUNK_SIZE       64

static struct {
    char chunk[PHP_MAX_CHUNK_SIZE];
    uint8_t dribble; // how much of the current chunk has been read
    bool finished;
    
    uint8_t node_stack[PHP_NODE_STACK_SIZE];
    uint8_t node_stack_ptr;
    uint16_t call_stack[PHP_CALL_STACK_SIZE];
    uint8_t call_stack_ptr;
    bool tag_decl_open;
    bool attr_open;
    
    uint16_t pc;
    bool truth;
    
    uint8_t content_type;
    PGM_P download_filename;
    
    uint8_t page_id;
    time_t render_date;
    
    struct {
        uint16_t page;
        uint16_t per_page;
        time_t date0;
        time_t date1;
        bool reverse;
        uint8_t tab;
        uint8_t command;
        uint8_t checkboxes;
    } params;
    
    struct {
        uint16_t pages;
        Query query;
        union {
            Record record;
            LogEntry event;
        } entry;
        uint8_t plant_idx;
        union {
            PlantStatus status;
            PlantConfig cfg;
        } plant;
    } vars;
} php;

void _php_format_uint8(char *buf, uint8_t *value_ptr)
{
    itoa10(*value_ptr, buf);
}

void _php_format_uint16(char *buf, uint16_t *value_ptr)
{
    ltoa10(*value_ptr, buf);
}

void _php_format_int16(char *buf, int16_t *value_ptr)
{
    ltoa10(*value_ptr, buf);
}

void _php_format_uint32(char *buf, uint32_t *value_ptr)
{
    ltoa10(*value_ptr, buf);
}

void _php_format_datetime(char *buf, time_t *value_ptr)
{
    time_format_rfc3339(buf, *value_ptr);
}

void _php_format_datetime_csv(char *buf, time_t *value_ptr)
{
    time_format_rfc3339(buf, *value_ptr);
    buf[10] = ' ';
}

void _php_format_event_type(char *buf, uint8_t *value_ptr)
{
    strcpy_P(buf, strset_get(PSTR("Info\0Note\0Error\0\0"), ((*value_ptr) >> 6) & 3));
}

void _php_format_event_message(char *buf, LogEntry *value_ptr)
{
    log_format_message(buf, value_ptr);
}

#define _php_format_ip           ip_format
#define _php_format_mac          eth_format_mac

#define PHP_UCODE_END                      (0x00)
#define PHP_UCODE_RET                      (0x01)
#define PHP_UCODE_CLOSE_TAG                (0x02)
#define PHP_UCODE_TEXT(string)             (0x03), (string)
#define PHP_UCODE_JUMP(addr)               (0x10 + ((addr) >> 8)), ((addr) & 0xff)
#define PHP_UCODE_CALL(addr)               (0x18 + ((addr) >> 8)), ((addr) & 0xff)
#define PHP_UCODE_FORMAT(var,conv)         (0x20 + (conv)), (var)
#define PHP_UCODE_ROUTINE(routine)         (0x40 + (routine))
#define PHP_UCODE_ATTR(attr)               (0x80 + (attr))
#define PHP_UCODE_OPEN_TAG(node)           (0xc0 + (node))

#include "php_code.h"

static void _php_close_attr(void)
{
    if (php.attr_open) {
        strcat_P(php.chunk, PSTR("\""));
        php.attr_open = FALSE;
    }
}

static void _php_close_tag_decl(void)
{
    uint8_t node_id;
    PGM_P node_name;
    bool is_pi;
    
    _php_close_attr();
    
    if (php.tag_decl_open) {
        node_id = php.node_stack[php.node_stack_ptr - 1];
        node_name = strset_get(PHP_NODES, node_id);
        is_pi = (pgm_read_byte(node_name) == '?');
        
        strcat_P(php.chunk, is_pi ? PSTR("?>") : PSTR(">"));
        php.tag_decl_open = FALSE;
    }
}

static void _php_exec_open_tag(uint8_t opcode)
{
    uint8_t node_id = opcode & 0x3f;
    
    _php_close_tag_decl();
    
    php.node_stack[php.node_stack_ptr++] = node_id;
    
    strcat_P(php.chunk, PSTR("<"));
    strcat_P(php.chunk, strset_get(PHP_NODES, node_id));
    
    php.tag_decl_open = TRUE;
}

static void _php_exec_close_tag(void)
{
    uint8_t node_id;
    
    _php_close_tag_decl();
    
    node_id = php.node_stack[--php.node_stack_ptr];
        
    strcat_P(php.chunk, PSTR("</"));
    strcat_P(php.chunk, strset_get(PHP_NODES, node_id));
    strcat_P(php.chunk, PSTR(">"));
}

static void _php_exec_attr(uint8_t opcode)
{
    _php_close_attr();
    
    strcat_P(php.chunk, PSTR(" "));
    strcat_P(php.chunk, strset_get(PHP_ATTRS, opcode & 0x3f));
    strcat_P(php.chunk, PSTR("=\""));
    
    php.attr_open = TRUE;
}

static void _php_exec_format(uint8_t opcode)
{
    uint8_t fn_index = opcode & 0x1f;
    uint8_t var_index = pgm_read_byte(PHP_CODE + (php.pc++));
    php_conv_fn_t fn = (php_conv_fn_t)pgm_read_word(PHP_CONVERSIONS + fn_index);
    void *addr = (void *)pgm_read_word(PHP_VARS + var_index);
    
    fn(php.chunk + strlen(php.chunk), addr);
}

static void _php_exec_text()
{
    uint8_t str_id = pgm_read_byte(PHP_CODE + (php.pc++));
    
    strcat_P(php.chunk, strset_get(PHP_STRINGS, str_id));
}

static void _php_exec_ret()
{
    php.pc = php.call_stack[--php.call_stack_ptr];
}

static void _php_exec_jump(uint8_t opcode)
{
    uint16_t addr = ((opcode & 0x07) << 8) + pgm_read_byte(PHP_CODE + (php.pc++));
    
    if (php.truth)
        php.pc = addr;
}

static void _php_exec_call(uint8_t opcode)
{
    uint16_t addr = ((opcode & 0x07) << 8) + pgm_read_byte(PHP_CODE + (php.pc++));
    
    php.call_stack[php.call_stack_ptr++] = php.pc;
    php.pc = addr;
}

static void _php_exec_routine(uint8_t opcode)
{
    uint8_t fn_index = opcode & 0x3f;
    php_routine_fn_t fn = (php_routine_fn_t)pgm_read_word(PHP_ROUTINES + fn_index);
    
    php.truth = fn();
}

static bool _php_next_chunk(void)
{
    bool just_called;
    uint8_t op;
    
    if (php.finished)
        return FALSE;
    
    php.chunk[0] = 0;
    php.dribble = 0;
    
    just_called = FALSE;
    
    while (TRUE) {
        if (!just_called)
            php.truth = TRUE;
        else
            just_called = FALSE;
        
        op = pgm_read_byte(PHP_CODE + (php.pc++));
        
        if (op == PHP_UCODE_END) {
            php.finished = TRUE;
            return FALSE;
        } else if (op == PHP_UCODE_RET) {
            _php_exec_ret();
        } else if (op == PHP_UCODE_CLOSE_TAG) {
            _php_exec_close_tag();
        } else if (op == 0x03) {
            _php_exec_text();
            break;
        } else if ((op & 0xf8) == 0x10) {
            _php_exec_jump(op);
        } else if ((op & 0xf8) == 0x18) {
            _php_exec_call(op);
        } else if ((op & 0xe0) == 0x20) {
            _php_exec_format(op);
            break;
        } else if ((op & 0xc0) == 0x40) {
            _php_exec_routine(op);
            just_called = TRUE;
        } else if ((op & 0xc0) == 0x80) {
            _php_exec_attr(op);
            break;
        } else if ((op & 0xc0) == 0xc0) {
            _php_exec_open_tag(op);
            break;
        }
    }
    
    php.dribble = 0;
    
    return TRUE;
}

static void _php_default_params()
{
    memset(&php.params, 0, sizeof(php.params));
    memset(&php.vars, 0, sizeof(php.vars));
    
    php.params.page = 1;
    php.params.per_page = 50;
    php.params.reverse = TRUE;
    php.params.tab = 1;
}

static void _php_read_params_phase1(PacketBuf *params)
{
    char para_name[5];
    char para_val[20];
    tm dates[2];
    tm *date_ptr;
    char c, c2, c3;
    int32_t as_int;
    
    memset(dates, 0, 2 * sizeof(tm));
    dates[0].tm_mday = 1;
    dates[1].tm_mday = 1;
    
    while (http_parse_param(params, para_name, 5, para_val, 20)) {
        c = para_name[0];
        c2 = para_name[1];
        c3 = para_name[2];
        as_int = atol(para_val);
    
        if (c == 'C') {
            php.params.command = as_int;
        } else if (c == 'd') {
            if (((c2 == '0') || (c2 == '1')) && para_val[0] && (as_int > 0)) {
                date_ptr = dates + (c2 - '0');
                
                switch (c3) {
                    case 'y':
                        date_ptr->tm_year = as_int - 1900;
                        break;
                    case 'M':
                        date_ptr->tm_mon = as_int - 1;
                        break;
                    case 'd':
                        date_ptr->tm_mday = as_int;
                        break;
                    case 'h':
                        date_ptr->tm_hour = as_int;
                        break;
                    case 'm':
                        date_ptr->tm_min = as_int;
                        break;
                    case 's':
                        date_ptr->tm_sec = as_int;
                        break;
                }
            }
        }

        if (php.page_id == PHP_PAGE_CONFIG_PHP) {
            if ((c == 't') && (!para_name[1])) {
                php.params.tab = clamp(as_int, 1, 5);
            }
        } else {
            switch (c) {
                case 'n':
                    php.params.per_page = max(as_int, 5);
                    break;
                case 'p':
                    php.params.page = max(as_int, 1);
                    break;
                case 'r':
                    php.params.reverse = as_int;
                    break;
            }
        }
    }
    
    if (dates[0].tm_year) {
        php.params.date0 = time_convert_to_raw(&dates[0]);
    }
    if (dates[1].tm_year) {
        php.params.date1 = time_convert_to_raw(&dates[1]);
    }
}

static void _php_read_params_phase2(PacketBuf *params)
{
    char para_name[5];
    char para_val[20];
    char c, c2, c3;
    int32_t as_int;
    uint8_t plant_idx;
    mac_addr_t mac;
    ip_addr_t ip;
    
    while (http_parse_param(params, para_name, 5, para_val, 20)) {
        c = para_name[0];
        c2 = para_name[1]; // good thing there's no memory protection on the
        c3 = para_name[2]; // ATMEGA, or this could segfault :)
        as_int = atol(para_val);
    
        if ((php.page_id == PHP_PAGE_CONFIG_PHP) && (php.params.command == PHP_CMD_SAVE)) {
            switch (c) {
                case 'p':
                    plant_idx = c2-'1';
                    if (plant_idx >= CFG_MAX_PLANTS) {
                        break;
                    }
                    if (c3 == 'i') {
                        php.params.checkboxes |= _BV(plant_idx);
                    }
                    
                    as_int = clamp(as_int, 0, 100);
                    
                    if (c3 == 'l') {
                        cfg.plants[plant_idx].watering_start_threshold = as_int;
                    }
                    if (c3 == 'u') {
                        cfg.plants[plant_idx].watering_stop_threshold = as_int;
                    }
                    break;
                case 'r':
                    if (c2 == 'i') {
                        cfg.recording_interval = max(as_int, 1);
                    }
                    break;
                case 'n':
                    if ((c2 == 'm') && eth_parse_mac(&mac, para_val)) {
                        cfg.mac_addr = mac;
                    }
                    break;
                case 'a':
                    if ((c2 == 'i') && ip_parse(&ip, para_val)) {
                        cfg.alerts_server_ip = ip;
                    }
                    if (c2 == 'p') {
                        cfg.alerts_port = as_int;
                    }
                    break;
            }
        }
    }
}

static void _php_read_params(PacketBuf *params)
{
    PacketBuf backup = *params;
    
    _php_default_params();
    
    _php_read_params_phase1(params);
    _php_read_params_phase2(&backup);
}

/**
 * php_start - Feeds an URL to the dynamic page generation engine and
 *             potentially starts the content generation
 * @url_path: The path in the URL
 * @params: A packet buffer loaded with the url-encoded query parameters
 */
bool php_start(const char *url_path, PacketBuf *params)
{
    php.page_id = strset_find(PHP_PAGES, PHP_PAGES_COUNT, url_path + 1);
    if (php.page_id == PHP_PAGES_COUNT) {
        return FALSE;
    }
    
    php.content_type = HTTP_CONTENT_TYPE_TEXT_XML;
    php.download_filename = NULL;
    php.render_date = time_get_raw();
    
    php.pc = pgm_read_word(PHP_PAGES_ENTRY + php.page_id);
    php.truth = TRUE;
    php.node_stack_ptr = 0;
    php.call_stack_ptr = 0;
    php.tag_decl_open = FALSE;
    php.attr_open = FALSE;
    php.finished = FALSE;
    
    _php_read_params(params);
    _php_next_chunk();
    
    return TRUE;
}

/**
 * php_read - Reads from the generated page content
 * @buffer: A buffer to receive the page data
 * @length: The number of bytes to read
 * 
 * Returns the number of bytes actually read and stored.
 */
uint16_t php_read(void *buffer, uint16_t length)
{
    uint8_t chunk_len;
    uint16_t count, tot_count = 0;

    while (length) {
        chunk_len = strlen(php.chunk);

        count = length;
        if (count + php.dribble > chunk_len) {
            count = chunk_len - php.dribble;
        }
        
        memcpy(buffer, php.chunk + php.dribble, count);
        php.dribble += count;
        buffer += count;
        tot_count += count;
        length -= count;

        if (!count || (php.dribble == chunk_len))
            if (!_php_next_chunk())
                break;
    }

    return tot_count;
}

/**
 * php_eof - Checks for the end of the PHP output stream
 */
bool php_eof(void)
{
    return php.finished && (php.dribble == strlen(php.chunk));
}

/**
 * php_get_content_type - Gets the Content-Type for the generated page
 */
uint8_t php_get_content_type(void)
{
    return php.content_type;
}

/**
 * php_is_download - Checks whether we are generating a downloadable file
 */
bool php_is_download(void)
{
    return php.download_filename != NULL;
}

/**
 * php_get_download_filename - Gets the filename for the download
 * @buf: A buffer to receive the filename
 */
void php_get_download_filename(char *buf)
{
    strcpy_P(buf, php.download_filename);
}
