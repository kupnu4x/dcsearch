<?php
class Searcher {
    private static $DB;
    private static $video_exts     = array('avi','wmv','vob','flv','mpg','mp4','mkv','mov','mpeg','3gp','asf',
                                        'mpe', 'rm', 'divx');
    private static $audio_exts     = array('mp3','wav','ogg','wma','flac','mid','aac','amr');
    private static $picture_exts   = array('jpg','gif','png','bmp','ico','swf','jpeg','tif','tiff','cdr','psd',
                                        '3ds','jpe');
    private static $images_exts    = array('vcd','iso','img','mds','mdf','nrg','image','cue');
    private static $documents_exts = array('htm','html','txt','doc','chm','rtf','fb2','pdf','djvu','xls','ppt',
                                        'mht','odt','ods','odp','docx','xlsx','pptx');
    private static $programs_exts  = array('exe','msi');
    private static $sysfiles_exts  = array('dll','ovl','dat','cab','sys','bat','cmd','com','ocx','obj');
    private static $satellites_exts= array('js','css','xml');
    private static $archives_exts  = array('rar','zip','7z','uha','tar','gz','bz2','tgz');
    private static $inifiles_exts  = array('ini','lst','inf','ifo','cfg','nfo','config','conf','info');
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

    public static function getTTHs($ids){
        $DB = self::getDB();
        $files = $DB->select("select
                files.size,
                files.tth,
                files.name,
                files.fullpath,
                files.extension,
                files.starttime,
                users.nick,
                users.lastseedtime as lasttime,
                '1' as rowtype_tth
            from files,users
            where
                files.id in (?a) and
                files.user_id=users.id",
                $ids);
        foreach($files as &$file) {
            $file['type'] = self::getType($file['extension']);
            $file['magnet'] = self::getMagnet($file);
            $file['lasttime'] = date('Y-m-d H:i:s', $file['lasttime']);
            $file['size'] = self::HumanSize($file['size']);
        }
        return $files;
    }

    public static function getFiles($ids){
        $DB = self::getDB();
        $files_by_id = $DB->select("select
                files.id AS ARRAY_KEY,
                files.size,
                files.tth,
                files.name,
                files.fullpath,
                files.extension,
                files.starttime,
                users.nick,
                users.lastseedtime as lasttime,
                '1' as rowtype_file
            from files,users
            where
                files.id in (?a) and
                files.user_id=users.id",
                $ids);
        $files = array();
        foreach ($ids as $id){ //correct the order as source
            $files[] = $files_by_id[$id];
        }
        foreach($files as &$file) {
            $file['type'] = self::getType($file['extension']);
            $file['magnet'] = self::getMagnet($file);
            $file['lasttime'] = date('Y-m-d H:i:s', $file['lasttime']);
            $file['size'] = self::HumanSize($file['size']);
        }
        return $files;
    }

    public static function getDirs($ids){
        $DB = self::getDB();
        $dirs_by_id = $DB->select("select
                dirs.id AS ARRAY_KEY,
                dirs.fullpath,
                dirs.name,
                dirs.starttime,
                users.nick,
                users.lastseedtime as lasttime,
                '1' as rowtype_dir
            from dirs,users
            where
                dirs.id in (?a) and
                dirs.user_id=users.id",
                $ids);
        $dirs = array();
        foreach ($ids as $id){ //correct the order as source
            $dirs[] = $dirs_by_id[$id];
        }
        foreach($dirs as &$dir) {
            $dir['lasttime'] = date('Y-m-d H:i:s', $dir['lasttime']);
        }
        return $dirs;
    }

    public static function HumanSize($size){
        if($size<1024) return $size." B";
        $size = round($size/1024,2);
        if($size<1024) return $size." KB";
        $size = round($size/1024,2);
        if($size<1024) return $size." MB";
        $size = round($size/1024,2);
        if($size<1024) return $size." GB";
        $size = round($size/1024,2);
        return $size."TB";
    }

    public static function getMagnet($file){
        $magnet = 'magnet:?xt=urn:tree:tiger:'.$file['tth'].'&xl='.$file['size'].'&dn='.$file['name'];
        if($file['extension']){
            $magnet .= '.'.$file['extension'];
        }
        return $magnet;
    }

    public static function getType($extension){
        $extension = strtolower($extension);
        if(in_array($extension, self::$video_exts))      return 'video';
        if(in_array($extension, self::$audio_exts))      return 'audio';
        if(in_array($extension, self::$picture_exts))    return 'picture';
        if(in_array($extension, self::$images_exts))     return 'image';
        if(in_array($extension, self::$documents_exts))  return 'document';
        if(in_array($extension, self::$programs_exts))   return 'program';
        if(in_array($extension, self::$sysfiles_exts))   return 'sysfile';
        if(in_array($extension, self::$satellites_exts)) return 'satellite';
        if(in_array($extension, self::$archives_exts))   return 'archive';
        if(in_array($extension, self::$inifiles_exts))   return 'inifile';
        return 'file';
    }

    public static function getCategories($selected = false,$showall = false){
        $array = array();
        if($showall){
            $array[] = array('key'=>''     ,'value'=>'All files');
        }
        $array[] = array('key'=>'video'     ,'value'=>'Videos');
        $array[] = array('key'=>'audio'     ,'value'=>'Audios');
        $array[] = array('key'=>'picture'   ,'value'=>'Pictures');
        $array[] = array('key'=>'images'    ,'value'=>'CD Images');
        $array[] = array('key'=>'documents' ,'value'=>'Documents');
        $array[] = array('key'=>'programs'  ,'value'=>'Programs');
        $array[] = array('key'=>'sysfiles'  ,'value'=>'System files');
        //$array[] = array('key'=>'satellites','value'=>'satellites');
        $array[] = array('key'=>'archives'  ,'value'=>'Archives');
        $array[] = array('key'=>'inifiles'  ,'value'=>'Config files');
        if($selected){
            foreach($array as $i=>$row) {
                if($row['key']==$selected){
                    $array[$i]['selected'] = true;
                }
            }
        }
        return $array;
    }

    public static function getExts($category){
        $array = self::${$category."_exts"};
        return $array;
    }

    public static function getExtsCrc32($category){
        $array = self::getExts($category);
        for ($i=0;$i<count($array);$i++){
            $array[$i] = crc32($array[$i]);
        }
        return $array;
    }

    public static function correctHumanSize($size){
        return (bool)preg_match('/^\s*(\d+)([\.\,]\d*)?\s*(k|kb|m|mb|g|gb|t|tb)?\s*$/i', $size);
    }

    public static function getSizeFromHuman($size){
        preg_match('/^\s*(\d+)([\.\,]\d*)?\s*(b|k|kb|m|mb|g|gb|t|tb)?\s*$/i', $size, $matches);
        $number = (int)$matches[1];
        if($matches[2]){
            $matches[2] = str_replace(',', '.', $matches[2]);
            $number += (float)"0".$matches[2];
        }
        if($matches[3]){
            $matches[3] = strtolower($matches[3]);
                if($matches[3]=='k'||$matches[3]=='kb') $number *= 1024;
            elseif($matches[3]=='m'||$matches[3]=='mb') $number *= 1024*1024;
            elseif($matches[3]=='g'||$matches[3]=='gb') $number *= 1024*1024*1024;
            elseif($matches[3]=='t'||$matches[3]=='tb') $number *= 1024*1024*1024*1024;
        }
        $number = round($number);
        return (int)$number;
    }

    public static function getLatestIds($category,$days,$minsize){
        $DB = self::getDB();
        $maxid = $DB->selectCell('select max(id) from files');
        $minsize = self::getSizeFromHuman($minsize);
        $ids = MC::get('latest_'.$category.'_'.$days.'_'.$minsize.'_'.$maxid);
        if(!$ids){
            $exts = self::getExts($category);
            
            $id = $DB->selectCell('select min(id) from files where starttime >= NOW() - interval ?d day',$days);
            $ids = $DB->selectCol('select min(id) as ARRAY_KEY, count(id)
                from files f
                where
                    id>=? and
                    extension in (?a) and
                    size>=?d and
                    not exists (select 1 from files temptable where id<? and temptable.tth=f.tth)
                group by tth
                order by id desc
                limit 0,1000',
                $id,$exts,$minsize,$id);
            var_dump($ids);
            MC::set('latest_'.$category.'_'.$days.'_'.$minsize.'_'.$maxid, $ids);
        }
        return $ids;
    }

    public static function getLatest($category,$days,$minsize,$page,$RPP){
        $st = microtime(true);
        $ids = self::getLatestIds($category, $days, $minsize);
        $total_results = count($ids);
        $ids = array_splice($ids, ($page-1)*$RPP, $RPP);
        $results = self::getFiles($ids);
        $et = microtime(true);
        $time = round($et-$st,3);
        return array($results, $total_results, $time);
    }

}
?>