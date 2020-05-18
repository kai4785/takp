[![Build Status](https://travis-ci.com/kai4785/takp.svg?branch=master)](https://travis-ci.com/kai4785/takp)
# Everquest (TAKP) dps parser in C++
## Name
dps - parses takp log files

## Synopsis
```
dps [--log file] [--me name] [--history] [--follow] [--since date] [--keepalive seconds] [--verbosity level] [--help]
```
## Description
Cross-platform dps parser. Works with all types of newlines (CR, LF, CRLF), on all three major OS platforms (Windows, Mac, Linux), and runs fast!

## Options
`--log file`

A path to a TAKP generated log file

`--me name`

Log files generally put "You" or "YOU" for the player character's own logs. By passing `--me`, you can specify who "You" are, so it prints the player character name, rather than "You".

`--follow`

Parse new lines as they are added to the file.

`--history`

Parse the entire history of the file

`--since date`

Skip to the specified date in the file. Use in combination with `--history`.

`--keepalive seconds`

Because the logs give no indication that a battle begins or ends, we start each battle with the first battle action, like "Soandso hits You for 10 points of damage.". We end the battle when a log entry has a date stamp that is greater than `keepalive` seconds since the last time we saw a battle action. 

`--verbosity level`

`--follow` and `--history` can be used together.

## Output

Given this log file:

```
[Tue Mar 16 04:00:00 1999] You hit So and so for 1 point of damage.
[Tue Mar 16 04:00:00 1999] So and so hits YOU for 1 point of damage.
[Tue Mar 16 04:00:11 1999] The end
```

The output looks something like this

```
$ ./dps --history --log tiny.log
Battle report! 1s [14400 : 14400]
(N)PC                               Target                          Sec Hits   h/s Damage    d/h    d/s
-------------------------------------------------------------------------------------------------------
You                                 So and so                         1    1  1.00      1   1.00   1.00
-------------------------------------------------------------------------------------------------------
So and so                           You                               1    1  1.00      1   1.00   1.00
-------------------------------------------------------------------------------------------------------
Total Heals: 0
```

## Musing about this project
This is a fun learn-to-code project. I originally wrote this program in Python, and then again in C. This time I'm writing it in C++17 with asio support. I really have no idea what I'm doing, except trying to add asio to a project I'm already familiar with that I think might gain from it.