# $Id: CMakeLists.txt 621774 2020-12-16 19:29:59Z ivanov $

NCBI_project_tags(test)
NCBI_set_test_resources(ServiceMapper)
NCBI_add_subdirectory(blast_format blastdb seqdb_reader api)

