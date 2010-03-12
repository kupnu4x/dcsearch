#!/bin/sh
echo `date "+%Y-%m-%d %H:%M:%S"`==SPHINX INDEXER WEEKLY====
time -h /usr/local/www/apache22/data/dcsearch/scripts/indexer.main.sh > /dev/null
