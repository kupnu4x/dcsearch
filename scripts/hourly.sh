#!/bin/sh
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/lister.php
/usr/local/bin/php /usr/local/www/apache22/data/dcsearch/processer.php
time -hl /usr/local/www/apache22/data/dcsearch/scripts/indexer.delta.sh > /dev/null
