# $Id: CMakeLists.txt 622501 2020-12-24 15:12:25Z ivanov $

NCBI_add_library(blast_app_util)
NCBI_add_app(
  blastp
  blastn
  blastx
  tblastn
  tblastx
  psiblast
  rpsblast
  rpstblastn
  blast_formatter
  blast_report
  deltablast
  seedtop
)

NCBI_project_watchers(camacho madden fongah2)
