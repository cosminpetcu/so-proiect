#!/bin/bash

if test "$#" -ne 1
then
    echo "Numar invalid de argumente"
    exit 1
fi

char="$1"
counter=0
regg1="^[A-Z][a-zA-Z0-9, ]*[\.$\!$\?$]"
regg2=",[]*si[]"

while read linie
do
    if echo "$linie" | grep -E "$regg1" | grep -v -E "$regg2" | grep -qF "$char"
    then
        ((counter++))
    fi
done

echo $counter