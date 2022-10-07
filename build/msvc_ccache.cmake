# ccache doesn't work with /Zi.
# See also: https://github.com/ccache/ccache/issues/1040
foreach(lang C CXX)
  foreach(flag_suffix
      ""
      "_INIT"
      "_RELEASE"
      "_RELEASE_INIT"
      "_RELWITHDEBINFO"
      "_RELWITHDEBINFO_INIT"
      "_DEBUG"
      "_DEBUG_INIT"
      "_MINSIZEREL"
      "_MINSIZEREL_INIT")
    message("XXX: ${lang}:${flag_suffix}: CMAKE_${lang}_FLAGS${flag_suffix}: ${CMAKE_${lang}_FLAGS${flag_suffix}}")
    string(REPLACE "/Zi" "/Z7"
      CMAKE_${lang}_FLAGS${flag_suffix}
      "${CMAKE_${lang}_FLAGS${flag_suffix}}")
  endforeach()
endforeach()
