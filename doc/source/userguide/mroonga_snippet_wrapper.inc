  CREATE TABLE `snippet_test` (
    `id` int(11) NOT NULL,
    `text` text,
    PRIMARY KEY (`id`),
    FULLTEXT KEY `text` (`text`)
  ) ENGINE=mroonga COMMENT 'engine = "innodb"' DEFAULT CHARSET=utf8

