<?php
set_time_limit(0);
define('RPP',50);
require_once(dirname(__FILE__).'/inc/config.inc');

$page = isset($_GET['p'])?(int)$_GET['p']:1;
$query = isset($_GET['q'])?(string)$_GET['q']:'';
$category = isset($_GET['cat'])?(string)$_GET['cat']:'';
$days = isset($_GET['d'])?(int)$_GET['d']:0;
$days = max($days,0);
$extsearch = isset($_GET['extsearch'])?(bool)$_GET['extsearch']:false;
$nodirs = isset($_GET['nodirs'])?(bool)$_GET['nodirs']:false;

$T = new Blitz();
$T->load(file_get_contents('tpl/index.tpl'));
$tpl_values = array();
$tpl_values['page'] = $page;
if($query || $extsearch){

    $tpl_values['query'] = htmlspecialchars($query);
    $tpl_values['extsearch'] = $extsearch;
    $tpl_values['nodirs'] = $nodirs;
    $categories = Searcher::getCategories($category,true);
    $tpl_values['categories'] = $categories;
    $tpl_values['days'] = $days;

    if($query){
        $searcher = new SphinxClient();
        $searcher->setServer("localhost", 3312);
        $searcher->setMatchMode(SPH_MATCH_ALL);
        $searcher->setSortMode(SPH_SORT_RELEVANCE);
        $searcher->setMaxQueryTime(3000);

        $min = ($page-1)*RPP;
        $max = $min + RPP;//max+1

        $out_array = array();
        //TTHS
        $prev_instanses_count = 0;
        $start = max(0,$min-$prev_instanses_count);
        $len = min(RPP, max(1,$max-$prev_instanses_count) );
        $searcher->setLimits( $start, $len );
        $tths_result = $searcher->query($query,"dc_tths dc_tths_delta");
        $total_tths = $tths_result['total'];
        if($total_tths && is_array($tths_result['matches']) && count($out_array)<RPP){
            $tths = Searcher::getTTHs(array_keys($tths_result['matches']));
            $out_array = array_merge($out_array,$tths);
        }

        if(!$nodirs){
            //DIRS
            $prev_instanses_count += $total_tths;
            $start = max(0,$min-$prev_instanses_count);
            $len = min(RPP, max(1,$max-$prev_instanses_count) );
            $searcher->setLimits( $start, $len );
            $dirs_result = $searcher->query($query,"dc_dirs dc_dirs_delta");
            $total_dirs = $dirs_result['total'];
            if($total_dirs && is_array($dirs_result['matches']) && count($out_array)<RPP){
                $dirs = Searcher::getDirs(array_keys($dirs_result['matches']));
                $out_array = array_merge($out_array,$dirs);
            }
        }

        if($days){
            $searcher->SetFilterRange("starttime", 0, time()-$days*24*60*60, true); //exclude too old results
        }
        if($category){
            $searcher->SetFilter("extension_crc32", Searcher::getExtsCrc32($category));
        }
        //FILES
        $prev_instanses_count += $total_dirs;
        $start = max(0,$min-$prev_instanses_count);
        $len = min(RPP, max(1,$max-$prev_instanses_count) );
        $searcher->setLimits( $start, $len );
        $files_result = $searcher->query($query,"dc_files dc_files_delta");
        $total_files = $files_result['total'];
        if($total_files && is_array($files_result['matches']) && count($out_array)<RPP){
            $files = Searcher::getFiles(array_keys($files_result['matches']));
            $out_array = array_merge($out_array,$files);
        }

        $total_pages = min(ceil(1000/RPP),ceil( ($total_tths+$total_dirs+$total_files)/RPP ));
        if($total_pages>1){
            $pagination = array_fill(1, $total_pages, array('selected'=>false,'query'=>urlencode($query)));
            $pagination[$page]['selected'] = true;
            $tpl_values['pagination'] = true;
            $tpl_values['pagination_search'] = $pagination;
            if($page>1){
                if($page==2){
                    $tpl_values['prevlink'] = '?q='.urlencode($query);
                }else{
                    $tpl_values['prevlink'] = '?p='.($page-1).'&q='.urlencode($query);
                }
            }
            if($page<$total_pages){
                $tpl_values['nextlink'] = '?p='.($page+1).'&q='.urlencode($query);
            }
        }

        if(!count($out_array))$tpl_values['nofound'] = true;
        $tpl_values['results'] = $out_array;
        $tpl_values['time'] = $tths_result['time']+$dirs_result['time']+$files_result['time'];
        $tpl_values['powered_sphinx'] = true;
    }
}elseif($category && $days){
    $days = max($days,1);

    $tpl_values['hide_search_form'] = true;
    $tpl_values['category'] = htmlspecialchars($category);
    $tpl_values['days'] = htmlspecialchars($days);
    $categories = Searcher::getCategories($category);
    $tpl_values['filter_last'] = array(
        'categories'=>$categories,
        //'category'=>$category,
        'days'=>$days
    );

    list($results, $total_results, $time) = Searcher::getLatest($category,$days,$page,RPP);
    $total_pages = ceil( ($total_results)/RPP );
    if($total_pages>1){
        $pagination = array_fill(1, $total_pages, array('selected'=>false,'category'=>urlencode($category),'days'=>urlencode($days)));
        $pagination[$page]['selected'] = true;
        $tpl_values['pagination'] = true;
        $tpl_values['pagination_viewlast'] = $pagination;
        if($page>1){
            if($page==2){
                $tpl_values['prevlink'] = '?cat='.urlencode($category).'&d='.$days;
            }else{
                $tpl_values['prevlink'] = '?p='.($page-1).'&cat='.urlencode($category).'&d='.$days;
            }
        }
        if($page<$total_pages){
            $tpl_values['nextlink'] = '?p='.($page+1).'&cat='.urlencode($category).'&d='.$days;
        }
    }
    if(!count($results))$tpl_values['nofound'] = true;
    $tpl_values['results'] = $results;
    $tpl_values['time'] = $time;
    $tpl_values['powered_mysql'] = true;
}else{
    $tpl_values['viewlast_categories'] = Searcher::getCategories();
}
$T->set($tpl_values);
echo $T->parse();