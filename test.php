<?php

echo "<pre>";
$obj = json_decode( file_get_contents('./tmp/queries_dirs_10.dump') );
print_r($obj[0]);


return;

require_once(dirname(__FILE__).'/inc/config.inc');
$DB = Indexer::getDB();

$testvar = $DB->select('select * from files where id=0');
if(!empty($testvar))die('gon on start2');
return;


set_time_limit(0);
if(!file_exists('data')){
    $chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ~`!@#$%^&*()_+-=\\|/?.,<>"\';:][}{№';
    $strings = array();
    for($i=0;$i<100000;$i++){
        $randstr = "";
        $len = mt_rand(10, 50);
        for($j=0;$j<$len;$j++){
            $randstr .= $chars{mt_rand(0, strlen($chars)-1)};
        }
        $strings[] = $randstr;
    }

    file_put_contents('data', serialize($strings));
}else{
    $strings = unserialize(file_get_contents('data'));
}

$st = microtime(true);
for ($i=0;$i<count($strings);$i++){
    $crc32[$i] = crc32($strings[$i]);
}
$et = microtime(true);
echo "crc32: ".($et-$st)."s<br/>\n";
echo $crc32[0]."<br/>\n";

$st = microtime(true);
for ($i=0;$i<count($strings);$i++){
    $sha1[$i] = sha1($strings[$i]);
}
$et = microtime(true);
echo "sha1: ".($et-$st)."s<br/>\n";

$st = microtime(true);
for ($i=0;$i<count($strings);$i++){
    $md5[$i] = md5($strings[$i]);
}
$et = microtime(true);
echo "md5: ".($et-$st)."s<br/>\n";
?>