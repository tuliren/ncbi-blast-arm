# $Id: CMakeLists.txt 621735 2020-12-16 15:47:41Z ivanov $

NCBI_add_subdirectory(
  unit_test_util readers blast lds2 data_loaders simple
  alnmgr cddalignview test manip cleanup format edit validator
  asniotest align seqmasks_io eutils
  align_format snputil uudutil variation writers pubseq_gateway
  logging import flatfile
)
