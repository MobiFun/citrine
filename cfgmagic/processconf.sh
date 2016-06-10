#!/bin/sh
# The top level Makefile invokes this Bourne shell script after ensuring
# that the build.conf file is present.  The job of this script is to grok
# that configuration file and to produce include/config.{h,mk,m4} files
# corresponding to the selected configuration.
#
# The current directory is expected to be the top level of gsm-fw, i.e.,
# all fragments are sourced as cfgmagic/blah.
# Don't run this script directly - let the Makefile do it for you.

set -e
. cfgmagic/functions
TARGET=
c_export_list=
mk_export_list=
m4_export_list=
target_feature_list=

# some defaults
FFS_IN_RAM=1
RVTMUX_UART_port=IrDA
RVTMUX_UART_baud=115200
FLASH_BOOT_VIA_BOOTROM=1
FLASHIMAGE_BASE_ADDR=0x2000
DWNLD=1
L1_DYN_DSP_DWNLD=1

# We always export CONFIG_INCLUDE_xxx to config.h and config.mk, whether
# enabled or disabled.  This way enabling any of these components is
# as simple as CONFIG_INCLUDE_xxx=1, without having to remember the
# export_to_{c,mk} magic.
CONFIG_INCLUDE_GPF=0
export_to_c	CONFIG_INCLUDE_GPF
export_to_mk	CONFIG_INCLUDE_GPF
CONFIG_INCLUDE_L1=0
export_to_c	CONFIG_INCLUDE_L1
export_to_mk	CONFIG_INCLUDE_L1
CONFIG_INCLUDE_PCM=0
export_to_c	CONFIG_INCLUDE_PCM
export_to_mk	CONFIG_INCLUDE_PCM
CONFIG_INCLUDE_PS=0
export_to_c	CONFIG_INCLUDE_PS
export_to_mk	CONFIG_INCLUDE_PS
CONFIG_INCLUDE_SIM=0
export_to_c	CONFIG_INCLUDE_SIM
export_to_mk	CONFIG_INCLUDE_SIM

. ./build.conf

if [ -z "$TARGET" ]
then
	echo "Error: target not set in build.conf" 1>&2
	exit 1
fi

# Serial configuration
case "$RVTMUX_UART_port" in
	IrDA)
		CONFIG_LLDBG_UART_BASE=0xFFFF5000
		;;
	MODEM)
		CONFIG_LLDBG_UART_BASE=0xFFFF5800
		CONFIG_RVTMUX_ON_MODEM=1
		export_to_c CONFIG_RVTMUX_ON_MODEM
		;;
	*)
		echo "Error: unknown RVTMUX_UART_port=$RVTMUX_UART_port" 1>&2
		exit 1
		;;
esac

TR_BAUD_CONFIG=TR_BAUD_$RVTMUX_UART_baud
export_to_c TR_BAUD_CONFIG

# FFS in RAM
export_to_c	FFS_IN_RAM
export_to_m4	FFS_IN_RAM
if [ $FFS_IN_RAM = 1 ]
then
	if [ -z "$RAMFFS_BLKSIZE_LOG2" -o -z "$RAMFFS_NBLOCKS" ]
	then
		echo \
	"Error: RAMFFS_BLKSIZE_LOG2 and RAMFFS_NBLOCKS need to be defined" 1>&2
		exit 1
	fi
	export_to_c	RAMFFS_BLKSIZE_LOG2 RAMFFS_NBLOCKS
	export_to_m4	RAMFFS_BLKSIZE_LOG2 RAMFFS_NBLOCKS
fi

# Sane vs. Compal target differences for flash boot
export_to_c	FLASH_BOOT_VIA_BOOTROM
export_to_m4	FLASH_BOOT_VIA_BOOTROM FLASHIMAGE_BASE_ADDR

# L1 configuration
if [ "$CONFIG_GSM" = 1 -a "$CONFIG_L1_STANDALONE" = 1 ]
then
	echo "Error: feature gsm and feature l1stand are mutually exclusive" \
		1>&2
	exit 1
fi
if [ "$CONFIG_INCLUDE_L1" = 1 ]
then
	export_to_c	L1_DYN_DSP_DWNLD DWNLD
	export_to_mk	L1_DYN_DSP_DWNLD
fi

# The list of build components: we have some invariants that are always
# included, and some others that are included depending on the configuration.

BUILD_COMPONENTS="bsp libiram nucleus riviera serial services sprintf sysglue"

if [ "$CONFIG_INCLUDE_GPF" = 1 ]
then
	BUILD_COMPONENTS="$BUILD_COMPONENTS gpf"
fi
if [ "$CONFIG_INCLUDE_L1" = 1 ]
then
	BUILD_COMPONENTS="$BUILD_COMPONENTS L1"
fi
if [ "$CONFIG_INCLUDE_PS" = 1 ]
then
	BUILD_COMPONENTS="$BUILD_COMPONENTS ccd comlib"
	BUILD_COMPONENTS="$BUILD_COMPONENTS g23m-aci g23m-glue g23m-gsm"
fi
if [ "$CONFIG_LLDBG" = 1 ]
then
	BUILD_COMPONENTS="$BUILD_COMPONENTS lldbg"
fi

export_to_mk BUILD_COMPONENTS

# The default build image type depends on the target and features
if [ -f "cfgmagic/defimage.$TARGET" ]
then
	. "cfgmagic/defimage.$TARGET"
else
	BUILD_DEFAULT_IMAGE=ramImage
fi

export_to_mk BUILD_DEFAULT_IMAGE

# Now generate the output files!

parse_export_list() {
	for var in $1
	do
		eval "value=\"\$$var\""
		emit_def "$var" "$value"
	done
}

# make config.h
emit_def() {
	echo "#define	$1	$2" >> include/config.h
}
: > include/config.h
parse_export_list "$c_export_list"

# make config.mk
emit_def() {
	echo "$1=	$2" >> include/config.mk
}
: > include/config.mk
parse_export_list "$mk_export_list"

# make config.m4
emit_def() {
	echo 'define(`'"$1'"',`'"$2')dnl" >> include/config.m4
}
: > include/config.m4
parse_export_list "$m4_export_list"
