/*
 * http.h - HTTP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_HTTP_H
#define _PROTO_HTTP_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#include "net/packet_buf.h"
#include "util/bool.h"

#define HTTP_COMMAND_GET              0
#define HTTP_COMMAND_POST             1
#define HTTP_COMMAND_UNKNOWN          2

#define HTTP_CODE_OK                  200
#define HTTP_CODE_REDIRECT            302
#define HTTP_CODE_NOT_FOUND           404
#define HTTP_CODE_SERVER_ERROR        500
#define HTTP_CODE_NOT_IMPLEMENTED     501

#define HTTP_CONTENT_TYPE_TEXT_PLAIN  0
#define HTTP_CONTENT_TYPE_TEXT_HTML   1
#define HTTP_CONTENT_TYPE_TEXT_XML    2
#define HTTP_CONTENT_TYPE_TEXT_XSLT   3
#define HTTP_CONTENT_TYPE_TEXT_CSS    4
#define HTTP_CONTENT_TYPE_IMAGE_PNG   5
#define HTTP_CONTENT_TYPE_IMAGE_JPEG  6
#define HTTP_CONTENT_TYPE_TEXT_CSV    7
#define HTTP_CONTENT_TYPE_NONE        8

uint8_t http_parse_command(const char *text);
bool http_parse_param(PacketBuf *packet, char *param_buf, uint8_t param_size,
    char *value_buf, uint8_t value_size);
void http_uri_decode(char *decoded, char *uri);
PGM_P http_get_http_code_text(uint16_t http_code);
PGM_P http_get_content_type_text(uint8_t content_type);
const char *http_get_url_extension(const char *url);
uint8_t http_guess_content_type(const char* url);
void http_format_chunk_size_hdr(char *buf, uint16_t size);

#endif
