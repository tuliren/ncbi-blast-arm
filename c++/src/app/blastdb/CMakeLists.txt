# $Id: CMakeLists.txt 661058 2023-01-04 14:29:49Z ivanov $

NCBI_add_app(
  blastdbcmd makeblastdb blastdb_aliastool blastdbcheck convert2blastmask
  blastdbcp makeprofiledb blastdb_convert blastdb_path makeclusterdb
)
