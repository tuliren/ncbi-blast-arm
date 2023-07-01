# $Id: CMakeLists.txt 621843 2020-12-17 14:02:27Z ivanov $

NCBI_requires(VDB)

NCBI_add_subdirectory(
  readers 
  data_loaders
  app 
)

