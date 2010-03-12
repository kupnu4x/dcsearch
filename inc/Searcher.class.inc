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
                users.lastlisttime as lasttime,
                '1' as rowtype_tth
            from files,users
            where
                files.id in (?a) and
                files.user_id=users.id",
                $ids);
        foreach($files as &$file) {
            $file['type'] = self::getType($file['extension']);
            $file['magnet'] = self::getMagnet($file);
            $file['lasttime'] = date('Y-m-dd H:i:s', $file['lasttime']);
            $file['size'] = self::HumanSize($file['size']);
        }
        return $files;
    }

    public static function getFiles($ids){
        $DB = self::getDB();
        $files = $DB->select("select
                files.size,
                files.tth,
                files.name,
                files.fullpath,
                files.extension,
                files.starttime,
                users.nick,
                users.lastlisttime as lasttime,
                '1' as rowtype_file
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

    public static function getDirs($ids){
        $DB = self::getDB();
        return $DB->select("select
                dirs.fullpath,
                dirs.name,
                users.nick,
                '1' as rowtype_dir
            from dirs,users
            where
                dirs.id in (?a) and
                dirs.user_id=users.id",
                $ids);
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

    public static function getCategories(){
        return array(
            array('key'=>'video'     ,'value'=>'Videos'),
            array('key'=>'audio'     ,'value'=>'Audios'),
            array('key'=>'picture'   ,'value'=>'Pictures'),
            array('key'=>'images'    ,'value'=>'CD Images'),
            array('key'=>'documents' ,'value'=>'Txt & Office documents'),
            array('key'=>'programs'  ,'value'=>'Programs'),
            array('key'=>'sysfiles'  ,'value'=>'System files'),
            //array('key'=>'satellites','value'=>'satellites'),
            array('key'=>'archives'  ,'value'=>'Archives'),
            array('key'=>'inifiles'  ,'value'=>'Ini & Config files')
            );
    }

    public static function getExts($category){
        $array = self::${$category."_exts"};
        return $array;
    }

    public static function getLatestIds($category,$days){
        $ids = MC::get('latest_'.$category.'_'.$days);
        if(!$ids){
            $exts = self::getExts($category);
            $DB = self::getDB();
            $id = $DB->selectCell('select min(id) from files where starttime >= NOW() - interval ?n day',$days);
            $ids = $DB->selectCol('select id 
                from files f
                where
                    id>=? and
                    extension in (?a) and
                    not exists (select 1 from files temptable where id<? and temptable.tth=f.tth)
                order by id desc
                limit 0,1000',
                $id,$exts,$id);
            MC::set('latest_'.$category.'_'.$days, $ids);
        }
        return $ids;
    }

    public static function getLatest($category,$days,$page,$RPP){
        $st = microtime(true);
        $ids = self::getLatestIds($category, $days);
        $total_results = count($ids);
        $ids = array_splice($ids, ($page-1)*$RPP, $RPP);
        $results = self::getFiles($ids);
        $et = microtime(true);
        $time = round($et-$st,3);
        return array($results, $total_results, $time);
    }

}
?>