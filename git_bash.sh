#!/bin/sh
commit_ts=`git log -1 --format="%ct"`
commit_time=`date -d@$commit_ts +"%Y-%m-%d %H:%M:%S"`
current_time=`date +"%Y-%m-%d %H:%M:%S"`
git_version=`git log -1 --format="%h"`
git_tag=`git tag`
touch version.h.tmp
#echo MYVERSION > version.h.tmp
sed  s/MYVERSION/"version: $git_tag commit_id: $git_version commit_date: $commit_time build_date: $current_time"/g version.h.tmp > version.h
#make clean
#make