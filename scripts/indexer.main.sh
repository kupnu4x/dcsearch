#!/bin/sh
/usr/local/bin/indexer --config /usr/local/etc/sphinx.conf dc_files dc_files_delta dc_dirs dc_dirs_delta dc_tths dc_tths_delta --rotate
