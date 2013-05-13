<!--{ rec_get_query(&php.vars.query); }-->
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
    <!--{ php.download_filename = PSTR("records.csv"); }-->
    Date<!--{ php.vars.plant_idx = 0; }-->
    <!--while php.vars.plant_idx++ < CFG_MAX_PLANTS-->
        ,Humid <!--=php.vars.plant_idx:uint8-->
        ,Ilum <!--=php.vars.plant_idx:uint8-->
        ,Info <!--=php.vars.plant_idx:uint8-->
    <!--endwhile-->
    <!--nl-->
    <!--while query_get(&php.vars.query, &php.vars.entry.record)-->
        <!--=php.vars.entry.event.time:datetime_csv-->
        <!--{ php.vars.plant_idx = 0; }-->
        <!--while php.vars.plant_idx < CFG_MAX_PLANTS-->
            <!--{ php.vars.plant.status = php.vars.entry.record.plants[php.vars.plant_idx++]; }-->
            <!--if !(php.vars.plant.status.flags & PLANT_FLAGS_NOT_INSTALLED)-->
                ,<!--=php.vars.plant.status.humidity:uint8-->
                ,<!--=php.vars.plant.status.ilumination:uint8-->
            <!--else-->,,<!--endif-->
        ,<!--endwhile-->
        <!--nl-->
        <!--{ query_next(&php.vars.query); }-->
    <!--endwhile-->
    <!--abort-->
<!--endif-->
<!--{ query_paginate(&php.vars.query, php.params.per_page, &php.params.page, &php.vars.pages); }-->
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<page datetime="<!--=php.render_date:datetime-->" id="recordsPage">
    <records
        page="<!--=php.params.page:uint16-->"
        pages="<!--=php.vars.pages:uint16-->"
        perPage="<!--=php.params.per_page:uint16-->"
        from="<!--=php.params.from_date:datetime-->"
        to="<!--=php.params.to_date:datetime-->"
        reverse="<!--=php.params.reverse:uint8-->"
        >
        <!--while query_get(&php.vars.query, &php.vars.entry.record)-->
        <row datetime="<!--=php.vars.entry.record.time:datetime-->">
            <!--{ php.vars.plant_idx = 0; }-->
            <!--while php.vars.plant_idx < CFG_MAX_PLANTS-->
            <!--{ php.vars.plant.status = php.vars.entry.record.plants[php.vars.plant_idx++]; }-->
            <plant
                <!--if !(php.vars.plant.status.flags & PLANT_FLAGS_NOT_INSTALLED)-->
                humid="<!--=php.vars.plant.status.humidity:uint8-->"
                ilum="<!--=php.vars.plant.status.ilumination:uint8-->"
                <!--endif-->
                />
            <!--endwhile-->
        </row>
        <!--{ query_next(&php.vars.query); }-->
        <!--endwhile-->
    </records>
</page>
