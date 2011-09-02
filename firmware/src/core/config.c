/*
 * config.c - Config file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "config.h"

/**
 * The current device configuration.
 */
Config cfg;

/**
 * cfg_load - Loads the settings from the config file
 */
void cfg_load(void)
{
    file_t file;
    file_open_P(&file, PSTR(CFG_CONFIG_FILE));
    file_read(&file, &cfg, sizeof(Config));
}

/**
 * cfg_save - Saves settings to the config file
 */
void cfg_save(void)
{
    file_t file;
    file_open_P(&file, PSTR(CFG_CONFIG_FILE));
    file_write(&file, &cfg, sizeof(Config));
    file_flush(&file);
}
