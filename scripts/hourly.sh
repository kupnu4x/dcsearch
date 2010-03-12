#!/bin/sh
echo `date "+%Y-%m-%d %H:%M:%S"`==LISTER=============
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/lister.php
echo `date "+%Y-%m-%d %H:%M:%S"`==PROCESSER==========
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/processer.php
echo `date "+%Y-%m-%d %H:%M:%S"`==SPHINX INDEXER=====
time -h /usr/local/www/apache22/data/dcsearch/scripts/indexer.delta.sh > /dev/null 2>&1
