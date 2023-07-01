# $Id: CMakeLists.txt 640800 2021-11-18 14:24:05Z stakhovv $

NCBI_add_subdirectory(
  clog 
  grid_cgi 
  xmlwrapp 
  eutils_client 
  hydra_client 
  discrepancy
  xmlreaders 
  hgvs 
  netstorage 
  jsonwrapp 
  biosample_util 
  data_loaders_util
  lapackwrapp 
  cgi_redirect 
  pmcidconv_client 
  third_party 
  third_party_static
)

