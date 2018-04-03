# Copyright(C) 2017 Sergei Golubchik <serg@mariadb.org>
# Copyright(C) 2018 Kouhei Sutou <kou@clear-code.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--let MROONGA_QUERY_LOG_FILE = `SHOW GLOBAL VARIABLES LIKE "mroonga_query_log_file"`
perl;
$mroonga_query_log_file = $ENV{MROONGA_QUERY_LOG_FILE};
$mroonga_query_log_file =~ s/^mroonga_query_log_file	//;
open(F, '<', $_="$ENV{MYSQLD_DATADIR}/$mroonga_query_log_file") or die "open(<$_): $!";
while (<F>) {
  s/^[^|]+\|[^|]+\| *//;
  print;
}
EOF
