#!/bin/bash

set -e

# Need gtk libs in order to draw windows, and it'll pull in all but the 32bit graphics drivers for x11 we need.
# CentOS7: gtk3.i686


midi=0
# Config file for usernames and passwords
takp_config=$HOME/.takp_launcher.conf
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

account_password()
{
    var="account_${1}_password"
    echo ${!var:-${default_password}}
}

account_suffix()
{
    var="account_${1}_suffix"
    echo ${!var:-${default_suffix}}
}

client_pid()
{
    var="client_${1}_pid"
    echo ${!var:-0}
}

accounts()
{
    if [ -z "$1" ]; then
        echo ${accounts[@]}
    else
        echo $@
    fi
}

gdown40()
{
    fileid="$1"
    filename="$2"
    cookie="/tmp/takp-google-cookie"
    tmphtml=/tmp/takp-google-tmphtml
    curl -c $cookie -L "https://drive.google.com/uc?export=download&id=${fileid}" -o $tmphtml
    confirm=$(cat $tmphtml | awk 'match($0, "confirm=([^&]+)&", m) {print m[1]}')
    curl -b $cookie -L "https://drive.google.com/uc?export=download&id=${fileid}&confirm=${confirm}" -o $filename
}

# Old-trusty 2.2-staging
takp_prefix="$HOME/.takp"
wine_version="2.2-staging"

# Wine 6.0 + dgVoodoo + dxvk is working!
wine_version=system

# Always needed
winetricks_verbs="dinput8"

opts=(takp_prefix wine_version winetricks_verbs wine_base)
commands=(install run install_and_run winetricks wine)

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

# CentOS 7
steam_apps_dir="$HOME/.local/share/Steam/steamapps"
# Ubuntu 18.04
steam_apps_dir="$HOME/.steam/steam/steamapps"
steam_proton_dir="${steam_apps_dir}/common/Proton 6.3/dist"
steam_wine_prefix="${steam_proton_dir}/share/default_pfx"

# Depends on parsed arguments
if [ "${wine_version}" == "system" ]; then
    wine_base=/usr
fi
if [ -z "${wine_base}" ]; then
    wine_base="${takp_prefix}/wineversion/${wine_version}"
fi
wine_prefix="${takp_prefix}/wineprefix/${wine_version}"
wine_bin="${wine_base}/bin/wine"
wineserver_bin="${wine_base}/bin/wineserver"
cache_base="${takp_prefix}/cache"
log_base="${takp_prefix}/logs"
log_dir="${log_base}/${wine_version}"
wine_filename="PlayOnLinux-wine-${wine_version}-linux-x86.pol"
wine_baseurl="https://www.playonlinux.com/wine/binaries/linux-x86"
wine_url="${wine_baseurl}/${wine_filename}"
wine_file="${cache_base}/${wine_filename}"
winetricks_url="https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks"
winetricks_bin="${cache_base}/winetricks"
drive_e="${takp_prefix}/drive_e"
takp_dir="${drive_e}/TAKP"

# Values for launching TAKP client
gs_key="org.gnome.settings-daemon.plugins.media-keys.custom-keybinding"
gs_key_path="/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings"

export WINEARCH=win32
export WINEPREFIX="${wine_prefix}"
export WINE="${wine_bin}"
#export WINEDEBUG=+relay,+seh,+tid,+d3d,+swapchain,+loaddll,+d3d9
#export WINEDEBUG="+wgl"
#export WINEDEBUG=+relay,+seh,+tid
#export WINEDEBUG=+timestamp,+loaddll,+d3d
#export WINEDEBUG=+timestamp,+loaddll

# You can force wine to ignore installing mono on first run, which doesn't seem to be used by the TAKP client, and it is helpful to test out a fresh wine prefix
#export WINEDLLOVERRIDES="mscoree,mshtml="

# With dxvk, you can see fps
export DXVK_HUD=fps
#export DXVK_HUD=full

install()
{
    mkdir -p "${wine_prefix}"
    # Some rudimentary caching
    mkdir -p "${cache_base}"
    if [ ! -e "${winetricks_bin}" ]; then
        curl -L "${winetricks_url}" -o "${winetricks_bin}"
        chmod +x "${winetricks_bin}"
    fi
    if [ "${wine_version}" = "steam" ]; then
        if [ ! -e "${wine_base}" ]; then
            mkdir -p "${wine_base}" "${wine_prefix}"
            rsync --exclude=default_pfx -av "${steam_proton_dir}"/ "${wine_base}"/
            rsync -av "${steam_wine_prefix}"/ "${wine_prefix}"/
        fi
    elif [ "${wine_version}" = "system" ]; then
        echo "system wine"
    else
        if [ ! -e "${wine_file}" ]; then
            curl -L "${wine_url}" -o "${wine_file}"
        fi

        if [ ! -e "${wine_base}" ]; then
            tar -C "${takp_prefix}"/ -xf "${wine_file}" wineversion/
        fi
    fi

    if [ ! -e "${takp_dir}/eqgame.exe" ]; then
        filename="${cache_base}/TAKP.zip"
        if [ ! -e "${filename}" ]; then
            gdown40 "1X46DuXJYPl2igcwO2f5sHuX1XKX9BhVy" "${filename}"
        fi
        unzip -d "${drive_e}" "${filename}"
    fi

    if [ $(sha1sum "${takp_dir}/eqgame.dll" | awk '{print $1}') != "e06d07cf1f21922caf31b19eef220655e8e76256" ]; then
        filename="${cache_base}/freethemouse.zip"
        if [ ! -e "$filename" ]; then
            curl -L -o "$filename" 'https://www.dropbox.com/s/zbha7dxydsr92w0/eqgame_dll%20v3.5.3%20for%20ftm.zip?dl=0'
        fi
        mv "${takp_dir}"/eqgame.dll{,.dist}
        unzip -d "${takp_dir}" "${filename}" "eqgame.dll"
    fi

    if [ ! -e ${wine_prefix}/system.reg ]; then
        "${wine_bin}" wineboot -u ||:
        "${wineserver_bin}" -k -w ||:
        "${winetricks_bin}" ${winetricks_verbs}
        "${wineserver_bin}" -k -w ||:
        ln -sf "${drive_e}" "${wine_prefix}/dosdevices/e:"
    fi

    if [ ! -e ${takp_dir}/d3d8.dll ]; then
        zipname="dgVoodoo2_75.zip"
        filename="${cache_base}/${zipname}"
        if [ ! -e "$filename" ]; then
            curl -L -o "$filename" "http://dege.freeweb.hu/dgVoodoo2/bin/${zipname}"
        fi
        unzip -d "${takp_dir}" "${filename}" "MS/x86*"
        mv ${takp_dir}/MS/x86/D3D8.dll ${takp_dir}/d3d8.dll
        mv ${takp_dir}/MS/x86/D3D9.dll ${takp_dir}/d3d9.dll
        ${wine_bin} reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v d3d8 /d native /f >/dev/null 2>&1
        ${wine_bin} reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v d3d9 /d native /f >/dev/null 2>&1
    fi

    if [ ! -e ${takp_dir}/d3d11.dll ]; then
        ver="1.9"
        tarname="dxvk-${ver}.tar.gz"
        filename="${cache_base}/${tarname}"
        if [ ! -e "$filename" ]; then
            curl -L -o "$filename" "https://github.com/doitsujin/dxvk/releases/download/v${ver}/${tarname}"
        fi
        tar -C "${takp_dir}" -xf "${filename}" "dxvk-${ver}/x32"
        mv ${takp_dir}/dxvk-${ver}/x32/d3d10.dll ${takp_dir}/d3d10.dll
        mv ${takp_dir}/dxvk-${ver}/x32/d3d11.dll ${takp_dir}/d3d11.dll
        ${wine_bin} reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v d3d10 /d native /f >/dev/null 2>&1
        ${wine_bin} reg add 'HKEY_CURRENT_USER\Software\Wine\DllOverrides' /v d3d11 /d native /f >/dev/null 2>&1
    fi
}

disable_dgVoodoo_watermark()
{
    echo "[DirectX]" > "${takp_dir}"/dgVoodoo.conf
    echo "dgVoodooWatermark = false" >> "${takp_dir}"/dgVoodoo.conf
}

play()
{
    mkdir -p "${log_dir}"
    cd "${takp_dir}"
    "${wine_bin}" eqgame.exe patchme >"${log_dir}"/wine.log 2>&1
}

run()
{
    account=$1
    pass=$(account_password $account)
    dir="${takp_dir}$(account_suffix $account)"
    mkdir -p "${log_dir}"
    cd "${dir}"
    "${wine_bin}" eqgame.exe patchme "/title:takp-${account}_login" "/ticket:$account/$pass" >"${log_dir}"/wine-${account}.log 2>&1 &
    set_client_pid ${account} $!
}

install_and_run()
{
    install
    run
}

winetricks()
{
    ${winetricks_bin} $@
}

wine()
{
    ${wine_bin} $@
}

wineserver()
{
    ${wineserver_bin} $@
}

execute()
{
    $@
}

set_client_window()
{
    let "client_${1}_window=$2"
}

set_client_pid()
{
    let "client_${1}_pid=$2"
}

client()
{
    var="client_${1}_window"
    echo ${!var}
}

client_pid()
{
    var="client_${1}_pid"
    echo ${!var}
}

collect_window_info()
{
    windows=$(xdotool search --name takp- ||:)
    for window in $windows; do
        windowname=$(xdotool getwindowname $window);
        client=${windowname/takp-/}
        pid=$(xdotool getwindowpid $window)

        set_client_window ${client} $window
        set_client_pid ${client} $pid

        #client ${client}
        #client_pid ${client}
    done
}

get_keyboard_shortcuts()
{
    gsettings get org.gnome.settings-daemon.plugins.media-keys custom-keybindings
    for num in {0..2}; do
        gsettings get ${gs_key}:${gs_key_path}/custom${num}/ name #'<newname>'
        gsettings get ${gs_key}:${gs_key_path}/custom${num}/ command #'<newcommand>'
        gsettings get ${gs_key}:${gs_key_path}/custom${num}/ binding #'<key_combination>'
    done

}

set_keyboard_shortcuts()
{
    gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings "['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom1/', '/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom2/']"

    gsettings set ${gs_key}:${gs_key_path}/custom0/ name "Client1"
    gsettings set ${gs_key}:${gs_key_path}/custom0/ command "$0 activate 1"
    gsettings set ${gs_key}:${gs_key_path}/custom0/ binding "KP_Divide"

    gsettings set ${gs_key}:${gs_key_path}/custom1/ name "Client2"
    gsettings set ${gs_key}:${gs_key_path}/custom1/ command "$0 activate 2"
    gsettings set ${gs_key}:${gs_key_path}/custom1/ binding "KP_Multiply"

    gsettings set ${gs_key}:${gs_key_path}/custom2/ name "Client3"
    gsettings set ${gs_key}:${gs_key_path}/custom2/ command "$0 activate 3"
    gsettings set ${gs_key}:${gs_key_path}/custom2/ binding "KP_Subtract"
}

drop_keyboard_shortcuts()
{
    gsettings reset ${gs_key}:${gs_key_path}/custom0/ name
    gsettings reset ${gs_key}:${gs_key_path}/custom0/ command
    gsettings reset ${gs_key}:${gs_key_path}/custom0/ binding

    gsettings reset ${gs_key}:${gs_key_path}/custom1/ name
    gsettings reset ${gs_key}:${gs_key_path}/custom1/ command
    gsettings reset ${gs_key}:${gs_key_path}/custom1/ binding

    gsettings reset ${gs_key}:${gs_key_path}/custom2/ name
    gsettings reset ${gs_key}:${gs_key_path}/custom2/ command
    gsettings reset ${gs_key}:${gs_key_path}/custom2/ binding

    gsettings reset org.gnome.settings-daemon.plugins.media-keys custom-keybindings
}

launch()
{
    for account in $(accounts $@); do
        memFree=$(awk '/^MemAvailable:/{available=$2}
                       /^Buffers:/{buffers=$2}
                       /^Cached:/{cached=$2}
                       END {print available+buffers+cached}' /proc/meminfo)
        memReq=$((1024 * 1024))
        if [ $memFree -lt $memReq ]; then
            echo "Not enough memory: $memFree < $memReq" >&2
            exit 1
        fi
        client=$(client ${account})
        if [ -z "$client" ]; then
            run $account
            sleep 1
        else
            echo Client "[ takp-${account} : $client ]" already running.
        fi
    done
}

rename_window()
{
    echo "Renaming window $1 -> $2"
    xdotool search --name "$1" set_window --name "$2"
}

swap_window_name()
{
    rename_window $1 takp-SWAP
    rename_window $2 $1
    rename_window takp-SWAP $1
}

login()
{
    for account in $(accounts $@); do
        pass=$(account_password ${account})
        client=$(client ${account}_login)
        if [ -n "$client" ]; then
            echo "Activating window"
            xdotool windowactivate ${client}
            sleep .5
            rename_window takp-${account}_login takp-${account}
            sleep .5
            xdotool key Return
            sleep 1
        fi
    done
}

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

startup()
{
    if [ $midi -eq 1 ]; then
        start_midi
    fi
    launch $@
    sleep 7
    collect_window_info
    login $@
}

altReturn()
{
    client=$1
    xdotool windowactivate ${client}
    sleep $sleeptime
    xdotool keydown alt
    sleep $sleeptime
    xdotool key Return
    sleep $sleeptime
    xdotool keyup alt
    sleep $sleeptime
}

# Helper script to move 3 clients to 3 different X positions on the desktop
position()
{
    num=0
    sleeptime=.2
    pos=(0 1920 3840)
    for account in $(accounts $@); do
        client=$(client ${account})
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep $sleeptime
            xdotool windowactivate ${client}
            sleep $sleeptime
            xdotool windowmove --sync ${client} ${pos[$num]} 0
            sleep $sleeptime
            xdotool windowsize --sync ${client} 1920 1080
            sleep $sleeptime
        fi
        num=$((num+1))
    done
}

close()
{
    for account in $(accounts $@); do
        pid=$(client_pid ${account})
        while [ -n "$pid" ] && [ -e /proc/$pid ]; do
            kill $pid
            sleep .1
        done
    done
}

camp()
{
    for account in $(accounts $@); do
        client=$(client ${account})
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep .2
            xdotool type /camp
            xdotool key Return
        fi
    done
    sleep 35
    for account in $(accounts $@); do
        client=$(client ${account})
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep 1
            xdotool key Escape
        fi
    done
    sleep 5
    close $@
}

emote()
{
    for account in $(accounts $@); do
        client=$(client ${account})
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep .1
            xdotool type /dance
            xdotool key Return
        fi
    done
}

activate()
{
    index=$((${1} - 1))
    account=${accounts[${index}]}
    client=$(xdotool search --name takp-${account})
    currentWindow=$(xdotool getactivewindow)
    if [ "${client}" -eq "${currentWindow}" ]; then
        echo "Moving mouse"
        xdotool mousemove $((1920 / 2 + 1920 * ${index})) $((1080/2))
    elif ! wmctrl -a takp-${account}; then
        echo "Couldn't find a window for ${account}" >&2
    fi
}

nparse_install()
{
    if [ -d "$takp_prefix/nparse-takp" ]; then
	echo "nparse already in place"
	exit 0
    fi
    mkdir -p $takp_prefix/nparse-takp
    cd $takp_prefix/nparse-takp
    wget https://github.com/hitechhippie/nparse-takp/archive/refs/tags/v0.6.0-takp.tar.gz
    tar zxvf v0.6.0-takp.tar.gz
    rm -f v0.6.0-takp.tar.gz
    mv nparse-takp-0.6.0-takp nparse-takp-template
    sed -i '1c#!/usr/bin/python3' nparse-takp-template/nparse.py
    chmod +x nparse-takp-template/nparse.py
}

nparse()
{
    account=$1
    cd $takp_prefix/nparse-takp
    if [ ! -d "$takp_prefix/nparse-takp/$1" ]; then
	cp -a nparse-takp-template $1
    fi
    cd $1
    ./nparse.py &
}

collect_window_info

if [ ${#@} -eq 0 ]; then
    print_help
fi
$@
