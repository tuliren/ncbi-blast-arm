# $Id: CMakeLists.txt 621735 2020-12-16 15:47:41Z ivanov $

NCBI_project_tags(test)
NCBI_requires(Boost.Test.Included)
NCBI_add_app(blast_services_unit_test)

