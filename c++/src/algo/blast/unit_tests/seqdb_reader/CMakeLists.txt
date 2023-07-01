# $Id: CMakeLists.txt 621774 2020-12-16 19:29:59Z ivanov $

NCBI_requires(Boost.Test.Included)
NCBI_add_app(seqdb_unit_test seqdb_lmdb_unit_test)

