<?php
class MC {
    private static $MC;
    /**
     * @return Memcache
     */
    private static function getMCObj(){
        if( !isset( self :: $MC ) ) {
            self::$MC = new Memcached();
            self::$MC->addServer( MC_HOST, MC_PORT );
        }
        return self::$MC;
    }

    public static function get($key){
        $MC = self::getMCObj();
        return $MC->get($key);
    }

    public static function set($key,$value){
        $MC = self::getMCObj();
        return $MC->set($key, $value);
    }
}
?>