#!/bin/sh
# Shell script for building gsm-fw release binaries

if [ $# != 1 ]
then
	echo "usage: ./buildrel.sh buildlist" 1>&2
	exit 1
fi

if [ ! -f "configs/buildlists/$1" ]
then
	echo "Error: no buildlist named $1" 1>&2
	exit 1
fi

# get to work
set -e
mkdir -p images
exec < "configs/buildlists/$1"
while read config imglist
do

	if [ -z "$config" -o -z "$imglist" ]
	then
		echo "Error: bad line in configs/buildlists/$1" 1>&2
		exit 1
	fi

	echo "Building $config configuration"
	cp configs/$config build.conf
	make clean

	for img in $imglist
	do
		case "$img" in
		flashImage)
			format=bin
			;;
		ramImage)
			format=srec
			;;
		*)
			echo \
		"Error: invalid image type $img in configs/buildlists/$1" 1>&2
			exit 1
			;;
		esac

		echo "Building $img"
		make $img
		cp -p finlink/$img.$format images/$config-$img.$format
		cp -p finlink/$img.elf images/$config-$img.elf
	done

done
