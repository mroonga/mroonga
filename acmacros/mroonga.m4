m4_define([mrn_version_major], [1])
m4_define([mrn_version_minor], [1])
m4_define([mrn_version_micro], [0])
m4_define([mrn_version],
          [mrn_version_major.mrn_version_minor[]mrn_version_micro])

AC_DEFUN([MRN_VERSION_SETUP],
[
  AC_DEFINE([MRN_VERSION_MAJOR],
	    [mrn_version_major],
	    [Define groonga storage engine major version])
  AC_DEFINE([MRN_VERSION_MINOR],
	    [mrn_version_minor],
	    [Define groonga storage engine minor version])
  AC_DEFINE([MRN_VERSION_MICRO],
	    [mrn_version_micro],
	    [Define groonga storage engine micro version])
  mrn_version_in_hex=`printf "0x%02x%02x" mrn_version_major mrn_version_minor[]mrn_version_micro`
  AC_DEFINE_UNQUOTED([MRN_VERSION_IN_HEX],
		     [$mrn_version_in_hex],
		     [Define groonga storage engine version in hex])
  AC_DEFINE_UNQUOTED([MRN_VERSION],
		     ["$PACKAGE_VERSION"],
		     [Define groonga storage engine version in string])
])
