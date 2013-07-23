#!/bin/sh

AUTOGEN=`which gnome-autogen.sh`

if [ ! -e "${AUTOGEN}" ]; then
	echo "Please install gnome-common."
	exit 1
fi

$AUTOGEN $@
