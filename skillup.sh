#!/bin/bash

window=$1
key=$2
casttime=$3
sittime=$4
castspersit=$5

window=$(xdotool search --name $window)
while true; do
    for num in $(seq 1 $castspersit); do
        echo "Cast $num/$castspersit"
        (
            flock 200
            xdotool windowactivate $window
            sleep .25
            xdotool key --clearmodifiers --delay 30 slash s t a n d Return
            xdotool key --delay 80 $key $key $key $key $key $key $key $key $key $key
            sleep .25
        ) 200>~/.takp.screen.lock
        echo "Waiting $casttime for spell"
        sleep $casttime
    done
    if [ $sittime -gt 0 ]; then
        (
            flock 200
            xdotool windowactivate $window
            sleep .25
            xdotool key --clearmodifiers --delay 30 slash s i t Return
            sleep .25
        ) 200>~/.takp.screen.lock
        echo "Medding for $sittime"
        sleep $sittime
    else
        sleep 1
    fi
done

# Pip 208
# Sam 185
