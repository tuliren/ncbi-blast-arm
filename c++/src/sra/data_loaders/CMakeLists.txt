# $Id: CMakeLists.txt 621843 2020-12-17 14:02:27Z ivanov $

NCBI_set_test_resources(ServiceMapper)
NCBI_add_subdirectory(
  bam sra csra wgs vdbgraph snp
)
