package My::Suite::Mroonga;

@ISA = qw(My::Suite);

return "No Mroonga engine" unless $ENV{HA_MROONGA_SO} or
                                  $::mysqld_variables{'mroonga'} eq "ON";

sub is_default { 1 }

if (-d "../sql")
{
  $ENV{GRN_PLUGINS_DIR}=$::basedir . '/storage/mroonga/vendor/groonga/vendor/plugins/groonga-normalizer-mysql';
}

bless { };

