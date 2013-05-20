/*
 * php_code.h - Compiled "PHP" code
 *
 * (C) Copyright 2009-2013  Cristian Dinu <goc9000@gmail.com>
 *
 * Licensed under the GPL-3.
 *
 * THIS FILE IS AUTOGENERATED. DO NOT EDIT MANUALLY!
 */

#define PHP_PAGES_COUNT                          5

static char const PHP_PAGES[] PROGMEM =
    "config.php\0"
    "header.php\0"
    "home.php\0"
    "log.php\0"
    "records.php\0";

#define PHP_PAGE_CONFIG_PHP                      0
#define PHP_PAGE_HEADER_PHP                      1
#define PHP_PAGE_HOME_PHP                        2
#define PHP_PAGE_LOG_PHP                         3
#define PHP_PAGE_RECORDS_PHP                     4

static uint16_t const PHP_PAGES_ENTRY[] PROGMEM = {
    (uint16_t)0,
    (uint16_t)56,
    (uint16_t)62,
    (uint16_t)81,
    (uint16_t)123
};

static char const PHP_NODES[] PROGMEM =
    "\0"
    "?xml\0"
    "?xml-stylesheet\0"
    "alerts\0"
    "config\0"
    "current\0"
    "event\0"
    "log\0"
    "net\0"
    "page\0"
    "plant\0"
    "plants\0"
    "recording\0"
    "records\0"
    "row\0";

#define PHP_NODE_NONE                            0
#define PHP_NODE_PI_XML                          1
#define PHP_NODE_PI_XML_STYLESHEET               2
#define PHP_NODE_ALERTS                          3
#define PHP_NODE_CONFIG                          4
#define PHP_NODE_CURRENT                         5
#define PHP_NODE_EVENT                           6
#define PHP_NODE_LOG                             7
#define PHP_NODE_NET                             8
#define PHP_NODE_PAGE                            9
#define PHP_NODE_PLANT                           10
#define PHP_NODE_PLANTS                          11
#define PHP_NODE_RECORDING                       12
#define PHP_NODE_RECORDS                         13
#define PHP_NODE_ROW                             14

static char const PHP_ATTRS[] PROGMEM =
    "\0"
    "code\0"
    "data\0"
    "datetime\0"
    "encoding\0"
    "executed\0"
    "flags\0"
    "from\0"
    "href\0"
    "humid\0"
    "id\0"
    "ilum\0"
    "interval\0"
    "ip\0"
    "mac\0"
    "max\0"
    "min\0"
    "page\0"
    "pages\0"
    "perPage\0"
    "port\0"
    "reverse\0"
    "tab\0"
    "to\0"
    "type\0"
    "version\0";

#define PHP_ATTR_NONE                            0
#define PHP_ATTR_CODE                            1
#define PHP_ATTR_DATA                            2
#define PHP_ATTR_DATETIME                        3
#define PHP_ATTR_ENCODING                        4
#define PHP_ATTR_EXECUTED                        5
#define PHP_ATTR_FLAGS                           6
#define PHP_ATTR_FROM                            7
#define PHP_ATTR_HREF                            8
#define PHP_ATTR_HUMID                           9
#define PHP_ATTR_ID                              10
#define PHP_ATTR_ILUM                            11
#define PHP_ATTR_INTERVAL                        12
#define PHP_ATTR_IP                              13
#define PHP_ATTR_MAC                             14
#define PHP_ATTR_MAX                             15
#define PHP_ATTR_MIN                             16
#define PHP_ATTR_PAGE                            17
#define PHP_ATTR_PAGES                           18
#define PHP_ATTR_PERPAGE                         19
#define PHP_ATTR_PORT                            20
#define PHP_ATTR_REVERSE                         21
#define PHP_ATTR_TAB                             22
#define PHP_ATTR_TO                              23
#define PHP_ATTR_TYPE                            24
#define PHP_ATTR_VERSION                         25

static char const PHP_STRINGS[] PROGMEM =
    "\0"
    "\n\0"
    ",\0"
    ",,\0"
    ",Humid \0"
    ",Ilum \0"
    ",Info \0"
    "1.0\0"
    "Date\0"
    "Date,Type,Message\0"
    "UTF-8\0"
    "configPage\0"
    "headerPage\0"
    "homePage\0"
    "logPage\0"
    "main.xsl\0"
    "recordsPage\0"
    "text/xsl\0";

#define PHP_STR_NONE                             0
#define PHP_STR__                                1
#define PHP_STR__2                               2
#define PHP_STR___                               3
#define PHP_STR__HUMID_                          4
#define PHP_STR__ILUM_                           5
#define PHP_STR__INFO_                           6
#define PHP_STR_1_0                              7
#define PHP_STR_DATE                             8
#define PHP_STR_DATE_TYPE_MESSAG                 9
#define PHP_STR_UTF_8                            10
#define PHP_STR_CONFIGPAGE                       11
#define PHP_STR_HEADERPAGE                       12
#define PHP_STR_HOMEPAGE                         13
#define PHP_STR_LOGPAGE                          14
#define PHP_STR_MAIN_XSL                         15
#define PHP_STR_RECORDSPAGE                      16
#define PHP_STR_TEXT_XSL                         17

static void * const PHP_VARS[] PROGMEM = {
    (void *)&cfg.alerts_port,
    (void *)&cfg.alerts_server_ip,
    (void *)&cfg.mac_addr,
    (void *)&cfg.recording_interval,
    (void *)&php.params.command,
    (void *)&php.params.date0,
    (void *)&php.params.date1,
    (void *)&php.params.page,
    (void *)&php.params.per_page,
    (void *)&php.params.reverse,
    (void *)&php.params.tab,
    (void *)&php.render_date,
    (void *)&php.vars.entry.event,
    (void *)&php.vars.entry.event.code,
    (void *)&php.vars.entry.event.data,
    (void *)&php.vars.entry.event.time,
    (void *)&php.vars.entry.record.time,
    (void *)&php.vars.pages,
    (void *)&php.vars.plant.cfg.flags,
    (void *)&php.vars.plant.cfg.watering_start_threshold,
    (void *)&php.vars.plant.cfg.watering_stop_threshold,
    (void *)&php.vars.plant.status.humidity,
    (void *)&php.vars.plant.status.ilumination,
    (void *)&php.vars.plant_idx
};

#define PHP_VAR_CFG_ALERTS_PORT                  0
#define PHP_VAR_CFG_ALERTS_SERVER_IP             1
#define PHP_VAR_CFG_MAC_ADDR                     2
#define PHP_VAR_CFG_RECORDING_INTERVAL           3
#define PHP_VAR_PARAMS_COMMAND                   4
#define PHP_VAR_PARAMS_DATE0                     5
#define PHP_VAR_PARAMS_DATE1                     6
#define PHP_VAR_PARAMS_PAGE                      7
#define PHP_VAR_PARAMS_PER_PAGE                  8
#define PHP_VAR_PARAMS_REVERSE                   9
#define PHP_VAR_PARAMS_TAB                       10
#define PHP_VAR_RENDER_DATE                      11
#define PHP_VAR_EVENT                            12
#define PHP_VAR_EVENT_CODE                       13
#define PHP_VAR_EVENT_DATA                       14
#define PHP_VAR_EVENT_TIME                       15
#define PHP_VAR_RECORD_TIME                      16
#define PHP_VAR_PAGES                            17
#define PHP_VAR_PLANT_CFG_FLAGS                  18
#define PHP_VAR_PLANT_CFG_WATERING_START_THRESH  19
#define PHP_VAR_PLANT_CFG_WATERING_STOP_THRESH   20
#define PHP_VAR_PLANT_STATUS_HUMIDITY            21
#define PHP_VAR_PLANT_STATUS_ILUMINATION         22
#define PHP_VAR_PLANT_IDX                        23

static php_conv_fn_t const PHP_CONVERSIONS[] PROGMEM = {
    (php_conv_fn_t)_php_format_datetime,
    (php_conv_fn_t)_php_format_datetime_csv,
    (php_conv_fn_t)_php_format_event_message,
    (php_conv_fn_t)_php_format_event_type,
    (php_conv_fn_t)_php_format_ip,
    (php_conv_fn_t)_php_format_mac,
    (php_conv_fn_t)_php_format_uint16,
    (php_conv_fn_t)_php_format_uint32,
    (php_conv_fn_t)_php_format_uint8
};

#define PHP_CONV_DATETIME                        0
#define PHP_CONV_DATETIME_CSV                    1
#define PHP_CONV_EVENT_MESSAGE                   2
#define PHP_CONV_EVENT_TYPE                      3
#define PHP_CONV_IP                              4
#define PHP_CONV_MAC                             5
#define PHP_CONV_UINT16                          6
#define PHP_CONV_UINT32                          7
#define PHP_CONV_UINT8                           8

static bool _php_routine_9(void) {
    if (php.params.date0)
        query_filter_after(&php.vars.query, php.params.date0);
    
    if (php.params.date1)
        query_filter_before(&php.vars.query, php.params.date1 + TIME_SECONDS_PER_DAY - 1);
    
    if (php.params.reverse)
        query_reverse(&php.vars.query); return TRUE;
}

static bool _php_routine_6(void) {
    log_get_query(&php.vars.query); return TRUE;
}

static bool _php_routine_11(void) {
    php.content_type = HTTP_CONTENT_TYPE_TEXT_CSV; return TRUE;
}

static bool _php_routine_12(void) {
    php.download_filename = PSTR("log.csv"); return TRUE;
}

static bool _php_routine_16(void) {
    php.download_filename = PSTR("records.csv"); return TRUE;
}

static bool _php_routine_4(void) {
    php.vars.plant.cfg = cfg.plants[php.vars.plant_idx++]; return TRUE;
}

static bool _php_routine_19(void) {
    php.vars.plant.status = php.vars.entry.record.plants[php.vars.plant_idx++]; return TRUE;
}

static bool _php_routine_2(void) {
    php.vars.plant_idx = 0; return TRUE;
}

static bool _php_routine_5(void) {
    plants_get_latest_record(&php.vars.entry.record); return TRUE;
}

static bool _php_routine_14(void) {
    query_next(&php.vars.query); return TRUE;
}

static bool _php_routine_15(void) {
    query_paginate(&php.vars.query, php.params.per_page, &php.params.page, &php.vars.pages); return TRUE;
}

static bool _php_routine_7(void) {
    query_reverse(&php.vars.query); query_limit(&php.vars.query, 5); return TRUE;
}

static bool _php_routine_8(void) {
    rec_get_query(&php.vars.query); return TRUE;
}

static bool _php_routine_20(void) {
    return !(!(php.vars.plant.status.flags & PLANT_FLAGS_NOT_INSTALLED));
}

static bool _php_routine_10(void) {
    return !(php.params.command == PHP_CMD_EXPORT);
}

static bool _php_routine_1(void) {
    return !(php.params.command);
}

static bool _php_routine_3(void) {
    return !(php.vars.plant_idx < CFG_MAX_PLANTS);
}

static bool _php_routine_17(void) {
    return !(php.vars.plant_idx++ < CFG_MAX_PLANTS);
}

static bool _php_routine_13(void) {
    return !(query_get(&php.vars.query, &php.vars.entry.event));
}

static bool _php_routine_18(void) {
    return !(query_get(&php.vars.query, &php.vars.entry.record));
}

static bool _php_routine_0(void) {
    uint8_t i;
    
    if (php.params.tab == PHP_TAB_RESET) {
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
    } else if (php.params.command == PHP_CMD_SAVE) {
        if (php.params.tab == PHP_TAB_PLANTS) {
            for (i = 0; i < CFG_MAX_PLANTS; i++) {
                if (php.params.checkboxes & _BV(i)) {
                    cfg.plants[i].flags &= ~PLANT_CFG_FLAG_NOT_INSTALLED;
                } else {
                    cfg.plants[i].flags |= PLANT_CFG_FLAG_NOT_INSTALLED;
                }
            }
        } else if (php.params.tab == PHP_TAB_TIME) {
            time_set_raw(php.params.date0);
            php.render_date = php.params.date0;
            rtc_set(php.params.date0);
        }
        cfg_save();
    } return TRUE;
}

static php_routine_fn_t const PHP_ROUTINES[] PROGMEM = {
    (php_routine_fn_t)_php_routine_9,
    (php_routine_fn_t)_php_routine_6,
    (php_routine_fn_t)_php_routine_11,
    (php_routine_fn_t)_php_routine_12,
    (php_routine_fn_t)_php_routine_16,
    (php_routine_fn_t)_php_routine_4,
    (php_routine_fn_t)_php_routine_19,
    (php_routine_fn_t)_php_routine_2,
    (php_routine_fn_t)_php_routine_5,
    (php_routine_fn_t)_php_routine_14,
    (php_routine_fn_t)_php_routine_15,
    (php_routine_fn_t)_php_routine_7,
    (php_routine_fn_t)_php_routine_8,
    (php_routine_fn_t)_php_routine_20,
    (php_routine_fn_t)_php_routine_10,
    (php_routine_fn_t)_php_routine_1,
    (php_routine_fn_t)_php_routine_3,
    (php_routine_fn_t)_php_routine_17,
    (php_routine_fn_t)_php_routine_13,
    (php_routine_fn_t)_php_routine_18,
    (php_routine_fn_t)_php_routine_0
};

#define PHP_ROUTINE_9                            0
#define PHP_ROUTINE_6                            1
#define PHP_ROUTINE_11                           2
#define PHP_ROUTINE_12                           3
#define PHP_ROUTINE_16                           4
#define PHP_ROUTINE_4                            5
#define PHP_ROUTINE_19                           6
#define PHP_ROUTINE_2                            7
#define PHP_ROUTINE_5                            8
#define PHP_ROUTINE_14                           9
#define PHP_ROUTINE_15                           10
#define PHP_ROUTINE_7                            11
#define PHP_ROUTINE_8                            12
#define PHP_ROUTINE_20                           13
#define PHP_ROUTINE_10                           14
#define PHP_ROUTINE_1                            15
#define PHP_ROUTINE_3                            16
#define PHP_ROUTINE_17                           17
#define PHP_ROUTINE_13                           18
#define PHP_ROUTINE_18                           19
#define PHP_ROUTINE_0                            20

#define PHP_LABEL_CONFIG_PHP_ENTRY               0
#define PHP_LABEL_CONFIG_PHP_IF_0_END            11
#define PHP_LABEL_CONFIG_PHP_WHILE_1_START       17
#define PHP_LABEL_CONFIG_PHP_WHILE_1_END         34
#define PHP_LABEL_HEADER_PHP_ENTRY               56
#define PHP_LABEL_HOME_PHP_ENTRY                 62
#define PHP_LABEL_LOG_PHP_ENTRY                  81
#define PHP_LABEL_LOG_PHP_WHILE_1_START          92
#define PHP_LABEL_LOG_PHP_WHILE_1_END            110
#define PHP_LABEL_LOG_PHP_IF_0_END               111
#define PHP_LABEL_RECORDS_PHP_ENTRY              123
#define PHP_LABEL_RECORDS_PHP_WHILE_1_START      133
#define PHP_LABEL_RECORDS_PHP_WHILE_1_END        150
#define PHP_LABEL_RECORDS_PHP_WHILE_2_START      152
#define PHP_LABEL_RECORDS_PHP_WHILE_3_START      158
#define PHP_LABEL_RECORDS_PHP_IF_4_END           175
#define PHP_LABEL_RECORDS_PHP_IF_4_END2          177
#define PHP_LABEL_RECORDS_PHP_WHILE_3_END        181
#define PHP_LABEL_RECORDS_PHP_WHILE_2_END        186
#define PHP_LABEL_RECORDS_PHP_IF_0_END           187
#define PHP_LABEL_SUB_1                          198
#define PHP_LABEL_HOME_PHP_WHILE_3_START         218
#define PHP_LABEL_SUB_2                          218
#define PHP_LABEL_HOME_PHP_WHILE_3_END           230
#define PHP_LABEL_SUB_3                          233
#define PHP_LABEL_HOME_PHP_WHILE_0_START         234
#define PHP_LABEL_HOME_PHP_IF_1_END              248
#define PHP_LABEL_HOME_PHP_WHILE_0_END           251
#define PHP_LABEL_HOME_PHP_WHILE_2_START         253
#define PHP_LABEL_SUB_4                          253
#define PHP_LABEL_HOME_PHP_WHILE_2_END           270
#define PHP_LABEL_SUB_5                          272

static const uint8_t PHP_CODE[] PROGMEM = {
    // config.php_entry
    PHP_UCODE_ROUTINE(PHP_ROUTINE_0),
    PHP_UCODE_CALL(PHP_LABEL_SUB_1),
    PHP_UCODE_TEXT(PHP_STR_CONFIGPAGE),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_1),
    PHP_UCODE_JUMP(PHP_LABEL_CONFIG_PHP_IF_0_END),
    PHP_UCODE_ATTR(PHP_ATTR_EXECUTED),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_COMMAND,PHP_CONV_UINT8),
    // config.php_if_0_end
    PHP_UCODE_OPEN_TAG(PHP_NODE_CONFIG),
    PHP_UCODE_ATTR(PHP_ATTR_TAB),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_TAB,PHP_CONV_UINT8),
    PHP_UCODE_OPEN_TAG(PHP_NODE_PLANTS),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_2),
    // config.php_while_1_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_3),
    PHP_UCODE_JUMP(PHP_LABEL_CONFIG_PHP_WHILE_1_END),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_4),
    PHP_UCODE_OPEN_TAG(PHP_NODE_PLANT),
    PHP_UCODE_ATTR(PHP_ATTR_MIN),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_CFG_WATERING_START_THRESH,PHP_CONV_UINT8),
    PHP_UCODE_ATTR(PHP_ATTR_MAX),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_CFG_WATERING_STOP_THRESH,PHP_CONV_UINT8),
    PHP_UCODE_ATTR(PHP_ATTR_FLAGS),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_CFG_FLAGS,PHP_CONV_UINT8),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_JUMP(PHP_LABEL_CONFIG_PHP_WHILE_1_START),
    // config.php_while_1_end
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_OPEN_TAG(PHP_NODE_RECORDING),
    PHP_UCODE_ATTR(PHP_ATTR_INTERVAL),
    PHP_UCODE_FORMAT(PHP_VAR_CFG_RECORDING_INTERVAL,PHP_CONV_UINT16),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_OPEN_TAG(PHP_NODE_NET),
    PHP_UCODE_ATTR(PHP_ATTR_MAC),
    PHP_UCODE_FORMAT(PHP_VAR_CFG_MAC_ADDR,PHP_CONV_MAC),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_OPEN_TAG(PHP_NODE_ALERTS),
    PHP_UCODE_ATTR(PHP_ATTR_IP),
    PHP_UCODE_FORMAT(PHP_VAR_CFG_ALERTS_SERVER_IP,PHP_CONV_IP),
    PHP_UCODE_ATTR(PHP_ATTR_PORT),
    PHP_UCODE_FORMAT(PHP_VAR_CFG_ALERTS_PORT,PHP_CONV_UINT16),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_END,
    // header.php_entry
    PHP_UCODE_CALL(PHP_LABEL_SUB_1),
    PHP_UCODE_TEXT(PHP_STR_HEADERPAGE),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_END,
    // home.php_entry
    PHP_UCODE_CALL(PHP_LABEL_SUB_1),
    PHP_UCODE_TEXT(PHP_STR_HOMEPAGE),
    PHP_UCODE_OPEN_TAG(PHP_NODE_CURRENT),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_5),
    PHP_UCODE_CALL(PHP_LABEL_SUB_3),
    PHP_UCODE_OPEN_TAG(PHP_NODE_LOG),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_6),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_7),
    PHP_UCODE_CALL(PHP_LABEL_SUB_4),
    PHP_UCODE_OPEN_TAG(PHP_NODE_RECORDS),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_8),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_7),
    PHP_UCODE_CALL(PHP_LABEL_SUB_2),
    PHP_UCODE_END,
    // log.php_entry
    PHP_UCODE_ROUTINE(PHP_ROUTINE_6),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_9),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_10),
    PHP_UCODE_JUMP(PHP_LABEL_LOG_PHP_IF_0_END),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_11),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_12),
    PHP_UCODE_TEXT(PHP_STR_DATE_TYPE_MESSAG),
    PHP_UCODE_TEXT(PHP_STR__),
    // log.php_while_1_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_13),
    PHP_UCODE_JUMP(PHP_LABEL_LOG_PHP_WHILE_1_END),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_TIME,PHP_CONV_DATETIME_CSV),
    PHP_UCODE_TEXT(PHP_STR__2),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_CODE,PHP_CONV_EVENT_TYPE),
    PHP_UCODE_TEXT(PHP_STR__2),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT,PHP_CONV_EVENT_MESSAGE),
    PHP_UCODE_TEXT(PHP_STR__),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_14),
    PHP_UCODE_JUMP(PHP_LABEL_LOG_PHP_WHILE_1_START),
    // log.php_while_1_end
    PHP_UCODE_END,
    // log.php_if_0_end
    PHP_UCODE_ROUTINE(PHP_ROUTINE_15),
    PHP_UCODE_CALL(PHP_LABEL_SUB_1),
    PHP_UCODE_TEXT(PHP_STR_LOGPAGE),
    PHP_UCODE_OPEN_TAG(PHP_NODE_LOG),
    PHP_UCODE_CALL(PHP_LABEL_SUB_5),
    PHP_UCODE_CALL(PHP_LABEL_SUB_4),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_END,
    // records.php_entry
    PHP_UCODE_ROUTINE(PHP_ROUTINE_8),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_9),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_10),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_IF_0_END),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_11),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_16),
    PHP_UCODE_TEXT(PHP_STR_DATE),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_2),
    // records.php_while_1_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_17),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_1_END),
    PHP_UCODE_TEXT(PHP_STR__HUMID_),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_IDX,PHP_CONV_UINT8),
    PHP_UCODE_TEXT(PHP_STR__ILUM_),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_IDX,PHP_CONV_UINT8),
    PHP_UCODE_TEXT(PHP_STR__INFO_),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_IDX,PHP_CONV_UINT8),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_1_START),
    // records.php_while_1_end
    PHP_UCODE_TEXT(PHP_STR__),
    // records.php_while_2_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_18),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_2_END),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_TIME,PHP_CONV_DATETIME_CSV),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_2),
    // records.php_while_3_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_3),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_3_END),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_19),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_20),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_IF_4_END),
    PHP_UCODE_TEXT(PHP_STR__2),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_STATUS_HUMIDITY,PHP_CONV_UINT8),
    PHP_UCODE_TEXT(PHP_STR__2),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_STATUS_ILUMINATION,PHP_CONV_UINT8),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_IF_4_END2),
    // records.php_if_4_end
    PHP_UCODE_TEXT(PHP_STR___),
    // records.php_if_4_end2
    PHP_UCODE_TEXT(PHP_STR__2),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_3_START),
    // records.php_while_3_end
    PHP_UCODE_TEXT(PHP_STR__),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_14),
    PHP_UCODE_JUMP(PHP_LABEL_RECORDS_PHP_WHILE_2_START),
    // records.php_while_2_end
    PHP_UCODE_END,
    // records.php_if_0_end
    PHP_UCODE_ROUTINE(PHP_ROUTINE_15),
    PHP_UCODE_CALL(PHP_LABEL_SUB_1),
    PHP_UCODE_TEXT(PHP_STR_RECORDSPAGE),
    PHP_UCODE_OPEN_TAG(PHP_NODE_RECORDS),
    PHP_UCODE_CALL(PHP_LABEL_SUB_5),
    PHP_UCODE_CALL(PHP_LABEL_SUB_2),
    PHP_UCODE_END,
    // sub_1
    PHP_UCODE_OPEN_TAG(PHP_NODE_PI_XML),
    PHP_UCODE_ATTR(PHP_ATTR_VERSION),
    PHP_UCODE_TEXT(PHP_STR_1_0),
    PHP_UCODE_ATTR(PHP_ATTR_ENCODING),
    PHP_UCODE_TEXT(PHP_STR_UTF_8),
    PHP_UCODE_OPEN_TAG(PHP_NODE_PI_XML_STYLESHEET),
    PHP_UCODE_ATTR(PHP_ATTR_TYPE),
    PHP_UCODE_TEXT(PHP_STR_TEXT_XSL),
    PHP_UCODE_ATTR(PHP_ATTR_HREF),
    PHP_UCODE_TEXT(PHP_STR_MAIN_XSL),
    PHP_UCODE_OPEN_TAG(PHP_NODE_PAGE),
    PHP_UCODE_ATTR(PHP_ATTR_DATETIME),
    PHP_UCODE_FORMAT(PHP_VAR_RENDER_DATE,PHP_CONV_DATETIME),
    PHP_UCODE_ATTR(PHP_ATTR_ID),
    PHP_UCODE_RET,
    // sub_2
    // home.php_while_3_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_18),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_3_END),
    PHP_UCODE_OPEN_TAG(PHP_NODE_ROW),
    PHP_UCODE_ATTR(PHP_ATTR_DATETIME),
    PHP_UCODE_FORMAT(PHP_VAR_RECORD_TIME,PHP_CONV_DATETIME),
    PHP_UCODE_CALL(PHP_LABEL_SUB_3),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_14),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_3_START),
    // home.php_while_3_end
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_RET,
    // sub_3
    PHP_UCODE_ROUTINE(PHP_ROUTINE_2),
    // home.php_while_0_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_3),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_0_END),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_19),
    PHP_UCODE_OPEN_TAG(PHP_NODE_PLANT),
    PHP_UCODE_ROUTINE(PHP_ROUTINE_20),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_IF_1_END),
    PHP_UCODE_ATTR(PHP_ATTR_HUMID),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_STATUS_HUMIDITY,PHP_CONV_UINT8),
    PHP_UCODE_ATTR(PHP_ATTR_ILUM),
    PHP_UCODE_FORMAT(PHP_VAR_PLANT_STATUS_ILUMINATION,PHP_CONV_UINT8),
    // home.php_if_1_end
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_0_START),
    // home.php_while_0_end
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_RET,
    // sub_4
    // home.php_while_2_start
    PHP_UCODE_ROUTINE(PHP_ROUTINE_13),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_2_END),
    PHP_UCODE_OPEN_TAG(PHP_NODE_EVENT),
    PHP_UCODE_ATTR(PHP_ATTR_DATETIME),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_TIME,PHP_CONV_DATETIME),
    PHP_UCODE_ATTR(PHP_ATTR_CODE),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_CODE,PHP_CONV_UINT8),
    PHP_UCODE_ATTR(PHP_ATTR_DATA),
    PHP_UCODE_FORMAT(PHP_VAR_EVENT_DATA,PHP_CONV_UINT32),
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_ROUTINE(PHP_ROUTINE_14),
    PHP_UCODE_JUMP(PHP_LABEL_HOME_PHP_WHILE_2_START),
    // home.php_while_2_end
    PHP_UCODE_CLOSE_TAG,
    PHP_UCODE_RET,
    // sub_5
    PHP_UCODE_ATTR(PHP_ATTR_PAGE),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_PAGE,PHP_CONV_UINT16),
    PHP_UCODE_ATTR(PHP_ATTR_PAGES),
    PHP_UCODE_FORMAT(PHP_VAR_PAGES,PHP_CONV_UINT16),
    PHP_UCODE_ATTR(PHP_ATTR_PERPAGE),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_PER_PAGE,PHP_CONV_UINT16),
    PHP_UCODE_ATTR(PHP_ATTR_FROM),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_DATE0,PHP_CONV_DATETIME),
    PHP_UCODE_ATTR(PHP_ATTR_TO),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_DATE1,PHP_CONV_DATETIME),
    PHP_UCODE_ATTR(PHP_ATTR_REVERSE),
    PHP_UCODE_FORMAT(PHP_VAR_PARAMS_REVERSE,PHP_CONV_UINT8),
    PHP_UCODE_RET
};
