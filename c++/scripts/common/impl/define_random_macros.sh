#!/bin/sh
# $Id: define_random_macros.sh 518561 2016-11-04 15:01:06Z ucko $

cat <<EOF
#define NCBI_RANDOM_VALUE_TYPE Uint4
#define NCBI_RANDOM_VALUE_MIN  0
#define NCBI_RANDOM_VALUE_MAX  0xffffffffu
EOF

for n in 0 1 2 3 4 5 6 7 8 9; do
   $SHELL -c 'echo $RANDOM $RANDOM $RANDOM; ps $$' | cksum | \
       awk "{ print \"#define NCBI_RANDOM_VALUE_$n    \" \$1 \"u\" }"
done
