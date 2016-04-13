<?php

$ch = curl_init("160.36.205.61:9202/_cat/indices");
$res = curl_exec($ch);
print_r($res);
curl_close($ch);
