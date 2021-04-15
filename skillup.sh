#!/bin/bash

window=$1
key=$2
casttime=$3
sittime=0
[ -n "$4" ] && sittime=$4
castspersit=$5

eval $(xdotool getmouselocation --shell)
x_mouse=$X
y_mouse=$Y
_sleep()
{
    eval $(xdotool getmouselocation --shell)
    #if [ $X -ne $x_mouse ] || [ $Y -ne $y_mouse ]; then
    #    echo "Something moved the mouse, so I'm done."
    #    exit 1
    #fi
    x_mouse=$X
    y_mouse=$Y
    sleep $1
}

#key=0
window=$(xdotool search --name $window)
while true; do
    date
    for num in $(seq 1 $castspersit); do
        #key=$(( (key++ % 3) + 1))
        echo "Cast [$key] $num/$castspersit"
        (
            flock 200
            xdotool windowactivate $window
            _sleep .5
            #xdotool key --clearmodifiers --delay 30 slash s t a n d Return
            #xdotool key --clearmodifiers Tab
            #_sleep .25
            #xdotool key --clearmodifiers F1
            #xdotool key --clearmodifiers --delay 30 slash p e t space t a r g e t Return
            #_sleep .25
            xdotool key --clearmodifiers --delay 80 $key $key $key $key $key $key $key $key $key $key
            _sleep .25
        ) 200>~/.takp.screen.lock
        echo "Waiting $casttime for spell"
        _sleep $casttime
    done
    if [ "$sittime" -gt 0 ]; then
        (
            flock 200
            xdotool windowactivate $window
            _sleep .5
            xdotool key --clearmodifiers F1
            _sleep .25
            #xdotool key --clearmodifiers --delay 80 7 7 7 7 7 7 7 7
            #_sleep 5
            #xdotool key --clearmodifiers --delay 80 5 5 5 5 5 5 5 5
            #_sleep 3
            xdotool key --clearmodifiers --delay 30 slash s i t Return
            _sleep .25
        ) 200>~/.takp.screen.lock
        echo "Medding for $sittime"
        _sleep $sittime
        (
            flock 200
            xdotool windowactivate $window
            _sleep .5
            xdotool key --clearmodifiers --delay 30 slash s t a n d Return
            _sleep .25
        ) 200>~/.takp.screen.lock
    else
        _sleep 1
    fi
done

# Pip 208
# Sam 185
