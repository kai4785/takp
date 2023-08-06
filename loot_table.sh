#!/bin/bash

find_mob()
{
    mob="$1"
    mysql -u eqemu -peqemu peqmac -s <<_EOF |
SELECT loottable_id, name FROM npc_types WHERE name LIKE '$mob';
_EOF
    while read -r row; do
        set $row
        loottable_id=$1
        shift
        echo Found $@
        loottable $loottable_id
    done
}

loottable()
{
    local loottable_id=$1
    mysql -u eqemu -peqemu peqmac -s <<_EOF |
SELECT * from loottable_entries where loottable_id=${loottable_id};
_EOF
    while read -r row; do
        #echo $row;
        set $row
        local multiplier=$3
        local probability=$4
        local droplimit=$5
        local mindrop=$6
        lootdrop $2
    done
}

lootdrop()
{
    local lootdrop_id=$1
    mysql -u eqemu -peqemu peqmac -s <<_EOF |
SELECT name FROM lootdrop
WHERE lootdrop.id=${lootdrop_id};
_EOF
    while read -r row; do
        echo "  $row"
    done
    echo "  ${multiplier}x ${probability}% [${mindrop},${droplimit}]"
    mysql -u eqemu -peqemu peqmac --table <<_EOF |
SELECT lootdrop_entries.chance, lootdrop_entries.multiplier, items.name
FROM lootdrop_entries
JOIN items ON lootdrop_entries.item_id = items.id
WHERE lootdrop_id=${lootdrop_id};
_EOF
    while read -r row; do
        echo "    $row"
    done
    total_chance=$(mysql -u eqemu -peqemu peqmac -s <<_EOF
SELECT SUM(chance) from lootdrop_entries where lootdrop_id=${lootdrop_id}
_EOF
)
    echo "    Chance Sum: ${total_chance}%"
}

find_mob $1
