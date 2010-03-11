-- phpMyAdmin SQL Dump
-- version 3.2.0.1
-- http://www.phpmyadmin.net
--
-- Хост: localhost
-- Время создания: Мар 04 2010 г., 10:46
-- Версия сервера: 5.1.28
-- Версия PHP: 5.3.0

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- База данных: `dcsearch`
--

-- --------------------------------------------------------

--
-- Структура таблицы `dirs`
--

CREATE TABLE `dirs` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `fullpath` text NOT NULL,
  `name` varchar(1024) NOT NULL DEFAULT '',
  `deleted` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `deleted` (`deleted`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `files`
--

CREATE TABLE `files` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `size` bigint(20) unsigned NOT NULL DEFAULT '0',
  `tth` varchar(39) NOT NULL DEFAULT '',
  `fullpath` text NOT NULL,
  `name` varchar(1024) NOT NULL DEFAULT '',
  `extension` varchar(10) NOT NULL DEFAULT '',
  `starttime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `deleted` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `deleted` (`deleted`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `sphinx`
--

CREATE TABLE `sphinx` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `last_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Структура таблицы `users`
--

CREATE TABLE `users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nick` varchar(50) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `filename` varchar(50) NOT NULL,
  `lastlisttime` int(10) unsigned DEFAULT NULL,
  `filelist_crc32` int(11) DEFAULT NULL,
  `files` int(10) unsigned NOT NULL DEFAULT '0',
  `dirs` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;
