gcc -Wall -Wmissing-prototypes -Wpointer-arith -Wdeclaration-after-statement -Wendif-labels -Wmissing-format-attribute -Wformat-security -fno-strict-aliasing -fwrapv -Wno-unused-command-line-argument -g -O0  -L../../../../src/port -L../../../../src/common -lpq -Wl,-dead_strip_dylibs   -bundle -bundle_loader ../../../../src/backend/postgres -o metaworker.so metaworker.o
/bin/sh ../../../../config/install-sh -c -d '/Users/YOuth/pgbuild/lib/postgresql'
/usr/bin/install -c -m 755  metaworker.so '/Users/YOuth/pgbuild/lib/postgresql/'
