DESCRIPTION = "PCIE driver"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://readme.txt;md5=4e618209c6691923978c3f8561a35e62 "

inherit module

PR = "r0"
PV = "0.1"

SRC_URI = "file://Makefile \
	   file://readme.txt \
	   file://uiogen2.c"
S = "${WORKDIR}"

