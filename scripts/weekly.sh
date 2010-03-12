#!/bin/sh
echo ==SPHINX INDEXER WEEKLY======================`date "+%Y-%m-%d %H:%M:%S"`==
echo `/usr/bin/time -h /usr/local/www/apache22/data/dcsearch/scripts/indexer.main.sh 2>&1 | tail -n 1`
