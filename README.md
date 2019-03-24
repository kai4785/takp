# Personal Collection of TAKP related code
This is my personal collection of scripts and code that I've slapped together since playing on the [The Al'Kabor Project](https://www.takproject.net/) Everquest server (aka TAKP).

## dps.py && dps/
DPS tracking software. I wrote the python one first, and then got tired of how slow it was. To be fair, I'm no python dev and certainly not versed in performance related python optimisations. I am, however, a C/C++ developer.

## launcher.sh
As a Linux guy, I got tired of running Windows just for my EQ fix. So this launcher is my 300 line shell script replacement for:
1. Play On Linux
  - To be fair, I am still downloading their versions of wine. I just couldn't hack toying with the UI every time I wanted to make a tweak.
2. HotKeyNet
  - The TAKP client comes with an old version of hotkeynet. It has a 3-client launcher mode, and keyboard shortcuts to swap between clients. I ported my config to xdotool and wmctrl.

It's still very "works for me". Pull Requests are welcome!

## skillup.sh
Just like launcher.sh, I wanted a way to properly do AFK skill-ups on the TAKP server. Does the same sort of xdotool and wmctrl stuff. Uses flock so you can run it against two or more running clients. The flock enforces that we follow the 'foreground' rules ensuring that keyboard strokes are only sent to clients in the foreground, one at a time.
