<?
set_time_limit(0);

$blacklist = array('Antimat','Stukach','Guard','Opers','SiskaOP');

require_once(dirname(__FILE__).'/inc/config.inc');
if(isset($_SERVER['SERVER_NAME'])){
    echo "<pre>";
}
try {
    $DC = new DC_P2S( BOT_NICKNAME, BOT_SHARESIZE, BOT_EMAIL, BOT_SPEED, BOT_VERSION );
    $DC->onEnter = 'enterFunc';
    $DC->onQuit  = 'quitFunc';    
    //$DC->onChatReceive = 'chatFunc';

    $starttime = microtime(true);
    $goodLists = 0;
    $badLists = 0;
    if( !$DC->connect( DC_SERVER, DC_PORT, BOT_TCP_PORT ) ) return;
    if( !$DC->authenticate() ) return;
    
    $nickList = Indexer::getNeedNicks(array_diff($DC->nickList, $blacklist) );

    printlog(1, 'We are interested in ');
    foreach($nickList as $k=>$v) {
        if(CONSOLE_OUT_CHARSET){
            printlog(1, iconv('utf-8', CONSOLE_OUT_CHARSET, $v)." ");
        }else{
            printlog(1, $v." ");
        }
    }
    printlog(1, "\n");
    
    foreach($nickList as $k=>&$nick) {
        if( !isset($nickList[$k])  || $nickList[$k] == null  || !$nickList[$k] || !$nick){
            continue;
        }
        $fn = getFileList($DC, $nick);
        if($fn){
            $goodLists++;
        }else{
            $badLists++;
        }
        Indexer::setNickList($nick, $fn);
    }
    unset($nick);
    printlog(0, "Lists: $goodLists good,  $badLists bad, ");
    printlog(0, round(microtime(true)-$starttime,2)."s\n");
}catch (ForceMoveException $e) {
    printlog(0, "ForceMove received\n");
}catch (ErrorReceivedException $e) {
    printlog(0, "Error received\n");
}

//$DC->LoopProcessing();

/**
 * FUNCTIONS
 */
function quitFunc($nickname){
    printlog(1, "-=$nickname=-quitted\n");

    global $nickList;
    foreach ($nickList as $i=>$v){
        if($v==$nickname){
            unset($nickList[$i]);
        }
    }
}

function enterFunc($nickname){
    printlog(1, "-=$nickname=-entered\n");

    global $nickList;
    global $blacklist;

    $nick_array = array_diff(array($nickname), $blacklist);
    if( !is_array($nick_array) || !count($nick_array)){
        return;
    }
    $nick_array = Indexer::getNeedNicks( $nick_array );
    if( !is_array($nick_array) || !count($nick_array)){
        return;
    }
   
    $nickList[] = $nickname;
}

function printlog($debugLevel, $message){
    $debug = defined('DEBUG')?constant('DEBUG'):0;
    if($debug >= $debugLevel){
        echo $message;
    }
}

function getFileList($DC, $nick){
    $st = microtime(true);

    if(CONSOLE_OUT_CHARSET){
        printlog(1, iconv('utf-8',CONSOLE_OUT_CHARSET,$nick)." - ");
    }else{
        printlog(1, $nick." - ");
    }
    
    $bz2Content = $DC->getFile($nick, 'files.xml.bz2');
    if($DC->connectedBack){
        printlog(1, " backconnect ".$DC->connectedBack." - ");
        $DC->connectedBack = false;
    }
    if($bz2Content === false){
        printlog(1, "fail(".$DC->lasterror.")\n");
        return false;
    }elseif($bz2Content === '') {
        printlog(1, "not bz2\n");
        return false;
    }
    $fn = uniqid().'.bz2';
    $fh = fopen(WORKER_TMPDIR.'/'.$fn, 'wb');
    $written = fwrite($fh, $bz2Content);
    fclose($fh);
    if(!$written){
        printlog(1, "nothing written\n");
        if(file_exists(WORKER_TMPDIR.'/'.$fn)) unlink(WORKER_TMPDIR.'/'.$fn);
        return false;
    }
    $et = microtime(true);
    printlog(1, "ok (".sprintf("%01.2f",$et-$st)." s) \n");
    return $fn;
}
?>