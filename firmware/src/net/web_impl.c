/*
 * web_impl.c - HTTP/Web layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>
#include <stdlib.h>

#include "storage/filesys.h"
#include "net/proto/http.h"
#include "php_impl.h"
#include "web_impl.h"
#include "dot_config.h"
#include "util/misc.h"

#define BODY_HTTP_ERROR 1
#define BODY_FILE       2
#define BODY_PHP        3

static struct { 
    struct {
        uint16_t http_code;
        uint8_t content_type;
        uint32_t cache;

        uint8_t body_type;
        union {
            file_t file;
        } body;
        
        bool headers_sent:1;
    } response;
} web;

void _web_write_response_headers(PacketBuf *buffer)
{
    char buf[128];
    uint8_t i;

    for (i=0; i<7; i++) {
        switch(i) {
            case 0:
                strcpy_P(buf, PSTR("HTTP/1.1 "));
                itoa10(web.response.http_code, buf+9);
                strcat_P(buf, PSTR(" "));
                strcat_P(buf, http_get_http_code_text(web.response.http_code));
                break;
            case 1:
                if (!web.response.cache) {
                    continue;
                }
                strcpy_P(buf, PSTR("Cache-Control: public, max-age="));
                ltoa10(web.response.cache, buf+31);
                break;
            case 2:
                strcpy_P(buf, PSTR("Connection: close"));
                break;
            case 3:
                strcpy_P(buf, PSTR("Content-Type: "));
                strcat_P(buf, http_get_content_type_text(web.response.content_type));
                break;
            case 4:
                if (!((web.response.body_type == BODY_PHP) &&
                    (php_is_download()))) {
                    continue;
                }

                strcpy_P(buf, PSTR("Content-Disposition: attachment; filename="));
                php_get_download_filename(buf + strlen(buf));
                break;
            case 5:
                strcpy_P(buf, PSTR("Transfer-Encoding: chunked"));
                break;
            case 6:
                buf[0] = 0;
                break;
        }

        strcat_P(buf, PSTR("\r\n"));

        pktbuf_add_data(buffer, buf, strlen(buf));
    }
}

void _web_serve_error(uint16_t http_code)
{
    web.response.http_code = http_code;
    web.response.content_type = HTTP_CONTENT_TYPE_TEXT_PLAIN;
    web.response.cache = FALSE;
    web.response.body_type = BODY_HTTP_ERROR;
}

bool _web_serve_static_resource(const char *path)
{
    char file_path[56];
    
    strcpy_P(file_path, PSTR(CFG_NET_WEB_WWWROOT_DIR));
    strcat(file_path, path);
    
    web.response.content_type = http_guess_content_type(path);

    if (file_open(&web.response.body.file, file_path)) {
        _web_serve_error(HTTP_CODE_NOT_FOUND);
        return FALSE;
    }

    web.response.http_code = HTTP_CODE_OK;
    web.response.body_type = BODY_FILE;
    web.response.cache = 31556926UL; // one year
    
    return TRUE;
}

bool _web_serve_dynamic_resource(const char *path, PacketBuf *data)
{
    if (!php_start(path, data)) {
        return FALSE;
    }
    
    web.response.http_code = HTTP_CODE_OK;
    web.response.body_type = BODY_PHP;
    web.response.content_type = php_get_content_type();
    web.response.cache = FALSE;

    return TRUE;
}

/**
 * web_new_request - Resets the Web state machine for a new request
 */
void web_new_request(void)
{
    web.response.headers_sent = FALSE;
}

/**
 * web_receive_request - Feeds more of the user request into the Web server
 * @data: A packet buffer storing a chunk of user input
 * 
 * Returns FALSE when reading the request is finished.
 */ 
bool web_receive_request(PacketBuf *data)
{
    char buf[32];
    char term;
    uint8_t http_cmd;
    
    // parse command
    term = pktbuf_get_token(data, buf, 32, PSTR(" "));
    http_cmd = http_parse_command(buf);
    if (!term || (http_cmd == HTTP_COMMAND_UNKNOWN)) {
        _web_serve_error(HTTP_CODE_NOT_IMPLEMENTED);
        return FALSE;
    }
    pktbuf_pull(data, 1);
    
    // parse url path
    term = pktbuf_get_token(data, buf, 32, PSTR("? "));
    if (term == '?') {
        pktbuf_pull(data, 1);
    }
    
    if ((buf[0] != '/') || (strstr_P(buf, PSTR("/..")))) {
        // reject suspicious URLs
        _web_serve_error(HTTP_CODE_NOT_FOUND);
        return FALSE;
    }
    if (!buf[1]) {
        // redirect empty URL to index
        strcpy_P(buf, PSTR("/index.htm"));
    }
    
    if (!_web_serve_dynamic_resource(buf, data)) {
        _web_serve_static_resource(buf);
    }

    return FALSE;
}

/**
 * web_get_answer - Gets more answer data from the Web server
 * @data: A buffer to receive the data chunk
 * 
 * Returns FALSE when submitting the response is finished
 */
bool web_get_answer(PacketBuf *data)
{
    bool done = FALSE, is_php;
    char buf[128];
    uint16_t chunk_len, max_len = pktbuf_remaining(data) - 32;
    uint8_t sub_len, count;
    
    if (!web.response.headers_sent) {
        _web_write_response_headers(data);
        web.response.headers_sent = TRUE;
        return TRUE;
    }
    
    pktbuf_reserve(data, 6);
    
    switch (web.response.body_type)
    {
        case BODY_HTTP_ERROR:
            pktbuf_add_str_P(data, http_get_http_code_text(web.response.http_code));
            done = TRUE;
            break;
        case BODY_FILE:
        case BODY_PHP:
            is_php = (web.response.body_type == BODY_PHP);
            while (max_len) {
                sub_len = (max_len > 128) ? 128 : max_len;
                count = is_php
                    ? php_read(buf, sub_len)
                    : file_read(&(web.response.body.file), buf, sub_len);
                
                pktbuf_add_data(data, buf, count);
                
                max_len -= count;
                
                if (count < sub_len) {
                    break;
                }
            }
            
            done = (is_php && php_eof()) ||
                (!is_php && file_eof(&(web.response.body.file)));
            break;
    }
    
    chunk_len = data->length;
    
    http_format_chunk_size_hdr(buf, chunk_len);
    pktbuf_add_header(data, buf, strlen(buf), 0);
    
    pktbuf_add_str_P(data, (done && chunk_len)
        ? PSTR("\r\n0\r\n\r\n") : PSTR("\r\n"));
    
    return !done;
}
