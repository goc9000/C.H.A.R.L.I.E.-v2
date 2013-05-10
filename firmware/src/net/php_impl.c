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
#include "storage/filesys.h"
#include "php_impl.h"
#include "charlie.h"

#define IS_PI(x) ((x==NAME_PI_XML) || (x==NAME_PI_XSL))

// Note: these are offsets in the PHP_TOKENS string
#define NAME_NONE                 0
#define NAME_PAGE                 1
#define NAME_ID                   6
#define NAME_DATETIME             9
#define NAME_CURRENT             18
#define NAME_LOG                 26
#define NAME_RECORDS             30
#define NAME_PLANT               38
#define NAME_HUMID               44
#define NAME_ILUM                50
#define NAME_ROW                 55
#define NAME_EVENT               59
#define NAME_CODE                65
#define NAME_DATA                70
#define NAME_VERSION             75
#define NAME_ENCODING            83
#define NAME_TYPE                92
#define NAME_HREF                97
#define NAME_FROM               102
#define NAME_TO                 107
#define NAME_PAGES              110
#define NAME_PERPAGE            116
#define NAME_REVERSE            124
#define NAME_CONFIG             132
#define NAME_PLANTS             139
#define NAME_RECORDING          146
#define NAME_NET                156
#define NAME_ALERTS             160
#define NAME_TIMESVR            167
#define NAME_MIN                175
#define NAME_MAX                179
#define NAME_FLAGS              183
#define NAME_INTERVAL           189
#define NAME_MAC                198
#define NAME_IP                 202
#define NAME_PORT               205
#define NAME_TZDELTA            210
#define NAME_PI_XML             218
#define NAME_PI_XSL             223
#define NAME_TAB                239
#define NAME_EXECUTED           243

static const char PHP_TOKENS[] PROGMEM =
    "\0page\0id\0datetime\0current\0log\0records\0plant\0humid\0ilum\0"
    "row\0event\0code\0data\0version\0encoding\0type\0href\0from\0to\0"
    "pages\0perPage\0reverse\0config\0plants\0recording\0net\0alerts\0"
    "timesvr\0min\0max\0flags\0interval\0mac\0ip\0port\0tzdelta\0"
    "?xml\0?xml-stylesheet\0tab\0executed\0";

#define TOK(x) (PHP_TOKENS + (x))

#define PHP_PAGES_COUNT           5

#define PHP_PAGE_HEADER           0
#define PHP_PAGE_HOME             1
#define PHP_PAGE_RECORDS          2
#define PHP_PAGE_LOG              3
#define PHP_PAGE_CONFIG           4
#define PHP_PAGE_NONE             5

static const char PHP_PAGES[] PROGMEM =
    "/header.php\0"
    "/home.php\0"
    "/records.php\0"
    "/log.php\0"
    "/config.php";

static const char PHP_PAGE_IDS[] PROGMEM =
    "headerPage\0"
    "homePage\0"
    "recordsPage\0"
    "logPage\0"
    "configPage\0";

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

#define PHP_STAT_OPEN_NODE        0
#define PHP_STAT_READ_ATTR        1
#define PHP_STAT_OPEN_ATTR        2
#define PHP_STAT_READ_ATTR_VAL    3
#define PHP_STAT_END_ATTR         4
#define PHP_STAT_READ_CHILDREN    5
#define PHP_STAT_READ_TEXT        6
#define PHP_STAT_CLOSE_NODE       7
#define PHP_STAT_DONE             8

struct {
    uint8_t dribble;

    uint8_t state;
    uint8_t xml_stack[12];
    uint8_t xml_level;

    uint8_t page;
    struct {
        uint16_t page;
        uint16_t per_page;
        time_t from_date;
        time_t to_date;
        bool reverse:1;
        uint8_t tab;
        uint8_t command;
        uint8_t checkboxes;
    } params;

    struct {
        bool export:1;
        bool first:1;
        uint8_t col;
    } csv;
    
    uint8_t plant_idx;
    Query query;
    union {
        Record record;
        LogEntry event;
    } entry;

    struct {
        uint16_t pages;
    } vars;
} php;

#define MAX_INT32 0x7FFFFFFF

static inline int32_t _limit(int32_t value, int32_t min_val, int32_t max_val) {
    return (value < min_val)
        ? min_val
        : ((value > max_val) ? max_val : value);
}

static void _php_get_xml_node_val(char *buf)
{
    buf[0] = 0;
}

static void _php_get_xml_attr_val(char *buf)
{
    time_t time = 0;
    uint8_t node = php.xml_level
        ? php.xml_stack[php.xml_level-1] : NAME_NONE;
    uint8_t attr = php.xml_stack[php.xml_level];
    
    buf[0] = 0;
    switch (attr) {
        case NAME_ID:
            strcpy_P(buf, strset_get(PHP_PAGE_IDS, php.page));
            break;
        case NAME_HUMID:
            itoa10(php.entry.record.plants[php.plant_idx].humidity / 2, buf);
            break;
        case NAME_ILUM:
            itoa10(php.entry.record.plants[php.plant_idx].ilumination / 2, buf);
            break;
        case NAME_MIN:
            itoa10(cfg.plants[php.plant_idx].watering_start_threshold / 2, buf);
            break;
        case NAME_MAX:
            itoa10(cfg.plants[php.plant_idx].watering_stop_threshold / 2, buf);
            break;
        case NAME_FLAGS:
            itoa10(cfg.plants[php.plant_idx].flags, buf);
            break;
        case NAME_TZDELTA:
            itoa10(cfg.timezone_delta, buf);
            break;
        case NAME_EXECUTED:
            itoa10(php.params.command, buf);
            break;
        case NAME_PORT:
            ltoa10(cfg.alerts_port, buf);
            break;
        case NAME_DATETIME:
            switch (node) {
                case NAME_PAGE:
                    time = time_get_raw();
                    break;
                case NAME_ROW:
                    time = php.entry.record.time;
                    break;
                case NAME_EVENT:
                    time = php.entry.event.time;
                    break;
            }
            time_format_rfc3339(buf, time);
            break;
        case NAME_CODE:
            ltoa10(php.entry.event.code, buf);
            break;
        case NAME_DATA:
            ltoa10(php.entry.event.data, buf);
            break;
        case NAME_VERSION:
            strcpy_P(buf, PSTR("1.0"));
            break;
        case NAME_ENCODING:
            strcpy_P(buf, PSTR("UTF-8"));
            break;
        case NAME_TYPE:
            strcpy_P(buf, PSTR("text/xsl"));
            break;
        case NAME_HREF:
            strcpy_P(buf, PSTR("main.xsl"));
            break;
        case NAME_PAGE:
            itoa10(php.params.page, buf);
            break;
        case NAME_PAGES:
            itoa10(php.vars.pages, buf);
            break;
        case NAME_PERPAGE:
            itoa10(php.params.per_page, buf);
            break;
        case NAME_REVERSE:
            itoa10(php.params.reverse, buf);
            break;
        case NAME_TAB:
            itoa10(php.params.tab, buf);
            break;
        case NAME_INTERVAL:
            ltoa10(cfg.recording_interval, buf);
            break;
        case NAME_FROM:
            time_format_rfc3339(buf, php.params.from_date);
            break;
        case NAME_TO:
            time_format_rfc3339(buf, php.params.to_date);
            break;
        case NAME_IP:
            ip_format(buf, (node == NAME_ALERTS)
                ? (&cfg.alerts_server_ip) : (&cfg.time_server_ip));
            break;
        case NAME_MAC:
            eth_format_mac(buf, &cfg.mac_addr);
            break;
    }
}

static void _php_get_xml_token(char *buf)
{
    uint8_t pre_top = php.xml_level
        ? php.xml_stack[php.xml_level - 1] : NAME_NONE;
    uint8_t top = php.xml_stack[php.xml_level];

    buf[0] = 0;
    switch (php.state) {
        case PHP_STAT_OPEN_NODE:
            buf[0] = '<';
            strcpy_P(buf + 1, TOK(pre_top));
            break;
        case PHP_STAT_OPEN_ATTR:
            buf[0] = ' ';
            strcpy_P(buf + 1, TOK(top));
            strcat_P(buf, PSTR("=\""));
            break;
        case PHP_STAT_READ_ATTR_VAL:
            _php_get_xml_attr_val(buf);
            strcat_P(buf, PSTR("\""));
            break;
        case PHP_STAT_END_ATTR:
            if (IS_PI(pre_top)) {
                strcpy_P(buf, PSTR("?>"));
                break;
            }
            strcpy_P(buf, PSTR(">"));
            break;
        case PHP_STAT_READ_TEXT:
            _php_get_xml_node_val(buf);
            break;
        case PHP_STAT_CLOSE_NODE:
            if (!pre_top || IS_PI(pre_top)) {
                break;
            }

            buf[0] = '<';
            buf[1] = '/';
            strcpy_P(buf + 2, TOK(pre_top));
            strcat_P(buf, PSTR(">"));
            break;
    }
}

static bool _php_next_xml_child(void)
{
    uint8_t parent = php.xml_level
        ? php.xml_stack[php.xml_level - 1] : NAME_NONE;
    uint8_t prev_sibling = php.xml_stack[php.xml_level];
    uint8_t node = NAME_NONE;
    Query *query = &(php.query);
    
    switch (parent) {
        case NAME_NONE:
            switch (prev_sibling) {
                case NAME_NONE:
                    node = NAME_PI_XML;
                    break;
                case NAME_PI_XML:
                    node = NAME_PI_XSL;
                    break;
                case NAME_PI_XSL:
                    node = NAME_PAGE;
                    break;
            }
            break;
        case NAME_PAGE:
            switch (php.page) {
                case PHP_PAGE_HOME:
                    switch (prev_sibling) {
                        case NAME_NONE:
                            node = NAME_CURRENT;
                            plants_get_latest_record(&(php.entry.record));
                            break;
                        case NAME_CURRENT:
                            node = NAME_LOG;
                            log_get_query(query);
                            query_reverse(query);
                            query_limit(query,5);
                            break;
                        case NAME_LOG:
                            node = NAME_RECORDS;
                            rec_get_query(query);
                            query_reverse(query);
                            query_limit(query,5);
                            break;
                    }
                    break;
                case PHP_PAGE_RECORDS:
                    if (!prev_sibling) {
                        node = NAME_RECORDS;
                    }
                    break;
                case PHP_PAGE_LOG:
                    if (!prev_sibling) {
                        node = NAME_LOG;
                    }
                    break;
                case PHP_PAGE_CONFIG:
                    if (!prev_sibling) {
                        node = NAME_CONFIG;
                    }
                    break;
            }
            break;
        case NAME_CURRENT:
        case NAME_ROW:
        case NAME_PLANTS:
            php.plant_idx = (prev_sibling == NAME_PLANT)
                ? (php.plant_idx + 1) : 0;
            node = (php.plant_idx < CFG_MAX_PLANTS)
                ? NAME_PLANT : NAME_NONE;
            break;
        case NAME_RECORDS:
            node = query_get(query, &php.entry.record) ? NAME_ROW : NAME_NONE;
            query_next(query);
            break;
        case NAME_LOG:
            node = query_get(query, &php.entry.event) ? NAME_EVENT : NAME_NONE;
            query_next(query);
            break;
        case NAME_CONFIG:
            switch (prev_sibling) {
                case NAME_NONE:
                    node = NAME_PLANTS;
                    break;
                case NAME_PLANTS:
                    node = NAME_RECORDING;
                    break;
                case NAME_RECORDING:
                    node = NAME_NET;
                    break;
                case NAME_NET:
                    node = NAME_ALERTS;
                    break;
                case NAME_ALERTS:
                    node = NAME_TIMESVR;
                    break;
            }
            break;
    }

    if (!node) {
        return FALSE;
    }

    php.xml_stack[php.xml_level++] = node;
    php.xml_stack[php.xml_level] = NAME_NONE;

    return TRUE;
}

static bool _php_next_xml_attr(void)
{
    uint8_t node = php.xml_level
        ? php.xml_stack[php.xml_level - 1] : NAME_NONE;
    uint8_t prev_attr = php.xml_stack[php.xml_level];
    uint8_t attr = NAME_NONE;

    switch (node) {
        case NAME_PI_XML:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_VERSION;
                    break;
                case NAME_VERSION:
                    attr = NAME_ENCODING;
                    break;
            }
            break;
        case NAME_PI_XSL:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_TYPE;
                    break;
                case NAME_TYPE:
                    attr = NAME_HREF;
                    break;
            }
            break;
        case NAME_PAGE:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_ID;
                    break;
                case NAME_ID:
                    attr = NAME_DATETIME;
                    break;
                case NAME_DATETIME:
                    attr = php.params.command ? NAME_EXECUTED : NAME_NONE;
                    break;
            }
            break;
        case NAME_PLANT:
            if (php.page == PHP_PAGE_CONFIG) {
                switch (prev_attr) {
                    case NAME_NONE:
                        attr = NAME_MIN;
                        break;
                    case NAME_MIN:
                        attr = NAME_MAX;
                        break;
                    case NAME_MAX:
                        attr = NAME_FLAGS;
                        break;
                }
                break;
            }
            switch (prev_attr) {
                case NAME_NONE:
                    attr = (php.entry.record.plants[php.plant_idx].flags
                        & PLANT_FLAGS_NOT_INSTALLED)
                        ? NAME_NONE : NAME_HUMID;
                    break;
                case NAME_HUMID:
                    attr = NAME_ILUM;
                    break;
            }
            break;
        case NAME_ROW:
            attr = (prev_attr == NAME_NONE) ? NAME_DATETIME : NAME_NONE; 
            break;
        case NAME_EVENT:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_DATETIME;
                    break;
                case NAME_DATETIME:
                    attr = NAME_CODE;
                    break;
                case NAME_CODE:
                    attr = NAME_DATA;
                    break;
            }
            break;
        case NAME_RECORDS:
        case NAME_LOG:
            if (php.page == PHP_PAGE_HOME) {
                break;
            }
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_PAGE;
                    break;
                case NAME_PAGE:
                    attr = NAME_PAGES;
                    break;
                case NAME_PAGES:
                    attr = NAME_PERPAGE;
                    break;
                case NAME_PERPAGE:
                    attr = NAME_FROM;
                    break;
                case NAME_FROM:
                    attr = NAME_TO;
                    break;
                case NAME_TO:
                    attr = NAME_REVERSE;
                    break;
            }
            break;
        case NAME_RECORDING:
            attr = (prev_attr == NAME_NONE) ? NAME_INTERVAL : NAME_NONE;
            break;
        case NAME_NET:
            attr = (prev_attr == NAME_NONE) ? NAME_MAC : NAME_NONE;
            break;
        case NAME_CONFIG:
            attr = (prev_attr == NAME_NONE) ? NAME_TAB : NAME_NONE;
            break;
        case NAME_ALERTS:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_IP;
                    break;
                case NAME_IP:
                    attr = NAME_PORT;
                    break;
            }
            break;
        case NAME_TIMESVR:
            switch (prev_attr) {
                case NAME_NONE:
                    attr = NAME_IP;
                    break;
                case NAME_IP:
                    attr = NAME_TZDELTA;
                    break;
            }
            break;
    }

    php.xml_stack[php.xml_level] = attr;
    
    if (!attr) {
        return FALSE;
    }

    return TRUE;
}

static void _php_next_xml_token(void)
{
    switch (php.state) {
        case PHP_STAT_READ_TEXT:
            php.state = PHP_STAT_READ_CHILDREN;
            break;
        case PHP_STAT_READ_CHILDREN:
            php.state = _php_next_xml_child()
                ? PHP_STAT_OPEN_NODE : PHP_STAT_CLOSE_NODE;
            break;
        case PHP_STAT_OPEN_NODE:
            php.state = PHP_STAT_READ_ATTR;
            break;
        case PHP_STAT_OPEN_ATTR:
            php.state = PHP_STAT_READ_ATTR_VAL;
            break;
        case PHP_STAT_CLOSE_NODE:
            if (!php.xml_level) {
                php.state = PHP_STAT_DONE;
                return;
            }
            php.xml_level--;
            php.state = PHP_STAT_READ_TEXT;
            break;
        case PHP_STAT_READ_ATTR:
            php.state = _php_next_xml_attr()
                ? PHP_STAT_OPEN_ATTR : PHP_STAT_END_ATTR;
            break;
        case PHP_STAT_END_ATTR:
            php.state = PHP_STAT_READ_TEXT;
            break;
        case PHP_STAT_READ_ATTR_VAL:
            php.state = PHP_STAT_READ_ATTR;
            break;
    }
}

static inline uint8_t _php_csv_col_count(void)
{
    return  (php.page == PHP_PAGE_RECORDS) ? (1 + 3 * CFG_MAX_PLANTS) : 3;
}

static void _php_format_date_csv(time_t datetime, char *buffer)
{
    time_format_rfc3339(buffer, datetime);
    buffer[10] = ' ';
}

static void _php_get_csv_header_token(char *buf)
{
    uint8_t aspect;
    
    *buf = 0;
    if (php.page == PHP_PAGE_RECORDS) {
        // Records page
        if (php.csv.col > 0) {
            aspect = (php.csv.col - 1) % 3;

            strcpy_P(buf, strset_get(
                PSTR("Humid 0 [%]\0Ilum # [%]\0Info"), aspect));
            *strchr(buf, '0') = '1' + (php.csv.col - 1) / 3;
        } else {
            strcpy_P(buf, PSTR("Date"));
        }
    } else {
        // Log page
        strcpy_P(buf, strset_get(PSTR("Date\0Type\0Message"), php.csv.col));
    }
}

static void _php_get_csv_body_token(char *buf)
{
    uint8_t aspect;
    PlantStatus *plant;
    
    *buf = 0;
    if (php.page == PHP_PAGE_RECORDS) {
        // Records page
        if (php.csv.col > 0) {
            aspect = (php.csv.col - 1) % 3;
            plant = php.entry.record.plants + ((php.csv.col - 1) / 3);
            
            if (aspect < 2) {
                if (!(plant->flags & PLANT_FLAGS_NOT_INSTALLED))
                    itoa10((aspect ? plant->ilumination : plant->humidity)/2, buf);
            }
        } else {
            _php_format_date_csv(php.entry.record.time, buf);
        }
    } else {
        // Log page
        switch (php.csv.col) {
            case 0:
                _php_format_date_csv(php.entry.event.time, buf);
                break;
            case 1:
                strcpy_P(buf, strset_get(PSTR("Info\0Note\0Error\0\0"),
                    (php.entry.event.code >> 6) & 3));
                break;
            case 2:
                log_format_message(buf, &php.entry.event);
                break;
        }
    }
}

static void _php_get_csv_token(char *buf)
{
    uint8_t tok_len;

    if (php.csv.first) {
        _php_get_csv_header_token(buf);
    } else {
        _php_get_csv_body_token(buf);
    }

    tok_len = strlen(buf);
    if (php.csv.col == _php_csv_col_count() - 1) {
        buf[tok_len++] = '\r';
        buf[tok_len++] = '\n';
    } else {
        buf[tok_len++] = ',';
    }
    buf[tok_len] = 0;
}

static void _php_next_csv_token(void)
{
    void *target;
    
    php.csv.col++;
    if (php.csv.col >= _php_csv_col_count()) {
        target = (php.page == PHP_PAGE_RECORDS)
            ? (void *)&php.entry.record : (void *)&php.entry.event;
        if (!query_get(&(php.query), target)) {
            php.state = PHP_STAT_DONE;
        }
        query_next(&(php.query));
        
        php.csv.first = FALSE;
        php.csv.col = 0;
    }
}

static void _php_prepare_query(void)
{
    Query *query = &(php.query);
    
    if (php.page == PHP_PAGE_LOG) {
        log_get_query(query);
    } else {
        rec_get_query(query);
    }
    
    if (php.params.from_date) {
        query_filter_after(query, php.params.from_date);
    }
    if (php.params.to_date) {
        query_filter_before(query,
            php.params.to_date + TIME_SECONDS_PER_DAY - 1);
    }
    if (php.params.reverse) {
        query_reverse(query);
    }
    if (!php.csv.export) {
        query_paginate(query, php.params.per_page,
            &(php.params.page), &(php.vars.pages));
    }
}

static void _php_execute(void)
{
    uint8_t i;
    
    switch (php.page) {
        case PHP_PAGE_RECORDS:
        case PHP_PAGE_LOG:
            if (php.params.command == PHP_CMD_EXPORT) {
                php.csv.export = TRUE;
                php.csv.col = 0;
                php.csv.first = TRUE;
            }
            _php_prepare_query();
            break;
        case PHP_PAGE_CONFIG:
            switch (php.params.tab) {
                case PHP_TAB_PLANTS:
                    for (i = 0; i < CFG_MAX_PLANTS; i++) {
                        if (php.params.checkboxes & _BV(i)) {
                            cfg.plants[i].flags &= ~PLANT_CFG_FLAG_NOT_INSTALLED;
                        } else {
                            cfg.plants[i].flags |= ~PLANT_CFG_FLAG_NOT_INSTALLED;
                        }
                    }
                    break;
                case PHP_TAB_RESET:
                    switch (php.params.command) {
                        case PHP_CMD_SHUTDOWN:
                            shutdown(FALSE);
                            break;
                        case PHP_CMD_RESTART:
                            shutdown(TRUE);
                            break;
                        case PHP_CMD_PURGE_LOG:
                            log_purge();
                            break;
                        case PHP_CMD_PURGE_RECORDS:
                            rec_purge();
                            break;
                    }
                    break;
                default:
                    if (php.params.command == PHP_CMD_SAVE) {
                        cfg_save();
                    }
                    break;
            }
    }
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
    tm date;
    char c;
    int32_t as_int;
    uint16_t from_year = 0, to_year = 0;
    uint8_t from_month = 0, from_day = 0, to_month = 0, to_day = 0;
    
    while (http_parse_param(params, para_name, 5, para_val, 20)) {
        c = para_name[0];
        as_int = atol(para_val);
    
        if (c == 'C') {
            php.params.command = as_int;
        }

        if (php.page == PHP_PAGE_CONFIG) {
            if ((c == 't') && (!para_name[1])) {
                php.params.tab = _limit(as_int, 1, 5);
            }
        } else {
            switch (c) {
                case 'n':
                    php.params.per_page = _limit(as_int, 5, MAX_INT32);
                    break;
                case 'p':
                    php.params.page = _limit(as_int, 1, MAX_INT32);
                    break;
                case 'r':
                    php.params.reverse = !as_int;
                    break;
                case 'Y':
                    from_year = as_int;
                    break;
                case 'M':
                    from_month = as_int;
                    break;
                case 'D':
                    from_day = as_int;
                    break;
                case 'y':
                    to_year = as_int;
                    break;
                case 'm':
                    to_month = as_int;
                    break;
                case 'd':
                    to_day = as_int;
                    break;
            }
        }
    }
    
    if (from_year && from_month && from_day) {
        memset(&date, 0, sizeof(tm));
        date.tm_year = from_year - 1900;
        date.tm_mon = from_month - 1;
        date.tm_mday = from_day;
        php.params.from_date = time_convert_to_raw(&date);
    }
    if (to_year && to_month && to_day) {
        memset(&date, 0, sizeof(tm));
        date.tm_year = to_year - 1900;
        date.tm_mon = to_month - 1;
        date.tm_mday = to_day;
        php.params.to_date = time_convert_to_raw(&date);
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
    
        if ((php.page == PHP_PAGE_CONFIG) && (php.params.command == PHP_CMD_SAVE)) {
            switch (c) {
                case 'p':
                    plant_idx = c2-'1';
                    if (plant_idx >= CFG_MAX_PLANTS) {
                        break;
                    }
                    if (c3 == 'i') {
                        php.params.checkboxes |= _BV(plant_idx);
                    }
                    
                    as_int = _limit(as_int, 0, 100);
                    
                    if (c3 == 'l') {
                        cfg.plants[plant_idx].watering_start_threshold = as_int << 1;
                    }
                    if (c3 == 'u') {
                        cfg.plants[plant_idx].watering_stop_threshold = as_int << 1;
                    }
                    break;
                case 'r':
                    if (c2 == 'i') {
                        cfg.recording_interval = _limit(as_int, 1, MAX_INT32);
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
                case 't':
                    if ((c2 == 'i') && ip_parse(&ip, para_val)) {
                        cfg.time_server_ip = ip;
                    }
                    if (c2 == 'z') {
                        cfg.timezone_delta = as_int;
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
    uint8_t page = strset_find(PHP_PAGES, PHP_PAGES_COUNT, url_path);
    if (page == PHP_PAGE_NONE) {
        return FALSE;
    }
    
    php.page = page;
    php.csv.export = FALSE;
    php.dribble = 0;
    php.state = PHP_STAT_READ_TEXT;
    _php_read_params(params);
    _php_execute();
    php.xml_level = 0;
    php.xml_stack[0] = NAME_NONE;
    
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
    char buf[48];
    uint8_t tok_len;
    uint16_t count, tot_count = 0;

    if (php.state == PHP_STAT_DONE) {
        return 0;
    }

    while (length) {
        if (php.csv.export) {
            _php_get_csv_token(buf);
        } else {
            _php_get_xml_token(buf);
        }
        tok_len = strlen(buf);

        count = length;
        if (count + php.dribble > tok_len) {
            count = tok_len - php.dribble;
        }
        
        memcpy(buffer, buf + php.dribble, count);
        php.dribble += count;
        buffer += count;
        tot_count += count;
        length -= count;

        if (!count || (php.dribble == tok_len)) {
            if (php.csv.export) {
                _php_next_csv_token();
            } else {
                _php_next_xml_token();
            }
            
            php.dribble = 0;
            if (php.state == PHP_STAT_DONE) {
                break;
            }
        }
    }

    return tot_count;
}

/**
 * php_eof - Checks for the end of the PHP output stream
 */
bool php_eof(void)
{
    return (php.state == PHP_STAT_DONE);
}

/**
 * php_get_content_type - Gets the Content-Type for the generated page
 */
uint8_t php_get_content_type(void)
{
    return php.csv.export ? HTTP_CONTENT_TYPE_TEXT_CSV : HTTP_CONTENT_TYPE_TEXT_XML;
}

/**
 * php_is_download - Checks whether we are generating a downloadable file
 */
bool php_is_download(void)
{
    return php.csv.export;
}

/**
 * php_get_download_filename - Gets the filename for the download
 * @buf: A buffer to receive the filename
 */
void php_get_download_filename(char *buf)
{
    strcpy_P(buf, (php.page == PHP_PAGE_RECORDS) ?
        PSTR("records.csv") : PSTR("log.csv"));
}
