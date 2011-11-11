--disable_query_log
let $inplace_index_change = `SELECT @@global.version >= '5.5'`;
--enable_query_log
if (!$inplace_index_change)
{
  skip Inplace index change is availavle in version 5.5 or later;
}
