/*
 * http.c - HTTP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>
#include <ctype.h>

#include "util/stringset.h"
#include "util/hex.h"
#include "http.h"

#define HTTP_COMMANDS_COUNT       2
#define HTTP_CODES_COUNT          5
#define HTTP_CONTENT_TYPES_COUNT  8

static const char HTTP_COMMAND_NAMES[] PROGMEM =
    "GET\0"
    "POST";

static const uint16_t HTTP_CODES[HTTP_CODES_COUNT] PROGMEM = {
    HTTP_CODE_OK,
    HTTP_CODE_REDIRECT,
    HTTP_CODE_NOT_FOUND,
    HTTP_CODE_SERVER_ERROR,
    HTTP_CODE_NOT_IMPLEMENTED
};

static const char HTTP_CODE_NAMES[] PROGMEM =
    "OK\0"
    "Found\0"
    "Not Found\0"
    "Internal Server Error\0"
    "Not Implemented\0"
    "Error";

static const char CONTENT_TYPE_NAMES[] PROGMEM =
    "text/plain\0"
    "text/html\0"
    "text/xml\0"
    "text/xslt\0"
    "text/css\0"
    "image/png\0"
    "image/jpeg\0"
    "text/x-csv\0"
    "text/plain";

#define EXTENSIONS_COUNT 7

static const uint8_t CONTENT_TYPES_FOR_EXT[EXTENSIONS_COUNT + 1] PROGMEM = {
    HTTP_CONTENT_TYPE_TEXT_HTML,
    HTTP_CONTENT_TYPE_TEXT_CSS,
    HTTP_CONTENT_TYPE_TEXT_PLAIN,
    HTTP_CONTENT_TYPE_IMAGE_PNG,
    HTTP_CONTENT_TYPE_IMAGE_JPEG,
    HTTP_CONTENT_TYPE_TEXT_XSLT,
    HTTP_CONTENT_TYPE_TEXT_XML,
    HTTP_CONTENT_TYPE_NONE
};

static const char EXTENSIONS[] PROGMEM =
    "htm\0"
    "css\0"
    "txt\0"
    "png\0"
    "jpg\0"
    "xsl\0"
    "php";

/**
 * http_parse_command - Parses an HTTP command name
 * @text: An HTTP command name
 */
uint8_t http_parse_command(const char *text)
{
    return strset_find(HTTP_COMMAND_NAMES, HTTP_COMMANDS_COUNT, text);
}

/**
 * http_parse_param - Reads an HTTP query parameter (from an URL or POST
 *                    request body)
 * @packet: A packet buffer containing parameter data
 * @param_buf: A buffer to store the parameter name
 * @param_size: The capacity of the name buffer, NUL included
 * @value_buf: A buffer to store the value name
 * @values_size: The capacity of the value buffer, NUL included
 */
bool http_parse_param(PacketBuf *packet, char *param_buf, uint8_t param_size,
    char *value_buf, uint8_t value_size)
{
    char term;
    
    param_buf[0] = 0;
    value_buf[0] = 0;
    
    term = pktbuf_get_token(packet, param_buf, param_size, PSTR(" =&\r\n"));
    if (!strlen(param_buf) && (term != '&') && (term != '=')) {
        return FALSE;
    }
    http_uri_decode(param_buf, param_buf);
    
    pktbuf_pull(packet, 1); // consume '&' or '=' character
    
    if (term == '=') {
        term = pktbuf_get_token(packet, value_buf, value_size, PSTR(" &\r\n"));
        http_uri_decode(value_buf, value_buf);
        
        if (term == '&') {
            pktbuf_pull(packet, 1);
        }
    }
    
    return TRUE;
}

/**
 * http_uri_decode - URL-decodes a portion of an URL by unescaping
 *                   %xx sequences and + signs.
 * @decoded: A buffer to receive the decoded URI
 * @uri: The URI fragment to decode. May be the same as decoded.
 */
void http_uri_decode(char *decoded, char *uri)
{   
    uint32_t val;
    char c;
    
    while ((c = *(uri++))) {
        if (c == '+') {
            c = ' ';
        }
        if (c == '%') {
            if (hex_parse(uri, 2, &val) == 2) {
                c = (uint8_t)val;
                uri += 2;
            }
        }
        
        *(decoded++) = c;
    }
    *decoded = 0;
}

/**
 * http_get_http_code_text - Gets the message for a HTTP code
 * @http_code: A HTTP_CODE_* constant
 */
PGM_P http_get_http_code_text(uint16_t http_code)
{
    uint8_t i;
    
    for (i = 0; i < HTTP_CODES_COUNT; i++) {
        if (http_code == pgm_read_word(HTTP_CODES + i)) {
            break;
        }
    }
    
    return strset_get(HTTP_CODE_NAMES, i);
}

/**
 * http_get_content_type_text - Gets the text for a content-type constant
 * @content_type: A HTTP_CONTENT_TYPE_* constant
 */
PGM_P http_get_content_type_text(uint8_t content_type)
{
    return strset_get(CONTENT_TYPE_NAMES, content_type);
}

/**
 * http_get_url_extension - Gets the extension from an URL
 * @url: An URL string
 */
const char *http_get_url_extension(const char *url)
{
    const char *ptr = strrchr(url, '/');
    if (ptr != NULL) {
        ptr = strrchr(ptr, '.') + 1;
    }

    return (ptr != NULL) ? ptr : (url + strlen(url));
}

/**
 * http_guess_content_type - Guess the Content Type from the URL extension
 * @url: An URL string
 */
uint8_t http_guess_content_type(const char *url)
{
    return pgm_read_byte(CONTENT_TYPES_FOR_EXT +
        strset_find(EXTENSIONS, EXTENSIONS_COUNT, 
            http_get_url_extension(url)));
}

/**
 * http_format_chunk_size_hdr - Formats a HTTP chunk size header
 * @buf: A buffer to store the header. Must be at least 7 bytes wide
 * @chunk_size: The chunk size
 */
void http_format_chunk_size_hdr(char *buf, uint16_t chunk_size)
{
    hex_format(buf, 4, chunk_size);
    buf[4] = '\r';
    buf[5] = '\n';
    buf[6] = 0;
}
