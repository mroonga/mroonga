# ccache doesn't work with /Zi.
# See also: https://github.com/ccache/ccache/issues/1040
string(APPEND CMAKE_C_FLAGS " /Z7")
string(APPEND CMAKE_CXX_FLAGS " /Z7")
foreach(lang C CXX)
  foreach(flag_suffix
      ""
      "_RELEASE"
      "_RELWITHDEBINFO"
      "_DEBUG"
      "_MINSIZEREL")
    string(REPLACE "/Zi" "/Z7"
      CMAKE_${lang}_FLAGS${flag_suffix}
      "${CMAKE_${lang}_FLAGS${flag_suffix}}")
  endforeach()
endforeach()

# XXX: Zstandard requires ASM. So we want to enable ASM in top-level.
# I don't know why "project(zstd LANGUAGES C ASM CXX)" doesn't work...
enable_language(ASM)
