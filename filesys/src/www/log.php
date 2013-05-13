<!--{ log_get_query(&php.vars.query); }-->
<!--{
    if (php.params.from_date)
        query_filter_after(&php.vars.query, php.params.from_date);
    
    if (php.params.to_date)
        query_filter_before(&php.vars.query, php.params.to_date + TIME_SECONDS_PER_DAY - 1);
    
    if (php.params.reverse)
        query_reverse(&php.vars.query);
}-->
<!--if php.params.command == PHP_CMD_EXPORT-->
    <!--{ php.content_type = HTTP_CONTENT_TYPE_TEXT_CSV; }-->
    <!--{ php.download_filename = PSTR("log.csv"); }-->
    Date,Type,Message<!--nl-->
    <!--while query_get(&php.vars.query, &php.vars.entry.event)-->
        <!--=php.vars.entry.event.time:datetime_csv-->
        ,<!--=php.vars.entry.event.code:event_type-->
        ,<!--=php.vars.entry.event:event_message-->
        <!--nl-->
        <!--{ query_next(&php.vars.query); }-->
    <!--endwhile-->
    <!--abort-->
<!--endif-->
<!--{ query_paginate(&php.vars.query, php.params.per_page, &php.params.page, &php.vars.pages); }-->
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<page datetime="<!--=php.render_date:datetime-->" id="logPage">
    <log
        page="<!--=php.params.page:uint16-->"
        pages="<!--=php.vars.pages:uint16-->"
        perPage="<!--=php.params.per_page:uint16-->"
        from="<!--=php.params.from_date:datetime-->"
        to="<!--=php.params.to_date:datetime-->"
        reverse="<!--=php.params.reverse:uint8-->"
        >
        <!--while query_get(&php.vars.query, &php.vars.entry.event)-->
        <event datetime="<!--=php.vars.entry.event.time:datetime-->"
               code="<!--=php.vars.entry.event.code:uint8-->"
               data="<!--=php.vars.entry.event.data:uint32-->" />
        <!--{ query_next(&php.vars.query); }-->
        <!--endwhile-->
    </log>
</page>
