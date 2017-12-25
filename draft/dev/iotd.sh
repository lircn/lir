#!/bin/sh
#
# Copyright (c) , TENCENT TECHNOLOGIES CO., LTD.
# Author: zaynli <zaynli@tencent.com>

SRC_PATH="/home/ftp1/upload"
TMP_PATH="/tmp/upload_tmp"
PKG_PWD="tencent"
POST_FILE="iot_data"
BASE_URL="http://113.105.141.27/"

# treat this as usr and grp
FTP_USR="ftp1"


pack_file()
{
	if [ `ls $SRC_PATH | wc -l` -gt 0 ]; then
		cd $SRC_PATH
		tar -zcvf - ./*|openssl des3 -salt -k $PKG_PWD|dd of=$TMP_PATH/$POST_FILE
		rm -rf ./*
		true
		return
	fi

	false
}

unpack_file()
{
	file=$1
	if [ -s $file ]; then
		dd if=$file|openssl des3 -d -k $PKG_PWD|tar -zxf -
		true
		return
	fi

	false
}

upload_file()
{
	curl -F "file=@${TMP_PATH}/${POST_FILE}" "${BASE_URL}/iot/cgn_upload"
}

check_env()
{
	if [ ! -d $TMP_PATH ]; then
		mkdir -p $TMP_PATH
	fi

	if [ ! -d $SRC_PATH ]; then
		mkdir -p $SRC_PATH
		chown $FTP_USR:$FTP_USR $SRC_PATH
	fi
}

run()
{
	check_env

	while true; do
		pack_file
		upload_file
		sleep 30
	done
}

case $1 in
	"run")
		run
		;;
	"unpack")
		unpack_file $2
		;;
esac
