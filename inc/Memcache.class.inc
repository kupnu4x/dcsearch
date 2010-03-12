<?php
class MС {
    /**
     * @var Memcache
     */
    private static $MCObj = false;
    /**
     * @return Memcache
     */
    private static function getMCObj(){
        if(!self::$MCObj){
            self::$MCObj = new Memcache();
            self::$MCObj->addServer(MC_HOST,MC_PORT);
        }
        return self::$MCObj;
    }

    public static function get($key){
        $MC = self::getMCObj();
        return $MC->get($key);
    }

    public static function set($key,$var){
        $MC = self::getMCObj();
        return $MC->set($key, $var);
    }
    
}
?>