# Everquest (TAKP) dps parser

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

## Musing about this project in Python vs C
This was a fun learn-to-code project. This project supports the narrative that python (among other interpreted languages) is "quicker to write but slower to run" than C. I think I had a fully funtional python version in a few hack sessions, maybe 10 hours. The C version probably has a good 40 hours into it over 3 weeks, including the integration of travis-ci. However, the C code tends to run somewhere around 10-30 times faster. For instance, a log file with about a million lines can take the python script about 10 seconds to parse, where the C code is less than 1 second.

I chose to write the code at a "Hurt me plenty." difficulty, so I didn't alias away all the `struct`s and tried to avoid macros as much as possible. If it is worth running, it's worth writing. I also focused on minimizing memory allocations, relying on the stack as much as possible.

I also purposefully chose to emulate C++ class type behaviors with Constructor/Destructor semantics, rather than init/fini or alloc/free. It was kinda neat to implement a dirty hacked up replacement for C++ "vtables". The `struct Array` structure is not too far away from a Macro version of a Template. Something like `#define DECLARE_STRUCT_ARRAY(type)`. But I'm a C guy, and I'm comfortable with `void *`.
