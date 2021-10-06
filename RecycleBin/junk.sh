###############################################################################
# Author: Kaiqi Chee
# Date: 02/16/2021
# Pledge: I pledge my honor that I have abided by the Stevens Honor System.
# Description: Creates a junk file with difference flags.
###############################################################################
#!/bin/bash

help_flag=0
list_flag=0
purge_flag=0

readonly JUNK=~/.junk

usage_message(){
cat <<ENDOFTEXT 
Usage: $(basename "$0") [-hlp] [list of files]
   -h: Display help.
   -l: List junked files.	
   -p: Purge all files.
   [list of files] with no other arguments to junk those files.
ENDOFTEXT
}	   
		   
while getopts ":hlp" option; do
	case $option in
		h) help_flag=1
		   ;;
		l) list_flag=1
		   ;;
		p) purge_flag=1
		   ;;
		?) printf "Error: Unknown option '-%s'.\n" $OPTARG >&2
		   usage_message
		   exit 1
		   ;;
		 esac
done

declare -a filename
shift "$((OPTIND-1))"

index=0
for f in $@; do
	filename[$index]="$f"
	((index++))
done

if [ ! -d "/home/user/.junk" ]; then
	mkdir $JUNK
fi

if [ $((help_flag+list_flag+purge_flag+index)) -gt 1 ]; then
	printf "Error: Too many options enabled.\n"
	usage_message
	exit 1
fi

if [ $help_flag -eq 1 ];then
	usage_message
	exit 0
fi

if [ $list_flag -eq 1 ];then
	ls -lAF $JUNK
	exit 0
fi

if [ $purge_flag -eq 1 ];then
	rm -rf $JUNK/*
	rm -rf $JUNK/.* 2> /dev/null
	exit 0
fi

if [ $index -eq 1 ];then
	mv $filename $JUNK
	exit 0
fi

if [ $((help_flag+list_flag+purge_flag+index)) -eq 0 ]; then
	usage_message
	exit 0
fi


	
