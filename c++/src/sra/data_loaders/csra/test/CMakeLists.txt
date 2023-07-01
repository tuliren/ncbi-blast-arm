# $Id: CMakeLists.txt 621843 2020-12-17 14:02:27Z ivanov $

NCBI_set_test_requires(in-house-resources)
NCBI_add_app(test_csra_loader test_csra_loader_mt test_sra)
