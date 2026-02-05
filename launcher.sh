#!/bin/bash

set -e

# Config file for usernames and passwords
takp_config=$HOME/.takp_launcher.conf
# variables the config file is expected to override.
accounts=()
default_password=""
default_suffix=""
if [ ! -e ${takp_config} ]; then
    cat << _EOF > "${takp_config}"
accounts=("user2" "user3" "user4")
default_password="password"
default_suffix=""
account_user1_password="differentpassword"
account_user1_suffix=".differentfolder"
_EOF
    echo "Unable to find config file, so I made one for you. Edit '${takp_config}' to set up your usernames and passwords." >&2
    echo "Press Enter to continue"
    read nothing
else
    . ${takp_config}
fi

# Glorious Eggroll's GE-Proton provides the best out-of-the-box wine experience for gaming.
# umu is the preferred way to launch GE-Proton.
umu_version="1.3.0"
ge_version="10-29"
wine_flavor="umu"
wine_version="GE-Proton${ge_version}"
takp_prefix="$HOME/.takp"

echo_opts()
{
    for var in ${opts[@]}; do
        echo -n "${var}:,"
    done
}

print_help()
{
    echo "$0 [[options] --] command [command args]"
    echo "    Options and current/default values are:"
    for var in ${opts[@]}; do
        echo "        --$var ${!var}"
    done
    echo "    Commands are:"
    for command in ${commands[@]}; do
        echo "        $command"
    done
}

# Argument parsing
opts=(takp_prefix wine_version wine_base wine_prefix)
commands=(install run wine)

OPTS=$(getopt -o h --long help,$(echo_opts) -n 'parse-options' -- "$@") || print_help
eval set -- "${OPTS}"

while [ -n "$1" ]; do
    case "$1" in
        -h | --help)
            print_help; exit 0;;
        --)
            shift; break;;
        --*)
            # All the rest of the long opts take two arguments
            export ${1#--}="${2}"
            shift; shift;;
    esac
done

# Values based on Commandline options
cache_base="${takp_prefix}/cache"
wine_base="${takp_prefix}/wineversion/${wine_version}"
wine_bin="${wine_base}/umu/umu-run"
if [ -z "${wine_prefix}" ]; then
    wine_prefix="${takp_prefix}/wineprefix/${wine_version}"
fi
log_base="${takp_prefix}/logs"
log_dir="${log_base}/${wine_version}"
drive_e="${takp_prefix}/drive_e"
takp_dir="${drive_e}/TAKP"

# Required umu/proton environtment variables
export PROTONPATH="${wine_base}"
# "run" allows us to execute multiple applications in the same umu environment
export PROTON_VERB="run"
export PROTON_DXVK_D3D8=1
if [ -e ${takp_dir}/dxvk.conf ]; then
    export DXVK_CONFIG_FILE=${takp_dir}/dxvk.conf
fi
# Isolate all of umu's data inside the takp_prefix
export XDG_DATA_HOME="${takp_prefix}/xdg/data"
export XDG_CACHE_HOME="${takp_prefix}/xdg/cache"
export XDG_STATE_HOME="${takp_prefix}/xdg/state"

# Do you like Midi? Try fluidsynth, and turn in-game sound to midi
midi=0
start_midi()
{
    if ! pidof fluidsynth >/dev/null; then
        nohup fluidsynth -l -s -i -aalsa -o audio.alsa.device=default "${takp_dir}"/synthusr-samplefix-BASSMIDI.sf2 >/dev/null 2>&1 &
    fi
}

stop_midi()
{
    killall fluidsynth
}

# Helper functions not designed to be called directly
_account_password()
{
    var="account_${1}_password"
    echo ${!var:-${default_password}}
}

_account_suffix()
{
    var="account_${1}_suffix"
    echo ${!var:-${default_suffix}}
}

_client_pid()
{
    var="client_${1}_pid"
    echo ${!var:-0}
}

_accounts()
{
    if [ -z "$1" ]; then
        echo ${accounts[@]}
    else
        echo $@
    fi
}

_gdown40()
{
    fileid="$1"
    filename="$2"
    curl -L "https://drive.usercontent.google.com/download?id=${fileid}&confirm=xxx" -o $filename
}

_set_client_window()
{
    let "client_${1}_window=$2"
}

_set_client_pid()
{
    let "client_${1}_pid=$2"
}

_client()
{
    var="client_${1}_window"
    echo ${!var}
}

_client_pid()
{
    var="client_${1}_pid"
    echo ${!var}
}

_collect_window_info()
{
    windows=$(xdotool search --name _takp- ||:)
    for window in $windows; do
        windowname=$(xdotool getwindowname $window);
        client=${windowname/_takp-/}
        pid=$(xdotool getwindowpid $window)

        _set_client_window ${client} $window
        _set_client_pid ${client} $pid

        #client ${client}
        #_client_pid ${client}
    done
}

# Advanced Commands

# Convenient way to run an arbitrary command inside the WINE environment
# For example:
#   launcher.sh wine winecfg
#   launcher.sh wine ~/Downloads/some_other_program.exe
#   launcher.sh wine cmd.exe
wine()
{
    ${wine_bin} $@
}

winecfg()
{
    ${wine_bin} winecfg
}

add_override()
{
    dll=$1
    local override="native,builtin"
    if [ -n "$2" ]; then
        override=$2
    fi
    ${wine_bin} reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v $dll /d $override /f >/dev/null 2>&1
}

del_override()
{
    dll=$1
    ${wine_bin} reg delete 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v $dll /f >/dev/null 2>&1
}

uninstall_dll()
{
    local dll=$1
    rm ${takp_dir}/$dll.dll
    del_override $dll
}

install_dxvk()
{
    ver="2.7.1"
    if [ -n "$1" ]; then
        ver="$1"
    fi
    tarname="dxvk-${ver}.tar.gz"
    filename="${cache_base}/${tarname}"
    dirname="${cache_base}/dxvk-${ver}"
    if [ -n "$2" ]; then
        dlls=$@
    else
        dlls=("d3d8" "d3d9")
        #dlls=("d3d11" "dxgi")
    fi
    if [ ! -e "${dirname}" ]; then
        if [ ! -e "$filename" ]; then
            curl -L -o "${filename}" "https://github.com/doitsujin/dxvk/releases/download/v${ver}/${tarname}"
        fi
        tar -C "${cache_base}" -xf "${filename}"
    fi
    for dll in ${dlls[@]}; do
        cp -a ${dirname}/x32/$dll.dll ${takp_dir}
        add_override $dll
    done
}

uninstall_dxvk()
{
    if [ -n "$2" ]; then
        dlls=$@
    else
        dlls=("d3d8" "d3d9")
        #dlls=("d3d11" "dxgi")
    fi
    for dll in ${dlls[@]}; do
        uninstall_dll ${dll}
    done
}

install_takp_dlls()
{
    set -x
    old_eqw_sha=4d850dac67c11bd3b72828cbed90cff3818be8025b877224edc93eea9d2cbdc7
    cur_eqw_sha=$(sha256sum ${takp_dir}/eqw.dll | awk '{print $1}')
    if [ "$cur_eqw_sha" = "$old_eqw_sha" ]; then
        mv "${takp_dir}/eqw.dll" "${takp_dir}/eqw.dll.old"
    fi
    eqw_version="1.0.1"
    eqw_cache_file="${cache_base}/eqw-${eqw_version}.dll"

    if [ ! -e "${eqw_cache_file}" ]; then
        curl -L https://github.com/CoastalRedwood/eqw_takp/releases/download/v${eqw_version}/eqw.dll -o "${eqw_cache_file}"
    fi
    cp ${eqw_cache_file} ${takp_dir}/eqw.dll


    old_eqgame_sha=79942003830fc7e73c1c3f2a76fa884258c6e6d53280aad519cad136c850e3fc
    cur_eqgame_sha=$(sha256sum ${takp_dir}/eqgame.dll | awk '{print $1}')
    if [ "$cur_eqgame_sha" = "$old_eqgame_sha" ]; then
        mv "${takp_dir}/eqgame.dll" "${takp_dir}/eqgame.dll.old"
    fi
    eqgame_version="0.0.0.3"
    eqgame_cache_file="${cache_base}/eqgame-${eqgame_version}.dll"

    if [ ! -e "${eqgame_cache_file}" ]; then
        curl -L https://github.com/EQMacEmu/eqgame_dll_takp/releases/download/v${eqgame_version}/eqgame.dll -o "${eqgame_cache_file}"
    
    fi
    cp ${eqgame_cache_file} ${takp_dir}/eqgame.dll
}

install_wine()
{
    ge_proton_filename="GE-Proton${ge_version}.tar.gz"
    ge_proton_url="https://github.com/GloriousEggroll/proton-ge-custom/releases/download/GE-Proton${ge_version}/${ge_proton_filename}"
    ge_proton_file="${cache_base}/${ge_proton_filename}"
    if [ ! -e "${ge_proton_file}" ]; then
        curl -L "${ge_proton_url}" -o "${ge_proton_file}"
    fi

    if [ ! -e "${wine_base}" ]; then
        mkdir -p "${takp_prefix}"/wineversion
        tar -C "${takp_prefix}"/wineversion -xf ${ge_proton_file}
    fi

    umu_filename="umu-launcher-${umu_version}-zipapp.tar"
    umu_url="https://github.com/Open-Wine-Components/umu-launcher/releases/download/${umu_version}/${umu_filename}"
    umu_file="${cache_base}/${umu_filename}"

    if [ ! -e "${umu_file}" ]; then
        curl -L "${umu_url}" -o "${umu_file}"
    fi

    if [ ! -e "${wine_base}/umu" ]; then
        tar -C "${wine_base}" -xf ${umu_file}
    fi
}

install_TAKP()
{
    if [ ! -e "${takp_dir}/eqgame.exe" ]; then
        filename="${cache_base}/TAKP.zip"
        if [ ! -e "${filename}" ]; then
            _gdown40 "1X46DuXJYPl2igcwO2f5sHuX1XKX9BhVy" "${filename}"
        fi
        unzip -d "${drive_e}" "${filename}"
    fi
}

# Commands

play()
{
    mkdir -p "${log_dir}"
    cd "${takp_dir}"
    "${wine_bin}" eqgame.exe patchme >"${log_dir}"/wine.log 2>&1
}

launch()
{
    for account in $(_accounts $@); do
        # Quick check that you have 1GiB of memory available.
        # Otherwise we can risk oom-killer or thrasing SWAP
        memFree=$(awk '/^MemAvailable:/{available=$2}
                       /^Buffers:/{buffers=$2}
                       /^Cached:/{cached=$2}
                       END {print available+buffers+cached}' /proc/meminfo)
        memReq=$((1024 * 1024))
        if [ $memFree -lt $memReq ]; then
            echo "Not enough memory: $memFree < $memReq" >&2
            exit 1
        fi
        client=$(_client ${account})
        if [ -z "$client" ]; then
            run $account
            sleep 1
        else
            echo Client "[ _takp-${account} : $client ]" already running.
        fi
    done
}

login()
{
    for account in $(_accounts $@); do
        pass=$(_account_password ${account})
        client=$(_client ${account})
        if [ -n "$client" ]; then
            echo "Activating window"
            xdotool windowactivate ${client}
            sleep 1
            xdotool key Return
            sleep 1
        fi
    done
}

startup()
{
    # Launch and login
    _collect_window_info
    if [ $midi -eq 1 ]; then
        start_midi
    fi
    launch $@
    # TODO: Sleeps are bad. We can do better.... someday
    sleep 10
    _collect_window_info
    login $@
}

close()
{
    # Close all client windows
    _collect_window_info
    for account in $(_accounts $@); do
        pid=$(_client_pid ${account})
        while [ -n "$pid" ] && [ -e /proc/$pid ]; do
            kill $pid
            sleep .1
        done
    done
}

camp()
{
    # Send each window the in-game `/camp` command
    _collect_window_info
    for account in $(_accounts $@); do
        client=$(_client ${account})
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep .2
            xdotool type /camp
            xdotool key Return
        fi
    done
    sleep 35
    close $@
}

install()
{
    mkdir -p "${wine_prefix}"
    install_wine
    install_TAKP
    install_takp_dlls
}

run()
{
    account=$1
    pass=$(_account_password $account)
    dir="${takp_dir}$(_account_suffix $account)"
    mkdir -p "${log_dir}"
    cd "${dir}"
    "${wine_bin}" eqgame.exe patchme "/title:_takp-${account}" "/ticket:$account/$pass" >"${log_dir}"/wine-${account}.log 2>&1 &
    _set_client_pid ${account} $!
}

if [ ${#@} -eq 0 ]; then
    print_help
fi
$@
