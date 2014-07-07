#!/bin/sh


#******************************************************
# This script builds all the trace simulators and runs 
# all the traces through them.
#*****************************************************

bm=$1
Ncyc=$2
Ncolor=$3
l1=$4
l2=$5

trace_dir="../traces"

mkdir -p logs
mkdir -p cfg
mkdir -p ctags
mkdir -p results
mkdir -p rules 
mkdir -p tags
mkdir -p l2misses
mkdir -p m-vectors

if [ ! -f "simulate" ]
then
	make
fi

taint_file=${trace_dir}/${bm}/init_taints.libraries.gz

echo on host `hostname` >> logs/${bm}.${l1}.${l2}
cmd="./simulate ${trace_dir} ${bm} ${taint_file} ${Ncyc} ${Ncolor} ${l1} ${l2}"
echo running $cmd >> logs/${bm}.${l1}.${l2}

$cmd
