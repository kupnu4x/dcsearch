#!/bin/sh
echo ==SPHINX INDEXER WEEKLY==`date "+%Y-%m-%d %H:%M:%S"`==
/usr/bin/time -h /usr/local/www/apache22/data/dcsearch/scripts/indexer.delta.sh 2>&1 | tail -n 1