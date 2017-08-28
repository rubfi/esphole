#!/bin/sh
# Script for downloading and converting the ad server lists to smaller files

HOSTS_URL="https://pgl.yoyo.org/adservers/serverlist.php?hostformat=hosts&mimetype=plaintext&useip=0.0.0.0"
#HOSTS_URL="http://someonewhocares.org/hosts/zero/hosts"
HOSTS_TEMP="/tmp/host_list"

wget $HOSTS_URL -O - | grep -v '#' | grep "0.0.0.0" | sed 's/0\.0\.0\.0 //g' > $HOSTS_TEMP

# saves each domain in a file depending on its characters
for i in $(cat $HOSTS_TEMP)
do
    x=$(echo -n $i | tr '\n' ' ' | tr "\r" " ")
    echo -n ,$x >> hosts_$(echo -n "$x" | wc -m)
done

# adds a kind of EOF for making easier the findUntil
for i in $(ls -1 hosts_*)
do
    echo ",@@@" >>$i
done

mkdir -p ../data
mv hosts_* ../data/
