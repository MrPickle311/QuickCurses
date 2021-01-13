file(REMOVE_RECURSE
  "libALL_LIBRARIES.a"
  "libALL_LIBRARIES.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/ALL_LIBRARIES.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
