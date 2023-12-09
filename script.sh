#!/bin/bash
 
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <c>"
    exit 1
fi
 
character=$1
count=0

regex='^[A-Z][A-Za-z0-9 ,.!?]*[.!?]$'
regex_exclusiv=',si'

while IFS= read -r line; do
        if [[ $line =~ $regex ]]
            then
                ((count++))
        if [[ $line =~ $regex_exclusiv ]]
            then
                ((count--))
        fi
    fi
done 
 
echo "Au fost identificate in total $count propozitii corecte care contin caracterul $character"
