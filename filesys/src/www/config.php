<!--{
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
    }
}-->
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<page datetime="<!--=php.render_date:datetime-->" id="configPage" 
    <!--if php.params.command -->executed="<!--=php.params.command:uint8-->"<!--endif-->
    >
    <config tab="<!--=php.params.tab:uint8-->">
        <plants>
            <!--{ php.vars.plant_idx = 0; }-->
            <!--while php.vars.plant_idx < CFG_MAX_PLANTS-->
            <!--{ php.vars.plant.cfg = cfg.plants[php.vars.plant_idx++]; }-->
            <plant min="<!--=php.vars.plant.cfg.watering_start_threshold:uint8-->"
                   max="<!--=php.vars.plant.cfg.watering_stop_threshold:uint8-->"
                   flags="<!--=php.vars.plant.cfg.flags:uint8-->" />
            <!--endwhile-->
        </plants>
        <recording interval="<!--=cfg.recording_interval:uint16-->" />
        <net mac="<!--=cfg.mac_addr:mac-->" />
        <alerts ip="<!--=cfg.alerts_server_ip:ip-->" port="<!--=cfg.alerts_port:uint16-->" />
    </config>
</page>
