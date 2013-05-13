<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<page datetime="<!--=php.render_date:datetime-->" id="homePage">
    <current>
        <!--{ plants_get_latest_record(&php.vars.entry.record); }-->
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
    </current>
    <log>
        <!--{ log_get_query(&php.vars.query); }-->
        <!--{ query_reverse(&php.vars.query); query_limit(&php.vars.query, 5); }-->
        <!--while query_get(&php.vars.query, &php.vars.entry.event)-->
        <event datetime="<!--=php.vars.entry.event.time:datetime-->"
               code="<!--=php.vars.entry.event.code:uint8-->"
               data="<!--=php.vars.entry.event.data:uint32-->" />
        <!--{ query_next(&php.vars.query); }-->
        <!--endwhile-->
    </log>
    <records>
        <!--{ rec_get_query(&php.vars.query); }-->
        <!--{ query_reverse(&php.vars.query); query_limit(&php.vars.query, 5); }-->
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
