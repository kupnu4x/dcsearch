#!/bin/sh
echo ==LISTER=================`date "+%Y-%m-%d %H:%M:%S"`==
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/lister.php
echo ==PROCESSER==============`date "+%Y-%m-%d %H:%M:%S"`==
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/processer.php
echo ==SPHINX INDEXER=========`date "+%Y-%m-%d %H:%M:%S"`==
/usr/bin/time -h /usr/local/www/apache22/data/dcsearch/scripts/indexer.delta.sh 2>&1 | tail -n 1
