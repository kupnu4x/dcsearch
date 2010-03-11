<?php
class ForceMoveException extends Exception{ }
class ErrorReceivedException extends Exception{ }

class Command {
    public $command = '';
    public $params = array();
    public $time = 0;
    public function  __construct() {
        $this->time = time();
    }
}

class SL{
    private static $SL=false;

    public static function getSL(){
        return self::$SL;
    }
    public static function setSL($SL){
        self::$SL = $SL;
    }

}

class DCConnection{
    const CONNECT_TIMEOUT = 10;
    const LISTEN_TIMEOUT = 10;
    const READ_BINARY_TIMEOUT = 10;
    const READ_COMMAND_TIMEOUT = 10;
    private $socketListener = false;
    private $socket = false;
    private $connected = false;
    public $lasterror = '';

    public function connect($server, $port){
        $this->socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if( !$this->socket ) {
            //echo "not socket_create!\n";
            return false;
        }
        if( !@socket_connect($this->socket, $server, $port) ) {
            //echo iconv('cp1251', 'cp866',  socket_strerror(socket_last_error($this->socket)) );
            //echo "not socket_connect!\n";
            return false;
        }
        if( !@socket_set_nonblock($this->socket) ) {
            //echo "not socket_set_nonblock!\n";
            return false;
        }
        //sleep(1);
        $this->connected = true;
        return true;
    }


    public function bind($myaddr, $myport){
        $this->socketListener = SL::getSL();
        if($this->socketListener){
            return true;
        }
        $this->socketListener = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if(!$this->socketListener) {
            $this->lasterror = 'socket_create fail';
            return false;
        }
        if( !@socket_bind($this->socketListener, $myaddr, $myport) ) {
            $this->lasterror = 'socket_bind fail';
            return false;
        }
        if( !@socket_listen($this->socketListener) ) {
            $this->lasterror = 'socket_listen fail';
            return false;
        }
        if( !@socket_set_nonblock($this->socketListener) ) {
            $this->lasterror = 'socket_set_nonblock fail';
            return false;
        }
        //sleep(1);
        SL::setSL($this->socketListener);
        return true;
    }

    public function accept(){
        $start = time();
        while( time()-$start < self::LISTEN_TIMEOUT ){
            $this->socket = @socket_accept($this->socketListener);
            if($this->socket) break;
        }
        if(!$this->socket) return false;
        if( !@socket_set_nonblock($this->socket) ) return false;
        //sleep(1);
        $this->connected = true;
        return true;
    }

    public function isConnected(){
        if( !$this->connected ) {
            return false;
        }
        if( !$this->socket ){
            return false;
        }
        /*if($this->socketListener){
            $s = @socket_accept($this->socketListener);
            if($s){
                $this->socket = $s;
                echo "ALERT!\n";
            }
        }*/
        /*if( $err = @socket_last_error($this->socket) ) {
            if( !in_array( $err, array(10035) ) ) {
                echo "last error $err :".socket_strerror($err)."\n";
                //return false;
            }
        }*/
        return true;
    }

    public function readBinary($size){
        if(!$this->connected)return null;
        $data = '';
        $buffer = '';
        $readbyte = 0;
        $prevtime = time();
        while( $readbyte < $size && time()-$prevtime < self::READ_BINARY_TIMEOUT ){
            $len = strlen($data);
            if($len){
                $buffer .= $data;
                $readbyte += $len;
                $prevtime = time();
            }
            $data = @socket_read($this->socket, $size - $readbyte);
        }
        return $buffer;
    }

    public function readCommand(){
        if(!$this->connected)return null;
        $spacesCount = 0;
        $c = @socket_read($this->socket, 1);
        if($c === false) return null;
        $Command = new Command();
        $scommand = ( $c=='$' );
        $prevtime = time();
        while ($c !== '|' && time()-$prevtime < self::READ_COMMAND_TIMEOUT ) {
            if( $c === ' ' && ($spacesCount==0 || $scommand ) ) {
                $prevtime = time();
                $spacesCount++;                
            }elseif($c!==false && $c!=='') {
                $prevtime = time();
                if($spacesCount == 0){
                    $Command->command .= $c;
                }else{
                    if(isset($Command->params[$spacesCount-1])){
                        $Command->params[$spacesCount-1] .= $c;
                    }else{
                        $Command->params[$spacesCount-1] = $c;
                    }
                }
            }
            $c = @socket_read($this->socket, 1);
            /*if( @socket_last_error($this->socket) ){
                $Command = null;
                return null;
            }*/
		}
        if(!$Command->command){
            $Command = null;
            return null;
        }
        if($scommand){//remove $ char
            $Command->command = substr($Command->command, 1);
        }
        return $Command;
    }

    public function sendCommand($data){
        if(!$data)return false;
        if(!$this->connected)return false;
        if( $data{0} != '<' && $data{0} != '$' )$data = '$'.$data;
        $data .= "|";
        $written = @socket_write($this->socket, $data);
        if($written !== strlen($data))return false;
        return true;
    }

    public function close(){
        if($this->socket){
            $arrOpt = array('l_onoff' => 1, 'l_linger' => 1);
            @socket_set_block($this->socket);
            @socket_set_option($this->socket, SOL_SOCKET, SO_LINGER, $arrOpt);
            @socket_shutdown($this->socket);
            @socket_close($this->socket);
        }
        /*if($this->socketListener){
            $arrOpt = array('l_onoff' => 1, 'l_linger' => 1);
            @socket_set_block($this->socketListener);
            @socket_set_option($this->socketListener, SOL_SOCKET, SO_LINGER, $arrOpt);
            @socket_shutdown($this->socketListener);
            @socket_close($this->socketListener);
        }*/
    }

    public function  __destruct() {
        $this->close();
    }
}

abstract class DC {
    const SUPPORTS = 'ADCGet';
    //const SUPPORTS = '';
    const WAIT_FOR_COMMAND_TIMEOUT = 15;

    public $nickname = '';
    public $sharesize = 0;
    public $email = '';
    public $speed = '';
    public $version = '';

    public $onChatReceive = '';
    public $onEnter = '';
    public $onQuit = '';

    public $opponentSupports = array();
    public $hubname = '';
    public $hubtopic = '';

    protected $connection = null;

    public $lasterror;

    protected function processCommand(Command $Command){
        if(!$Command)return true;
        if($Command->command{0}=='<'){
            if($this->onChatReceive && function_exists($this->onChatReceive)){
                call_user_func($this->onChatReceive,
                        $Command->time,$Command->command,$Command->params[0]);
            }
            return true;
        }
        //TODO:move some command processing in DC_P2S and DC_P2P
        switch($Command->command) {
            case 'Supports':
                $this->opponentSupports = $Command->params;
                return true;
            case 'HubName':
                $this->hubname = implode(' ', $Command->params);
                return true;
            case 'HubTopic':
                $this->hubtopic = implode(' ', $Command->params);
                return true;
            case 'Hello':
                //do nothing
                return true;
            case 'MyINFO':
                if($Command->params[1]!=$this->nickname){
                    if(!in_array($Command->params[1], $this->nickList)){
                        $this->nickList[] = $Command->params[1];
                        if($this->onEnter && function_exists($this->onEnter)){
                            call_user_func($this->onEnter, $Command->params[1]);
                        }
                    }
                }
                return true;
            case 'UserIP':
                //TODO: ask my UserIP if none "UserIP2" in hub's supports list
                if($Command->params[0]==$this->nickname){
                    $ip = $Command->params[1];
                    if(substr($ip,-2,2)=='$$'){
                        $ip = substr($ip,0,-2);
                    }
                    $this->myaddrOuter = $ip;
                }
                return true;
            case 'Search':
                //TODO
                return true;
            case 'ConnectToMe':
                //TODO
                return true;
            case 'RevConnectToMe':
                //TODO
                echo "RevConnectToMe received\n";
                return true;
            case 'MaxedOut':
                //TODO
                return true;
            case '!!BitTorrent':
            case '‼BitTorrent':
                //TODO
                return true;
            case 'Quit':
                for($i=0;$i<count($this->nickList);$i++) {
                    if($this->nickList[$i]==$Command->params[0]){
                        unset($this->nickList[$i]);
                        return true;
                    }
                }
                if($this->onQuit && function_exists($this->onQuit)){
                    call_user_func($this->onQuit, $Command->params[0]);
                }
                return true;
            case 'ForceMove':
                throw new ForceMoveException();
                return false;
            case 'Error':
                throw new ErrorReceivedException();
                return false;            
            default:
                if(strpos($Command->command,'BitTorrent')!==false){
                    return true;
                }
                for($i=0;$i<20;$i++){
                    if(strpos($Command->command,chr($i))!==false){
                        return true;
                    }
                }
                print_r($Command);
                return true;
        }
    }

    public function LoopProcessing(){
        while(true){
            $Command = $this->readCommand();
            if($Command){
                if(!$this->processCommand($Command)){
                    return;
                }
            }else{
                sleep(1);
            }
        }
    }

    protected function sendCommand($data){
        if( !$this->connection ) return false;
        if( !$this->connection->isConnected() ){
            $this->connection = null;
            return false;
        }
        return $this->connection->sendCommand($data);
    }

    protected function readBinary($size){
        if( !$this->connection ) return null;
        if( !$this->connection->isConnected() ){
            $this->connection = null;
            return null;
        }
        return $this->connection->readBinary($size);
    }

    public function readCommand(){
        if( !$this->connection ) return null;
        if( !$this->connection->isConnected() ){
            $this->connection = null;
            return null;
        }
        return $this->connection->readCommand();
    }

    /**
     * @param string|array of string
     * @param boolean
     * @return Command
     */
    protected function waitForCommand( $expectedCommand,$debug = false ){
        if(!$expectedCommand)return $this->readCommand();

        //$stepno = 0;
        $start = time();
        while( time() - $start < self::WAIT_FOR_COMMAND_TIMEOUT ) {
            //if($stepno){sleep(1);}
            //$stepno++;
            $Command = $this->readCommand();
            if(!$Command)continue;
            if($debug){
                print_r($Command);
            }
            if( 
                (!is_array($expectedCommand) && $Command->command == $expectedCommand) ||
                (is_array($expectedCommand) && in_array($Command->command, $expectedCommand) )
            ){
                return $Command;
            }else {
                if(!$this->processCommand($Command)){
                    return null;
                }
            }
        }
    }

    protected static function lock2key($lock) {
        $from = array(chr(0),    chr(5),        chr(36),     chr(96),     chr(124),    chr(126));
        $to = array("/%DCN000%/","/%DCN005%/","/%DCN036%/","/%DCN096%/","/%DCN124%/","/%DCN126%/");
        
        $len = strlen($lock);
        $key = '';
        for ($i = 0; $i < $len; $i++) {
            if( $i == 0 ) {
                $b = ord($lock{0}) ^ ord($lock{$len-1}) ^ ord($lock{$len-2}) ^ 5;
            }else {
                $b = ord($lock{$i}) ^ ord($lock{$i-1});
            }
            $b = $b % 256;
            $key .= str_replace($from, $to, chr( (($b<<4) & 240) | (($b>>4) & 15) ) );
        }
        return $key;
    }

    protected static function generateLock() {
        return 'EXTENDEDPROTOCOLABCABCABCABCABCABC';
    }

    protected static function generatePk() {
        return 'DCPLUSPLUS.699ABCABC';
    }
}

class DC_P2S extends DC {
    public $server = '127.0.0.1';
    public $port = 411;
    public $nickList = array();
    public $opList = array();
    //public $myaddr = '';
    public $myport = '';

    protected $myaddrOuter = '';

    public $client;

    public $connectedBack = false;

    public function  __construct($nickname = '', $sharesize = 0,
            $email = '', $speed = '', $version = '') {
        
        if($nickname) $this->nickname = $nickname;
        if($sharesize)$this->sharesize= $sharesize;
        if($email)    $this->email    = $email;
        if($speed)    $this->speed    = $speed;
        if($version)  $this->version  = $version;
    }

    public function connect($server, $port, $myport){
        if($server)$this->server=$server;
        if($port)  $this->port  =$port;
        if($myport)$this->myport=$myport;
        $this->connection = new DCConnection();
        if(!$this->connection->connect($this->server, $this->port)){
            $this->connection = null;
            return false;
        }
        if(!$this->connection->isConnected()){
            $this->connection = null;
            return false;
        }
        return true;
    }

    public function authenticate(){
        $Lock = $this->waitForCommand('Lock');
        if( !$Lock ) return false;
        if( !$this->sendCommand('Supports '.self::SUPPORTS.' ') ) return false;
        $key = self::lock2key($Lock->params[0]);
        if( !$this->sendCommand("Key $key") ) return false;
        if( !$this->sendCommand("ValidateNick $this->nickname") ) return false;
        $Hello = $this->waitForCommand('Hello');
        if( !$Hello ) return false;
        if( $Hello->params[0] != $this->nickname ) return false;

        if( !$this->sendCommand("Version ".$this->version) ) return false;
        if( !$this->sendCommand("GetNickList") ) return false;
        if( !$this->sendCommand('MyINFO $ALL '.$this->nickname.' '.$this->version.
            '$ $'.$this->speed.'$'.$this->email.'$'.$this->sharesize.'$') ) return false;

        $NickList = $this->waitForCommand('NickList');
        if( !$NickList ) return false;
        $this->nickList = explode('$$', $NickList->params[0]);
        array_pop($this->nickList);
        $OpList = $this->waitForCommand('OpList');
        if(!$OpList) return false;
        $this->opList = explode('$$', $OpList->params[0]);
        array_pop($this->opList);
        return true;
    }

    public function getFile($nickName, $fileName) {
        if(FIX_HUB_CP1251){
            $nickName = iconv('utf-8','cp1251',$nickName);
        }
        $this->client = new DC_P2P($this->nickname, $this->myport);
        if( !$this->client->bind() ) {
            $this->lasterror = "couldn't bind p2p socket: ".$this->client->lasterror;
            $this->client->close();
            $this->client = null;
            return false;
        }

        //TODO:ask my UserIP (if old hubs) to get myaddrOuter
        if( !$this->myaddrOuter )$this->myaddrOuter = gethostbyname(gethostname());
        if( !$this->sendCommand('ConnectToMe '.$nickName.
            ' '.$this->myaddrOuter.':'.$this->myport) ) {
            $this->lasterror = "couldn't send ConnectToMe";
            $this->client->close();
            $this->client = null;
            return false;
        }

        if( $this->client->accept() ) {
            if( !$this->client->authenticate($nickName) ){
                $this->lasterror = "couldn't authenticate: ".$this->client->lasterror;
                $this->client->close();
                $this->client = null;
                return false;
            }
        }else{
            //%username% cannot connect to us
            //could we instead connect to him?

            do{
                $ConnectToMe = $this->waitForCommand('ConnectToMe');
                if(!$ConnectToMe){
                    $this->lasterror = "user not connecting to us";
                    $this->client->close();
                    $this->client = null;
                    return false;
                }
                if($ConnectToMe->params[0] != $this->nickname) {
                    $this->lasterror = "ConnectToMe not for me";
                    $this->client->close();
                    $this->client = null;
                    return false;
                }
                list($host,$port) = explode(':', $ConnectToMe->params[1]);
                $this->connectedBack = "$host:$port";
                if(!$this->client->reconnectBack($host, $port)){
                    $this->lasterror = "couldn't reconnect back";
                    $this->client->close();
                    $this->client = null;
                    return false;
                }
                
            //if there was not needed ConnectToMeCommand - wait for other one
            }while (!$this->client->authenticateBack($nickName)) ;
            
        }
        
        $file = $this->client->getFile($fileName);
        if(!$file) {
            $this->lasterror = "binary sending error:".$this->client->lasterror;
            $file = false;
        }
        $this->client->close();
        $this->client = null;
        return $file;
    }

    public function sendMessage($text){
        $this->sendCommand("<$this->nickname> $text");
    }
}

class DC_P2P extends DC {
    const LISTEN_ADDR = '0.0.0.0';
    public $myport = '';
    public $lasterror = '';

    private $needToSendFirst = false;

    public function  __construct($mynickname, $myport) {
        $this->myport = $myport;
        $this->nickname = $mynickname;
        $this->opponentSupports = array();
    }

    public function bind(){
        $this->connection = new DCConnection();
        if( !$this->connection->bind(self::LISTEN_ADDR, $this->myport) ){
            $this->lasterror =  $this->connection->lasterror;
            $this->connection = null;
            return false;
        }
        return true;
    }

    public function accept(){
        if( !$this->connection ) return false;
        if( !$this->connection->accept() ){
            $this->connection = null;
            return false;
        }
        if(!$this->connection->isConnected()){
            $this->connection = null;
            return false;
        }
        return true;
    }

    /*
     * For connecting back to %username% which cannot connect to us
     */
    public function reconnectBack($hostname,$port){
        if($this->connection){
            $this->connection->close();
            //$this->connection->__destruct();
            $this->connection = null;
        }
        
        $this->connection = new DCConnection();
        
        if(!$this->connection->connect($hostname, $port)){
            $this->connection = null;
            return false;
        }
        if(!$this->connection->isConnected()){
            $this->connection = null;
            return false;
        }
        return true;
    }

    public function sendPeerWhatHeWant(){
        //now we just send peer empty list and don't send any file
        if(in_array('ADCGet', $this->opponentSupports)){
            $Command = $this->waitForCommand('ADCGET');
            if($Command->params[1]=='MyList.DcLst'){
                $this->sendCommand('ADCSND file MyList.DcLst 0 0');
            }else{
                $this->sendCommand('Error File Not Found');
            }
        }else{
            $Command = $this->waitForCommand('Get');
            $this->sendCommand('Error File Not Found');
        }
    }

    public function authenticate( $wantedNickname ){
        $MyNick = $this->waitForCommand('MyNick');
        if(!$MyNick){
            $this->lasterror = 'mynick not received';
            return false;
        }
        if($MyNick->params[0] != $wantedNickname){
            $this->lasterror = "it's not who we want";
            return false;
        }
        $Lock = $this->waitForCommand('Lock');
        if(!$Lock){
            $this->lasterror = 'lock not received';
            return false;
        }
        $key = self::lock2key($Lock->params[0]);

        if( !$this->sendCommand('MyNick '.$this->nickname) ) {
            $this->lasterror = 'mynick sending error';
            return false;
        }
        if( !$this->sendCommand('Lock '.self::generateLock().
            ' Pk='.self::generatePk()) ) {
            $this->lasterror = 'lock sending error';
            return false;
        }
        if( !$this->sendCommand('Supports '.self::SUPPORTS.' ')) {
            $this->lasterror = 'Supports sending error';
            return false;
        }
        $myrand = mt_rand(0, 0x7FFF);
        if( !$this->sendCommand("Direction Download $myrand") ) {
            $this->lasterror = 'Direction sending error';
            return false;
        }
        if( !$this->sendCommand('Key '.$key) ) {
            $this->lasterror = 'Key sending error';
            return false;
        }

        $Direction = $this->waitForCommand('Direction');
        if(!$Direction){
            $this->lasterror = 'direction not received';
            return false;
        }

        $Key = $this->waitForCommand('Key');
        if(!$Key){
            $this->lasterror = 'key not received';
            return false;
        }

        if($Direction->params[0]!='Upload' && $Direction->params[1] > $myrand){
            $this->needToSendFirst = true;
        }
        
        return true;
    }

    /*
     * authenticating vise versa: if we connected to %username% instead of him to us
     */
    public function authenticateBack( $wantedNickName ){
        $MyNick = $this->waitForCommand('MyNick');
        if(!$MyNick){
            echo "not awaited mynick\n";
            return false;
        }
        if($MyNick->params[0] != $wantedNickName){
            echo $MyNick->params[0]." instead of $wantedNickName\n";
            return false;
        }

        if( !$this->sendCommand('MyNick '.$this->nickname) ) {
            echo "not sended mynick\n";
            return false;
        }

        if( !$this->sendCommand('Lock '.self::generateLock().
            ' Pk='.self::generatePk()) ) {
            echo "not sended lock\n";
            return false;
        }

        $Lock = $this->waitForCommand('Lock');
        if(!$Lock){
            echo "not awaited lock\n";
            return false;
        }
        if( !$this->sendCommand('Key '.self::lock2key($Lock->params[0])) ) {
            echo "not sended key\n";
            return false;
        }
        
        $Direction = $this->waitForCommand('Direction');
        if(!$Direction){
            echo "not awaited direction\n";
            return false;
        }

        $myrand = mt_rand(0, 0x7FFF);
        if( !$this->sendCommand("Direction Download $myrand") ) {
            echo "not sended direction\n";
            return false;
        }

        $Key = $this->waitForCommand('Key');
        if(!$Key){
            echo "not awaited key\n";
            return false;
        }

        if($Direction->params[0]!='Upload' && $Direction->params[1] > $myrand){
            $this->needToSendFirst = true;
        }
        return true;
    }

    public function getFile( $filename ){
        if($this->needToSendFirst){
            $this->sendPeerWhatHeWant();
            $this->needToSendFirst = false;
        }
        if(in_array('ADCGet', $this->opponentSupports)){
            if( !$this->sendCommand('ADCGET file '.$filename.' 0 -1') ) {
                $this->lasterror = 'ADCGET sending error';
                return false;
            }
            $Answer = $this->waitForCommand(array('ADCSND','MaxedOut'));
            if(!$Answer){
                $this->lasterror = 'ADCSND not received';
                return false;
            }elseif($Answer->command == 'MaxedOut'){
                $this->lasterror = 'no free slots';
                return false;
            }
            $size = $Answer->params[3];
        }else{
            if( !$this->sendCommand('Get '.$filename.'$1') ) {
                $this->lasterror = 'Get sending error';
                return false;
            }
            $FileLength = $this->waitForCommand('FileLength');
            if(!$FileLength) {
                $this->lasterror = 'FileLength not received';
                return false;
            }
            $size = $FileLength->params[0];
            if( !$this->sendCommand('Send') ) {
                $this->lasterror = 'Send command error';
                return false;
            }
        }
        $data = $this->readBinary($size);
        if(!$data){
            $this->lasterror = "binary data empty($size)";
        }
        return $data;
    }

    public function close(){
        if($this->connection){
            $this->connection->close();
            $this->connection = null;
        }
    }
}

?>