#!/bin/bash

set -e

# TODO: Add optional Wined3d support
# http://downloads.fdossena.com/Projects/WineD3D/Builds/WineD3DForWindows_2.2-staging.zip

# Need gtk libs in order to draw windows, and it'll pull in all but the 32bit graphics drivers for x11 we need.
# CentOS7: gtk3.i686


# Config file for usernames and passwords
takp_config=$HOME/.takp_launcher.conf
if [ ! -e ${takp_config} ]; then
    cat << _EOF > "${takp_config}"
users=("user1" "user2" "user3")
passwords=("user1" "user2" "user3")
_EOF
    echo "Unable to find config file, so I made one for you. Edit '${takp_config}' to set up your usernames and passwords." >&2
    exit 1
else
    . ${takp_config}
fi

# Defaults that can be overridden
takp_prefix="$HOME/.takp"
wine_version="2.2-staging"
winetricks_verbs="d3dx9 dinput8 csmt=on glsl=disabled"

opts=(takp_prefix wine_version winetricks_verbs)
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
steam_photon_dir="${steam_apps_dir}/common/Proton 3.7/dist"
steam_wine_prefix="${steam_photon_dir}/share/default_pfx"

# Depends on parsed arguments
wine_prefix="${takp_prefix}/wineprefix/${wine_version}"
wine_base="${takp_prefix}/wineversion/${wine_version}"
wine_bin="${wine_base}/bin/wine"
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
boxes=${#users[@]}
gs_key="org.gnome.settings-daemon.plugins.media-keys.custom-keybinding"
gs_key_path="/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings"

export WINEARCH=win32
export WINEPREFIX="${wine_prefix}"
export WINE="${wine_bin}"
#export WINEDEBUG=+relay,+seh,+tid,+d3d,+swapchain,+loaddll,+d3d9
#export WINEDEBUG="+wgl"
#export WINEDEBUG=+relay,+seh,+tid
#export WINEDEBUG=+timestamp,+loaddll,+d3d

# You can force wine to ignore installing mono on first run, which doesn't seem to be used by the TAKP client, and it is helpful to test out a fresh wine prefix
export WINEDLLOVERRIDES="mscoree,mshtml="

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
            rsync --exclude=default_pfx --exclude="dinput.dll.*" -av "${steam_photon_dir}"/ "${wine_base}"/
            #rsync -av "${steam_wine_prefix}"/ "${wine_prefix}"/
        fi
    else
        if [ ! -e "${wine_file}" ]; then
            curl -L "${wine_url}" -o "${wine_file}"
        fi

        if [ ! -e "${wine_base}" ]; then
            tar -C "${takp_prefix}"/ -xf "${wine_file}" wineversion/
        fi
    fi

    if [ ! -e "${takp_dir}/eqgame.exe" ]; then
        echo "Hey, you should install TAKP here: '${takp_dir}', ie '${takp_dir}/eqgame.exe'"
        #TODO: Maybe we should download the client from google docs?
        mkdir -p $(dirname "${takp_dir}")
        rsync -av "$HOME/Downloads/TAKP.dist/" "${takp_dir}"/
    fi

    "${winetricks_bin}" ${winetricks_verbs}
    ln -sf "${drive_e}" "${wine_prefix}/dosdevices/e:"
}

run()
{
    mkdir -p "${log_dir}"
    cd "${takp_dir}"
    "${wine_bin}" eqgame.exe patchme >"${log_dir}"/wine.log 2>&1
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

get_client()
{
    xdotool search --name "${1}" ||:
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

startup()
{
    num=${1:-$boxes}
    for num in $(seq 1 $num); do
        user=${users[$((num-1))]}
        pass=${passwords[$((num-1))]}
        client=$(get_client takp-${user})
        if [ -z "$client" ]; then
            run &
            pid=$!
            echo $pid
            sleep .5
        fi
    done
}

login()
{
    num=${1:-$boxes}
    for num in $(seq 1 $num); do
        user=${users[$((num-1))]}
        pass=${passwords[$((num-1))]}
        client=$(get_client takp-${user})
        if [ -z "$client" ]; then
            xdotool search --name Client set_window --name takp-${user}-login
        fi
        client=$(get_client takp-${user}-login)
        if [ -n "$client" ]; then
            xdotool windowactivate ${client}
            sleep .5
            echo -en "${user}\x0" > ${takp_dir}/equname.txt
            xdotool key Tab Return
            sleep .5
            xdotool key Return
            sleep .5
            xdotool type ${pass}
            xdotool key Return
            sleep .5
            xdotool search takp-${user}-login set_window --name takp-${user}
            sleep .5
            xdotool key Return
            sleep .5
        fi
    done
}

activate()
{
    user=${users[$((${1}-1))]}
    #if ! xdotool search --name takp-${user} windowactivate; then
    if ! wmctrl -a takp-${user}; then
        echo "Couldn't find a window for ${user}" >&2
    fi
}

move_windows()
{
    num=${1:-$boxes}
    for num in $(seq 0 $((num - 1))); do
        user=${users[$num]}
        echo Activating $user
        wmctrl -a takp-${user}
        wmctrl -a takp-${user}
        #wmctrl -r takp-${user} -g 1920,1080
        pos="0,$((1920 * num)),0,1920,1080"
        echo moving $user to $pos
        wmctrl -r takp-${user} -e $pos
    done
}

if [ -z "$@" ]; then
    print_help
fi
$@
