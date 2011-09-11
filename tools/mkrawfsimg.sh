#!/bin/bash

INDEX_BLOCKS=32
DATA_FILE_BLOCKS=131072

function write_byte()
{
	value=$1

	a=`expr $1 / 64`
	b=`expr $1 / 8 % 8`
	c=`expr $1 % 8`

	echo -n -e "\\0$a$b$c"
}

function write_int32()
{
	value=$1

	write_byte `expr $1 % 256`
	write_byte `expr $1 / 256 % 256`
	write_byte `expr $1 / 65536 % 256`
	write_byte `expr $1 / 16777216 % 256`
}

function write_entry()
{
	filename=$1
	address=$2
	length=$3
	capacity=$4

	echo -n $filename
	
	dd if=/dev/zero bs=`expr 52 - ${#filename}` count=1 status=noxfer 2>/dev/null

	write_int32 $address
	write_int32 $length
	write_int32 $capacity
}

function get_file_stats()
{
	filename=$1

	size=`stat $filename --printf='%s'`
	blocks=`expr '(' $size '+' 511 ')' / 512`
	if [ $blocks -eq 0 ]; then
		blocks=1
	fi
	is_data_file=0
	if expr $filename : ^data/ > /dev/null; then
		blocks=$DATA_FILE_BLOCKS
		is_data_file=1
	fi
	capacity=`expr $blocks '*' 512`
}

if [ $# -lt 1 ]; then
	echo 'Usage:' $0 '<rootfs contents directory>'
fi

if [ ! -d $1 ]; then
	echo 'Error: directory not found'
	exit 1
fi

cd $1

index=`find -type f | sed 's/^[.][/]//;s/^/0/;s/^0data\//1data\//' | sort | sed s/^.//`

n_entries=`echo "$index" | wc -l`

data_base=`expr 512 '*' $INDEX_BLOCKS`
address=$data_base
echo "$index" | while read filename; do
	get_file_stats $filename
	write_entry $filename $address $size $capacity

	address=`expr $address + $capacity`
done

dd if=/dev/zero bs=`expr $data_base '-' '(' $n_entries '*' 64 ')'` count=1 status=noxfer 2>/dev/null

echo "$index" | while read filename; do
	get_file_stats $filename

	if [ $is_data_file -eq 0 ]; then
		dd if=$filename bs=$size count=1 status=noxfer 2>/dev/null
		dd if=/dev/zero bs=`expr $capacity '-' $size` count=1 status=noxfer 2>/dev/null
	fi
done

