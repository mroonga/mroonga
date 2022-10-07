# ccache doesn't work with /Zi.
# See also: https://github.com/ccache/ccache/issues/1040
string(REPLACE "/Zi" "/Z7"
  CMAKE_C_FLAGS_RELWITHDEBINFO
  "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
string(REPLACE "/Zi" "/Z7"
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
