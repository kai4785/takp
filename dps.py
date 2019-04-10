#!/usr/bin/python3.6

'''
takp_dps.py

This script will parse and follow (see Python-Tail below) a TAKP client
generated log file, and generate Battle Reports.

Battle reports are printed to the screen 10 seconds (keep_alive) after battle
messages have stopped. All timestamps come from the log file, so if it's been
10 seconds, but you haven't seen an update, say something to your group.

Examples:

Change parse Meriadoc's file, change 'You' to Meriadoc in the log entries, and follow the file
takp_dps.py -l ~/.takp/drive_e/TAKP/eqlog_Meriadoc_loginse.txt --me Meriadoc -f

Show everything for Meriadoc that's happened today.
takp_dps.py -l ~/.takp/drive_e/TAKP/eqlog_Meriadoc_loginse.txt --history --me Meriadoc -s 00:00:00

History and follow can be used together:
takp_dps.py -l ~/.takp/drive_e/TAKP/eqlog_Meriadoc_loginse.txt --history --me Meriadoc -s 00:00:00 -f
'''


'''
Python-Tail - Unix tail follow implementation in Python. 

python-tail can be used to monitor changes to a file.

Example:
    import tail

    # Create a tail instance
    t = tail.Tail('file-to-be-followed')

    # Register a callback function to be called when a new line is found in the followed file. 
    # If no callback function is registerd, new lines would be printed to standard out.
    t.register_callback(callback_function)

    # Follow the file with 5 seconds as sleep time between iterations. 
    # If sleep time is not provided 1 second is used as the default time.
    t.follow(s=5) '''

# Author - Kasun Herath <kasunh01 at gmail.com>
# Source - https://github.com/kasun/python-tail

import os
import sys
import time
import re
from collections import defaultdict
from datetime import datetime, timedelta, date
from argparse import ArgumentParser as Argp
from contextlib import suppress

class Tail(object):
    ''' Represents a tail command. '''
    def __init__(self, tailed_file):
        ''' Initiate a Tail instance.
            Check for file validity, assigns callback function to standard out.
            
            Arguments:
                tailed_file - File to be followed. '''

        self.check_file_validity(tailed_file)
        self.tailed_file = tailed_file
        self.callback = sys.stdout.write

    def follow(self, s=1):
        ''' Do a tail follow. If a callback function is registered it is called with every new line. 
        Else printed to standard out.
    
        Arguments:
            s - Number of seconds to wait between each iteration; Defaults to 1. '''

        with open(self.tailed_file) as file_:
            # Go to the end of file
            file_.seek(0,2)
            while True:
                curr_position = file_.tell()
                line = file_.readline()
                if not line:
                    file_.seek(curr_position)
                    time.sleep(s)
                else:
                    self.callback(line)

    def register_callback(self, func):
        ''' Overrides default callback function to provided function. '''
        self.callback = func

    def check_file_validity(self, file_):
        ''' Check whether the a given file exists, readable and is a file '''
        if not os.access(file_, os.F_OK):
            raise TailError("File '%s' does not exist" % (file_))
        if not os.access(file_, os.R_OK):
            raise TailError("File '%s' not readable" % (file_))
        if os.path.isdir(file_):
            raise TailError("File '%s' is a directory" % (file_))

class TailError(Exception):
    def __init__(self, msg):
        self.message = msg
    def __str__(self):
        return self.message

class Damage(object):
    def __init__(self):
        self.damage = 0
        self.hits = 0

    def hit(self, damage):
        # NOTE: Killing blows are purposefully over-kill, and they don't help show you your dps. We should track them separately.
        if damage < 32000:
            self.damage += damage
            self.hits += 1


class Fight(object):
    def __init__(self, start):
        self.start = start
        self.end = self.start
        self._melee = Damage()
        self._cripp = Damage()
        self._crit = Damage()
        self._backstab = Damage()

    def melee(self, time, verb, damage):
        self.end = time
        self._melee.hit(damage)
        
    def crit(self, time, verb, damage):
        self._crit.hit(damage)

    def cripp(self, time, verb, damage):
        self._cripp.hit(damage)

    def backstab(self, time, verb, damage):
        self._backstab.hit(damage)

    @property
    def seconds(self):
        seconds = (self.end - self.start).total_seconds()
        if not seconds:
            seconds = 1
        return int(seconds)

    @property
    def damage(self):
        return self._melee.damage

    @property
    def hits(self):
        return self._melee.hits

    @property
    def dps(self):
        return self.damage / self.seconds

    @property
    def dpa(self):
        if self.hits == 0:
            return 0
        else:
            return self.damage / self.hits

    @property
    def hps(self):
        return self.hits / self.seconds

    def __str__(self):
        return f'{self.seconds}s, {self._melee.hits}({self.aps}/s) hits, {self._melee.damage}({self.dps}/s, {self.dpa}/a) damage'


class Battle(object):
    def __init__(self, time, keep_alive):
        self._melee = defaultdict(dict)
        self._deaths = defaultdict(dict)
        self._cripps = defaultdict(int)
        self._crits = defaultdict(int)
        self.start = time
        self._keep_alive = keep_alive
        self.keep_alive(self.start)

    def keep_alive(self, time, bump = 10):
        bump = timedelta(seconds=max(bump, self._keep_alive))
        self.end = time
        self.expire = self.end + bump

    def melee(self, time, attacker, target, verb, damage):
        '''TODO 
        Handle the case where you switch targets.
            With only a start and stop, damage done to target A before switching to target B causes @seconds to come back with large delays between hits when you're off hitting B.
        Handle the case where "Your target is too far away, get closer!"
            With only a start and stop, damage done to target A before it goes out of reach means you end up with large delays between attacks while A is out of range.
        '''
        self.keep_alive(time)
        #if verb != 'hit by non-melee':
            #attacker = f'{attacker}({verb})'
        if target not in self._melee[attacker]:
            self._melee[attacker][target] = Fight(time)

        if damage == self._cripps[attacker]:
            self._melee[attacker][target].cripp(time, verb, damage)
            self._cripps[attacker] = 0
        elif damage == self._crits[attacker]:
            self._melee[attacker][target].crit(time, verb, damage)
            self._crits[attacker] = 0
        elif verb == 'backstabs':
            self._melee[attacker][target].backstab(time, verb, damage)

        self._melee[attacker][target].melee(time, verb, damage)

    def cripp(self, attacker, damage):
        self._cripps[attacker] = damage

    def crit(self, attacker, damage):
        self._crits[attacker] = damage

    def magic(self, time, target, verb, damage):
        self.melee(time, f'Spell/DS({damage})', target, verb, damage)
        self.melee(time, f'Spell/DS(total)', target, verb, damage)

    @property
    def seconds(self):
        seconds = (self.end - self.start).total_seconds()
        if not seconds:
            seconds = 1
        return int(seconds)

    def death(self, target, slayer = ''):
        if target not in self._deaths:
            self._deaths[target] = dict()
        if slayer not in self._deaths[target]:
            self._deaths[target][slayer] = 0
        self._deaths[target][slayer] += 1

    def report(self, time):
        print(f'Battle report {self.seconds}s [{self.start} - {self.end}]:')
        print('Melee:')
        header_format = '{:<35s} {:<30s} {:>4s} {:>4s} {:>5s} {:>6s} {:>6s} {:>6s}'
        fight_format  = '{:<35s} {:<30s} {:>4d} {:>4d} {:>5.2f} {:>6d} {:>6.2f} {:>6.2f}'
        break_str = '-------------------------------------------------------------------------------------------------------'
        print(header_format.format('(N)PC', 'Target', 'Sec', 'Hits', 'h/s', 'Damage', 'd/h', 'd/s'))
        print(break_str)
        for _attacker, targets in self._melee.items():
            attacker = _attacker
            total_hits = 0
            total_damage = 0
            total_seconds = 0
            total_fights = 0
            for target, fight in targets.items():
                total_hits += fight._melee.hits
                total_damage += fight._melee.damage
                total_seconds += fight.seconds
                total_fights += 1
                print(fight_format.format(attacker, target, fight.seconds, fight._melee.hits, fight.hps, fight._melee.damage, fight.dpa, fight.dps))
                if fight._crit.hits > 0:
                    print(fight_format.format('', '+critical hits', 0, fight._crit.hits, 0, fight._crit.damage, fight._crit.damage / fight._crit.hits, fight._crit.damage / fight.seconds))
                if fight._cripp.hits > 0:
                    print(fight_format.format('', '+crippling blows', 0, fight._cripp.hits, 0, fight._cripp.damage, fight._cripp.damage / fight._cripp.hits, fight._cripp.damage / fight.seconds))
                if fight._backstab.hits > 0:
                    print(fight_format.format('', '+backstabs', 0, fight._backstab.hits, 0, fight._backstab.damage, fight._backstab.damage / fight._backstab.hits, fight._backstab.damage / fight.seconds))

                attacker = ''
            if total_fights > 1:
                total_hps = total_hits / self.seconds
                total_dpa = total_damage / total_hits
                total_dps = total_damage / self.seconds
                print(fight_format.format('', '+Total', self.seconds, total_hits, total_hps, total_damage, total_dpa, total_dps))
            print(break_str)
        print()
        print('Deaths:')
        print('{:<35s} {:<30s} {:<6s}'.format('Target', 'Slayer', 'Deaths'))
        print(break_str)
        total_times = 0
        for target, slayers in self._deaths.items():
            for slayer, times in slayers.items():
                print('{:<35s} {:<30s} {:<6d}'.format(target, slayer, times))
                print(break_str)
                total_times += times
        print('{:<35s} {:<30s} {:<6d}'.format("Total", "", total_times))
        print(break_str)
        print()
        sys.stdout.flush()


class Process(object):
    def __init__(self, pc_list, you, since, keep_alive):
        self.battle = None
        self.attacker = ''
        self.target = ''
        self.pc_list = pc_list
        self.you = you
        self.since = since
        self.pc_regexp = '|'.join(self.pc_list)
        self.melee_verbs = "smash|smashes|hit|slash|claw|claws|crush|pierce|kick|bash|maul|gore|gores|slice|slices|slashes|crushes|hits|punch|punches|kicks|bashes|bites|pierces|mauls|backstab|backstabs|rends"
        self.melee_reg = re.compile(fr'^({self.pc_regexp}) ({self.melee_verbs}) ({self.pc_regexp}) for ([0-9]+) points? of damage\.')
        self.cripp_re = re.compile(fr'^({self.pc_regexp}) lands a Crippling Blow\!\(([0-9]+)\)')
        self.crit_re = re.compile(fr'^({self.pc_regexp}) Scores a critical hit\!\(([0-9]+)\)')
        self.magic_re = re.compile(fr'({self.pc_regexp}) was (hit by non-melee) for ([0-9]+) points? of damage\.')
        self.death_re1 = re.compile(fr'({self.pc_regexp}) have slain ({self.pc_regexp})!')
        self.death_re2 = re.compile(fr'({self.pc_regexp}) (has|have) been slain by ({self.pc_regexp})!')
        self.death_re3 = re.compile(fr'({self.pc_regexp}) died\.')
        self.heal_re = re.compile(f'(You) have been (healed) for ([0-9]+) points? of damage\.')
        self.keep_alive = keep_alive

    def __call__(self, line):
        '''
        Melee messages are:
            'attacker' 'verb' 'target' for 'xx' points? of damage.

        Damage Shield and Proc messages are:
            'target' was hit by non-melee for 'xx' points? of damage.
        '''
        # Skip blank or mal-formed lines
        try:
            time = datetime.strptime(line[1:25], '%a %b %d %H:%M:%S %Y')
        except:
            return
        
        if time < since:
            return

        msg = line[27:-1]

        # End the battle
        if self.battle:
            end_the_battle = False
            #if msg == 'LOADING, PLEASE WAIT...':
                #end_the_battle = True
            if time > self.battle.expire:
                end_the_battle = True
            if end_the_battle:
                self.battle.report(time)
                self.battle = None

        magic = self.magic_re.search(msg)
        if magic:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            target, verb, damage = magic.group(1, 2, 3)
            self.battle.magic(time, target, verb, int(damage))
            return
                
        melee = self.melee_reg.search(msg)
        if melee:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            attacker, verb, target, damage = melee.group(1, 2, 3, 4)
            if attacker == 'You':
                attacker = self.you
            if target == 'YOU':
                target = self.you
            self.battle.melee(time, attacker, target, verb, int(damage))
            return

        cripp = self.cripp_re.search(msg)
        if cripp:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            attacker, damage = cripp.group(1, 2)
            self.battle.cripp(attacker, int(damage))

        crit = self.crit_re.search(msg)
        if crit:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            attacker, damage = crit.group(1, 2)
            self.battle.crit(attacker, int(damage))

        heal = self.heal_re.search(msg)
        if heal:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            target, verb, damage = heal.group(1, 2, 3)
            if target == 'You':
                target = self.you
            self.battle.melee(time, 'Heals', target, verb, int(damage))

        death = self.death_re1.search(msg)
        if death:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            slayer, target = death.group(1, 2)
            if slayer == 'You':
                slayer = self.you
            if target == 'You':
                target = self.you
            self.battle.death(target, slayer)
            return

        death = self.death_re2.search(msg)
        if death:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            target, slayer = death.group(1, 3)
            if slayer == 'You':
                slayer = self.you
            if target == 'You':
                target = self.you
            self.battle.death(target, slayer)
            return

        death = self.death_re3.search(msg)
        if death:
            if not self.battle:
                self.battle = Battle(time, self.keep_alive)
            target = death.group(1)
            if target == 'You':
                target = self.you
            self.battle.death(target)
            return

argp = Argp(description='Process TAKP logs')

argp.add_argument('--me', '-m', help='Who is "You" in the logs', default='Me')
argp.add_argument('--pc', '-p', action='append', help='Filter to specific (Non-)Player Characters to search for in the logs')
argp.add_argument('--log', '-l', help='Logfile to watch')
argp.add_argument('--history', help='Read the whole log history', action='store_true')
argp.add_argument('--follow', '-f', help='Follow the log file', action='store_true')
argp.add_argument('--since', '-s', help='Parse logs since', default='Thu Jan 01 00:00:00 1970')
argp.add_argument('--keep-alive', '-k', help='Keep alive seconds for each Battle', default=10)

args = argp.parse_args()
if args.pc:
    pc_list = args.pc + ['[Yy][Oo][Uu]']
else:
    pc_list = ['[0-9A-Za-z `]+']
since = None
today = date.today()
with suppress(ValueError):
    since = datetime.strptime(args.since, '%a %b %d %H:%M:%S %Y')
with suppress(ValueError):
    since = datetime.strptime(args.since, '%b %d %H:%M:%S %Y')
with suppress(ValueError):
    since = datetime.strptime(args.since, '%b %d %H:%M:%S')
    since = since.replace(year=today.year)
with suppress(ValueError):
    since = datetime.strptime(args.since, '%b %d')
    since = since.replace(year=today.year)
with suppress(ValueError):
    since = datetime.strptime(args.since, '%H:%M:%S')
    since = since.replace(year=today.year, month=today.month, day=today.day)

if not since:
    raise Exception(f'Unable to parse date string [{args.since}]')

process = Process(pc_list = pc_list, you = args.me, since=args.since, keep_alive = int(args.keep_alive))
if args.history:
    with open(args.log, "r") as fd:
        for line in fd.readlines():
            process(line)

if args.follow:
    t = Tail(args.log)
    t.register_callback(process)
    t.follow()
