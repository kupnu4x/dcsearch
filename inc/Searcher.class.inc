<?php
class Searcher {
    private static $DB;
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
                users.nick,
                '1' as rowtype_tth
            from files,users
            where
                files.id in (?a) and
                files.user_id=users.id",
                $ids);
        foreach($files as &$file) {
            $file['type'] = self::getType($file['extension']);
            $file['magnet'] = self::getMagnet($file);
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
                users.nick,
                '1' as rowtype_file
            from files,users
            where
                files.id in (?a) and
                files.user_id=users.id",
                $ids);
        foreach($files as &$file) {
            $file['type'] = self::getType($file['extension']);
            $file['magnet'] = self::getMagnet($file);
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

    public static function getMagnet($file){
        $magnet = 'magnet:?xt=urn:tree:tiger:'.$file['tth'].'&xl='.$file['size'].'&dn='.$file['name'];
        if($file['extension']){
            $magnet .= '.'.$file['extension'];
        }
        return $magnet;
    }

    public static function getType($extension){
        $extension = strtolower($extension);
        $videos = array('avi','wmv','vob','flv','mpg','mp4','mkv','mov','mpeg','3gp','asf',
            'mpe', 'rm', 'divx');
        if(in_array($extension, $videos)) return 'video';

        $audios = array('mp3','wav','ogg','wma','flac','mid','aac','amr');
        if(in_array($extension,$audios)) return 'audio';

        $pictures = array('jpg','gif','png','bmp','ico','swf','jpeg','tif','tiff','cdr','psd',
            '3ds','jpe');
        if(in_array($extension,$pictures)) return 'picture';

        $images = array('vcd','iso','img','mds','mdf','nrg','image','cue');
        if(in_array($extension,$images)) return 'image';

        $documents = array('htm','html','txt','doc','chm','rtf','fb2','pdf','djvu','xls','ppt',
            'mht','odt','ods','odp','docx','xlsx','pptx');
        if(in_array($extension,$documents)) return 'document';

        $programs = array('exe','msi');
        if(in_array($extension,$programs)) return 'program';

        $sysfiles = array('dll','ovl','dat','cab','sys','bat','cmd','com','ocx','obj');
        if(in_array($extension, $sysfiles)) return 'sysfile';

        $satellites = array('js','css','xml');
        if(in_array($extension, $satellites)) return 'satellite';

        $archives = array('rar','zip','7z','uha','tar','gz','bz2','tgz');
        if(in_array($extension, $archives)) return 'archive';

        $inifiles = array('ini','lst','inf','ifo','cfg','nfo','config','conf','info');
        if(in_array($extension, $inifiles)) return 'inifile';

        return 'file';
    }
}
?>