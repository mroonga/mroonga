--disable_query_log
let $fulltext_index_comment = `SELECT @@global.version >= '5.5'`;
--enable_query_log
if (!$fulltext_index_comment)
{
  skip Fulltext index comment is available in version 5.5 or later;
}
