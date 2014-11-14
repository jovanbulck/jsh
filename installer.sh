#!/bin/sh
# =============================================================
# This file is part of jsh.
# 
# jsh (jo-shell): A basic shell implementation
# Copyright (C) 2014 Jo Van Bulck <jo.vanbulck@student.kuleuven.be>
#
# jsh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# jsh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with jsh.  If not, see <http://www.gnu.org/licenses/>.
# ============================================================

############################## COMMON THINGS #############################

# common options for all dialogs
DIALOG="dialog --stderr --clear"

#USER=`whoami`
INSTALL_PATH="/usr/local/bin"

# create a tempfile to hold dialogs responses
tempfile=`tempfile 2>/dev/null` || tempfile=/tmp/jsh_installer$$

# cleanup tempfile if any of the signals - SIGHUP SIGINT SIGTERM it received.
trap "rm -f $tempfile; exit" SIGHUP SIGINT SIGTERM

exit_installer()
{
    clear
    echo "jsh installer exited"
    rm -f $tempfile
    exit
}

display_info()
{
    $DIALOG --backtitle "jsh installer" \
            --title "$1" \
            --msgbox "$2" 8 50 
    retval=$?
    if [ $retval -eq 255 ]
    then
        exit_installer
    fi
}

############################## HELLO DIALOG #############################

$DIALOG --backtitle "jsh installer" --title "Install jsh" \
        --msgbox "Hello $USERNAME, this installer will guide you through \
the jsh build and install process.\n\nHit enter to continue; ESC any time to abort." 10 41

retval=$?
if [ $retval -eq 255 ]
then
    exit_installer
fi

############################## INSTALL TARGETS DIALOG #############################

$DIALOG --backtitle "jsh installer" \
        --title "Install targets" \
        --separate-output \
        --checklist "select the install targets below" 10 70 5 \
        "jsh"   "the jo-shell - a basic UNIX shell implementation in C" on \
        "man"   "the jsh manpage" on \
        2> $tempfile

retval=$?
case $retval in
  0) # OK pressed; parse response
    while read line
    do
        case ${line} in
        "jsh")
            echo "you chose jsh";;
        "man")
            echo "you chose the man page";;
        esac
    done < $tempfile;;
  1)
    exit_installer;; # Cancel pressed
  255)
    exit_installer;; # ESC pressed
esac

############################## COMPILE FLAGS DIALOG #############################

$DIALOG --backtitle "jsh installer" \
        --title "Compile flags" \
        --separate-output \
        --checklist "select the compile flags below" 12 90 5 \
        "color"     "colorize jsh debug and error messages" on \
        "rcfile"    "auto load the ~/.jshrc file on jsh boot" on \
        "debug"     "turn on debug output by default" off \
        "update"    "check for new jsh release on jsh boot" off \
        "fallback"  "don't use the GNU readline library for input line editing and history" off \
        2> $tempfile

retval=$?
case $retval in
  0) # OK pressed; parse response
    while read line
    do
        case ${line} in
        "jsh")
            echo "you chose jsh";;
        "man")
            echo "you chose the man page";;
        esac
    done < $tempfile;;
  1)
    exit_installer;; # Cancel pressed
  255)
    exit_installer;; # ESC pressed
esac

############################## SELECT INSTALL DIRECTORY DIALOG ###################

        # --dselect /usr/local/bin -1 -1
$DIALOG --backtitle "jsh installer" \
        --title "Install directory" \
        --inputbox "type the jsh installation directory below" \
        7 50 "$INSTALL_PATH" \
        2> $tempfile

retval=$?
case $retval in
  0) # OK pressed; set install path
    while read line
    do
        case ${line} in
        "jsh")
            echo "you chose jsh";;
        "man")
            echo "you chose the man page";;
        esac
    done < $tempfile;;
  1)
    exit_installer;; # Cancel pressed
  255)
    exit_installer;; # ESC pressed
esac

############################## SELECT MAN INSTALL DIRECTORY DIALOG ###################

$DIALOG --backtitle "jsh installer" \
        --title "Install directory" \
        --inputbox "type the jsh manpage installation directory below" \
        8 50 "/usr/local/share/man/man1" \
        2> $tempfile

retval=$?
case $retval in
  0) # OK pressed; parse response
    while read line
    do
        case ${line} in
        "jsh")
            echo "you chose jsh";;
        "man")
            echo "you chose the man page";;
        esac
    done < $tempfile;;
  1)
    exit_installer;; # Cancel pressed
  255)
    exit_installer;; # ESC pressed
esac

############################## JSH CONFIG FILES ##############################

show_config_file_dialog()
{
    file=$1
    $DIALOG --backtitle "jsh installer" \
            --title "Create jsh configuration file" \
            --yesno "The installer hasn't found an existing '$file' config file. \
            Should I create an empty one? You will be provided with the possibility \
to edit it afterwards." 8 60
    retval=$?
    case $retval in 
        0) # Yes : create a default file and edit
            touch $file
            echo $2 > $file
            if [ $# -eq 3 ] && [ $3 = "add_dummy_conf" ]
            then
                echo "#" >> $file
                echo "# Insert commands here to create your own custom jsh shell! :-)" >> $file
                echo "# To get you started, see (https://github.com/jovanbulck/jo-shell/wiki/Sample-\
configuration-files)" >> $file
                echo "# for more info and example configuration files" >> $file
                echo "" >> $file
                echo "" >> $file
            fi
            show_edit_config_file_dialog $file;;
        1) # No : continue normal execution
            ;;
        255) # ESC pressed
            exit_installer;;
    esac
}

show_edit_config_file_dialog()
{
    file=$1
    $DIALOG --backtitle "jsh installer" \
            --title "Edit the new jsh configuration file below" \
            --editbox $file 50 100 \
            2> $tempfile
    
    retval=$?
    case $retval in
        0) # OK : write out the file
            cp $tempfile $file;;
        1) # Cancel : continue normal execution; dont write out the file
            ;;
        255) # ESC
            exit_installer;;
    esac
}

if [ ! -e "$HOME/.jshrc" ]
then
    show_config_file_dialog "$HOME/.jshrc" "# ~/.jshrc : file containing jsh-shell commands \
executed by jsh on startup of an interactive session" "add_dummy_conf"
fi

if [ ! -e "$HOME/.jsh_logout" ]
then
    show_config_file_dialog "$HOME/.jsh_logout" "# ~/.jsh_logout: a file containing jsh-shell \
commands executed by jsh when exiting an interactive sesssion." "add_dummy_conf"
fi

if [ ! -e "$HOME/.jsh_login" ]
then
    show_config_file_dialog "$HOME/.jsh_login" "Hi $USERNAME, welcome back to jsh!"
fi

############################## MAKE OUTPUT DIALOG ##############################
clear
echo "Will now make and install jsh to '$INSTALL_PATH'. Type your sudo password below:"
sudo --validate
make JSH_INSTALL_DIR="$INSTALL_PATH" 2>&1 | $DIALOG --backtitle "jsh installer" \
        --title "making jsh" \
        --exit-label "Continue" \
        --programbox "make install jsh output" 100 100

############################## DEFAULT SHELL DIALOG ############################

$DIALOG --backtitle "jsh installer" \
        --title "jsh as default shell" \
        --no-label "Yes" --yes-label "No" \
        --yesno "Do you want to set jsh as your default UNIX login shell? \
        (currently not recommended)" 6 50

retval=$?
case $retval in 
    0) # No pressed
        echo "not changing the default shell";;
    1) # Yes pressed
        clear
        echo "changing the default shell to jsh"
        OLD_SHELL=$SHELL
        chsh -s /usr/local/bin/jsh
        chsh_retval=$?
        if [ $chsh_retval -eq 0 ]
        then
            display_info "changing shell to jsh" "chsh exited successfully: \njsh is now \
your default UNIX login shell. Use 'chsh -s $OLD_SHELL' any time to revert the default shell."
        else
            display_info "changing shell to jsh" "chsh exited with an error: \nyour default
UNIX login shell hasn't changed. Use 'chsh -s $INSTALL_PATH/jsh' after the installation to \
find out why"
        fi;;
   255) # ESC pressed
        exit_installer;;
esac

############################## EXIT SUCCESS DIALOG #############################

display_info "jsh installation completed" "jsh is installed successfully on your system. \
Have fun with your new shell!"

clear
