package My::Suite::Mroonga;

@ISA = qw(My::Suite);

return "No Mroonga engine" unless $ENV{HA_MROONGA_SO} or
                                  $::mysqld_variables{'mroonga'} eq "ON";

sub is_default { 1 }

my $groonga_normalizer_mysql_dir=$::basedir . '/storage/mroonga/vendor/groonga/vendor/plugins/groonga-normalizer-mysql';

if (-d $groonga_normalizer_mysql_dir)
{
  $ENV{GRN_PLUGINS_DIR}=$groonga_normalizer_mysql_dir;
}

bless { };

