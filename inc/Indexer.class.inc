<?php
class Indexer {
    private static $DB;
    private static $XmlReader;

    /**
     * @return DbSimple_Mysql
     */
    public static function getDB(){
        if(!self::$DB){
            self::$DB = & DbSimple_Generic::connect(
                    'mysql://'.DB_USER.':'.DB_PASS.'@'.DB_HOST.'/'.DB_NAME);
            self::$DB->query("SET NAMES 'utf8' COLLATE 'utf8_general_ci'");
        }
        return self::$DB;
    }

    /**
     * @return XMLReader
     */
    private static function getXmlReader(){
        if(!self::$XmlReader){
            self::$XmlReader = new XMLReader();
        }
        return self::$XmlReader;
    }

    public static function freeXmlReader(){
        if(self::$XmlReader){
            self::$XmlReader = null;
        }
    }

    public static function getNeedNicks($serverNicks){
        if(FIX_HUB_CP1251){
            foreach($serverNicks as $k=>$v) {
                $serverNicks[$k] = @iconv('cp1251', 'utf-8', $v);
            }
        }

        $DB = self::getDB();
        //select users we don't need
        $bdNicks = $DB->selectCol("select nick
                from users
            where
                nick in (?a) and
                (filename <>'' or lastlisttime > ?d)",
            $serverNicks, time() - INTERVAL_NICKNAME );
        
        $needNicks = array_diff($serverNicks, $bdNicks);

        return $needNicks;
    }

    public static function setNickList($nick, $list_fn){
        $DB = self::getDB();
        $nickRow = $DB->selectRow('select * from users where nick=?',$nick);
        if($nickRow){
            $DB->query('update users
                    set filename=?
                    where id=?',
                    $list_fn, $nickRow['id']);
        }else{
            $DB->query('insert into users
                    set
                        nick=?,
                        filename=?',
                    $nick, $list_fn);
        }
    }

    public static function getUsersToProcess(){
        $DB = self::getDB();
        return $DB->select("select id,nick,filename,files,dirs from users where filename<>''");
    }

    public static function delOldUsers(){
        $DB = self::getDB();
        //id=0:
        $DB->query('delete from files where id=0');
        $DB->query('delete from dirs where id=0');
        //!
        $ids = $DB->selectCol("select id
                from users
            where
                isnull(lastlisttime) or
                (lastlisttime < ?d)",
             time() - INTERVAL_NICKNAME_DIE );
        $DB->query('delete from files where user_id in (?a)',$ids);
        $DB->query('delete from dirs where user_id in (?a)',$ids);
        $DB->query('delete from users where id in (?a)',$ids);
        return count($ids);
    }

    //just skip this list
    //ex: file not exists
    public static function skipUser($user_id){
        $DB = self::getDB();
        $DB->query("update users set filename='' where id=?",$user_id);
    }

    
    public static function processUser($user_id,$xml_url,$wasfiles,$wasdirs){
        $xml_str = file_get_contents($xml_url);
        if(!$xml_str){
            return array(0,0,0,0);
        }
        $DB = self::getDB();

        $crc32_old = $DB->selectCell('select filelist_crc32 
            from users
            where id=?',$user_id);
        $crc32_new = crc32($xml_str);

        if($crc32_old && ($crc32_old == $crc32_new) ){
            $DB->query("update users 
                set
                    filename='',
                    lastlisttime=?d
                where id=?", time(),$user_id );
            return array(0,0,0,0);
        }

        $files_old_ids = $DB->selectCol("select 
                md5(concat(tth,'/',fullpath,'/',name,'/',extension)) as ARRAY_KEY,
                id
            from files
            where user_id=?",$user_id);
        if(!$files_old_ids){
            $files_old_ids = array();
        }
        $dirs_old_ids = $DB->selectCol("select 
                md5(concat(fullpath,'/',name)) as ARRAY_KEY,
                id
            from dirs
            where user_id=?",$user_id);
        if(!$dirs_old_ids){
            $dirs_old_ids = array();
        }
        
        $files_to_add = array();
        $dirs_to_add = array();

        $XmlReader = self::getXmlReader();
        $XmlReader->XML($xml_str);
        //$XmlReader->open($xml_url);

        $currPath = array();
        $fullpath = '';
        while($XmlReader->read()){
            if( ($XmlReader->name == 'Directory') &&
                ($XmlReader->nodeType == XMLReader::ELEMENT)
            ){
                $dirname = $XmlReader->getAttribute('Name');
                $fullpath = implode('/', $currPath);
                $hash = md5("$fullpath/$dirname");
                
                if(isset($dirs_old_ids[$hash])){
                    unset($dirs_old_ids[$hash]);
                    //$dirs_old_ids[$hash] = 0;
                }else{
                    $dirs_to_add[] = array(
                        'name'=>$dirname,
                        'fullpath'=>$fullpath
                    );
                }
                $currPath[] = $dirname;
                $fullpath .= "/$dirname";
            }elseif( ($XmlReader->name == 'Directory') &&
                ($XmlReader->nodeType == XMLReader::END_ELEMENT)){
                array_pop($currPath);
                $fullpath = implode('/', $currPath);
            }elseif( $XmlReader->name =='File'){
                $size = $XmlReader->getAttribute('Size');
                if($size < MIN_FILESIZE){
                    continue;
                }
                $tth = $XmlReader->getAttribute('TTH');
                $name = $XmlReader->getAttribute('Name');
                $pathinfo = pathinfo($name);
                $name = $pathinfo['filename'];
                $extension = isset($pathinfo['extension'])?$pathinfo['extension']:'';
                $hash = md5("$tth/$fullpath/$name/$extension");
                if(isset($files_old_ids[$hash])){
                    unset($files_old_ids[$hash]);
                    //$files_old_ids[$hash] = 0;
                }else{
                    
                    $files_to_add[] = array(
                        'size'=>$size,
                        'tth'=>$tth,
                        'fullpath'=>$fullpath,
                        'name'=>$name,                        
                        'extension'=>$extension
                    );
                }
            }
        }
        

        $files_to_remove = array_diff(array_values($files_old_ids),array(0));
        unset($files_old_ids);
        $dirs_to_remove = array_diff(array_values($dirs_old_ids),array(0));
        unset($dirs_old_ids);

        $DB->transaction();
        if( is_array($files_to_remove) && count($files_to_remove) ){
            $DB->query("update files set deleted='1' where id in (?a)",$files_to_remove);
            $deleted_files = count($files_to_remove);
            unset($files_to_remove);
        }else{
            $deleted_files = 0;
        }
        if( is_array($dirs_to_remove) && count($dirs_to_remove) ){
            $DB->query("update dirs set deleted='1' where id in (?a)",$dirs_to_remove);
            $deleted_dirs = count($dirs_to_remove);
            unset($dirs_to_remove);
        }else{
            $deleted_dirs = 0;
        }
        ////////////add files
        //$test_queies = array();

        $query_start = 'insert into files (user_id,size,tth,name,fullpath,extension) values';
        $sql = '';$i=0;
        foreach($files_to_add as $file) {
            if( $i % 100 == 0){
                if($sql){
                    $DB->query($sql);
                    //$test_queies[] = $sql;
                }
                $sql = $query_start;
            }else{
                $sql.=',';
            }
            $sql .= "(".$user_id.",".
                    $file['size'].",'".
                    mysql_real_escape_string($file['tth'])."','".
                    mysql_real_escape_string($file['name'])."','".
                    mysql_real_escape_string($file['fullpath'])."','".
                    mysql_real_escape_string($file['extension'])."')";
            $i++;
        }
            
        if($sql){
            $DB->query($sql);
            //$test_queies[] = $sql;
        }

            //file_put_contents( "./tmp/queries_files_$user_id.dump", json_encode($test_queies));
        $added_files = count($files_to_add);
        unset($files_to_add);
        ////////////!add files
        $DB->commit();
        sleep(1+round($added_files/10000));//win+mysql 5.1: id=0 fixing
        $DB->transaction();
        ////////////add dirs
        //$test_queies = array();
        $query_start = 'insert into dirs (user_id,name,fullpath) values';
        $sql = '';$i=0;
        foreach($dirs_to_add as $dir) {
            if( $i % 100 == 0){
                if($sql){
                    //$test_queies[] = $sql;
                    $DB->query($sql);
                }
                $sql = $query_start;
            }else{
                $sql.=',';
            }
            $sql .= "(".$user_id.",'".
                    mysql_real_escape_string($dir['name'])."','".
                    mysql_real_escape_string($dir['fullpath'])."')";
            $i++;
        }
        if($sql){
            //$test_queies[] = $sql;
            $DB->query($sql);
        }
            //file_put_contents( "./tmp/queries_dirs_$user_id.dump", json_encode($test_queies));
        $added_dirs=count($dirs_to_add);
        unset($dirs_to_add);
        ////////////!add dirs
        $DB->query("update users 
            set
                filename='',
                lastlisttime=?d,
                filelist_crc32=?,
                files=?,
                dirs=?
            where id=?",
                
                time(),
                $crc32_new,
                $wasfiles+$added_files-$deleted_files,
                $wasdirs+$added_dirs-$deleted_dirs,
                $user_id );
        $DB->commit();
        return array($added_files,$added_dirs,$deleted_files,$deleted_dirs);
    }
}
?>