#! /bin/sh
# $Id: lmdb_test.sh 539943 2017-06-29 01:32:38Z ucko $

testdb="./testdb"

exit_code=0
rm -rf $testdb
mkdir $testdb

exec lmdb_test$1
