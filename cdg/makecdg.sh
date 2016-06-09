#!/bin/sh

set -e

if [ $# != 1 ]
then
	echo "Usage: %s config-name" 1>&2
	exit 1
fi

if [ ! -f "fflags-$1.h" ]
then
	echo "error: specified configuration is not known" 1>&2
	exit 1
fi

rm -rf tempout
mkdir tempout

# nowhine is a wrapper around wine that suppresses some obnoxious whine,
# see leo2moko.  TZ=GMT avoids some pesky time zone issues.
# The arguments to ccdgen have been taken from TCS211 pdt_*.mak makefiles.

TZ=GMT nowhine ccdgen.exe -h -m512 -a2 -ifflags-$1.h -otempout -Rpdf-mdf-list

echo "Converting from CRLF to UNIX line endings"

rm -rf cdginc-$1
mkdir cdginc-$1

# fromdos comes with Slackware, dunno about other distros
for i in `cat gen-file-list`
do
	fromdos < tempout/$i > cdginc-$1/$i
done
