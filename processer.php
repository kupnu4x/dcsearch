<?php
set_time_limit(0);
require_once(dirname(__FILE__).'/inc/config.inc');
if(isset($_SERVER['SERVER_NAME'])){
    echo "<pre>";
}
///////////////////
$dir = opendir(WORKER_TMPDIR);
$deleted = 0;
while($fn = readdir($dir)){
    if(pathinfo($fn,PATHINFO_EXTENSION)=='dump'){
        @unlink(WORKER_TMPDIR.'/'.$fn);
        $deleted++;
    }
}
printlog(1, "$deleted dump files deleted\n");
///////////////////

$gst = microtime(true);
$DB = Indexer::getDB();
$usersToProcess = Indexer::getUsersToProcess();
$total_count = $total_added_files = $total_added_dirs = $total_deleted_files = $total_deleted_dirs = 0;
if(is_array($usersToProcess) && count($usersToProcess)){
    foreach($usersToProcess as $user) {
    //$user = $usersToProcess[1];
        if(CONSOLE_OUT_CHARSET){
            printlog(1, iconv('utf-8', CONSOLE_OUT_CHARSET, $user['nick']));
        }else{
            printlog(1, $user['nick']);
        }
        printlog(1,"(".$user['files'].",".$user['dirs'].")-");
        $st = microtime(true);
        if(!file_exists( WORKER_TMPDIR.'/'.$user['filename']) ){
            printlog(1,"list not exists, skipping\n");
            Indexer::skipUser($user['id']);
            continue;
        }
        list($added_files,$added_dirs,$deleted_files,$deleted_dirs) = Indexer::processUser($user['id'], 'compress.bzip2://'.WORKER_TMPDIR.'/'.$user['filename'],$user['files'],$user['dirs']);
        printlog(1, "ADDED:$added_files files,$added_dirs dirs;DELETED:$deleted_files files,$deleted_dirs dirs-");
        $total_added_files += $added_files;
        $total_added_dirs += $added_dirs;
        $total_deleted_files += $deleted_files;
        $total_deleted_dirs += $deleted_dirs;
        $total_count++;
        $et = microtime(true);
        printlog(1, round($et-$st,2)."s\n");
        sleep(1); //win+mysql 5.1: id=0 fixing
    }
}
$get = microtime(true);
printlog(1, "========\n");
printlog(0, "$total_count users-ADDED $total_added_files files,$total_added_dirs dirs;DELETED:$total_deleted_files files,$total_deleted_dirs dirs-".sprintf("%01.2f",$get-$gst)."s\n");
if( $deleted_users = Indexer::delOldUsers() ){
    printlog(0, "$deleted_users old(or empty) users deleted from db\n");
}

Indexer::freeXmlReader();
$dir = opendir(WORKER_TMPDIR);
$deleted = 0;
while($fn = readdir($dir)){
    if(pathinfo($fn,PATHINFO_EXTENSION)=='bz2'){
        @unlink(WORKER_TMPDIR.'/'.$fn);
        $deleted++;
    }
}
printlog(1, "$deleted bz2 files deleted\n");

function printlog($debugLevel, $message){
    $debug = defined('DEBUG')?constant('DEBUG'):0;
    if($debug >= $debugLevel){
        echo $message;
    }
}

?>