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

USER=`whoami`

# create a tempfile to hold dialogs responses
tempfile=`tempfile 2>/dev/null` || tempfile=/tmp/jsh_installer$$

# cleanup tempfile if any of the signals - SIGHUP SIGINT SIGTERM it received.
trap "rm -f $tempfile; exit" SIGHUP SIGINT SIGTERM

exit_installer()
{
    clear
    echo "jsh installer exited"
    exit
}

############################## HELLO DIALOG #############################

$DIALOG --title "jsh installer" --msgbox "Hello $USER, this installer will guide you through \
the jsh build and install process.\n\nHit enter to continue; ESC any time to abort." 10 41

retval=$?

if [ $retval -eq 255 ]
then
    exit_installer
fi

############################## INSTALL TARGETS DIALOG #############################

$DIALOG --title "jsh installer" \
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

$DIALOG --title "jsh installer" \
        --separate-output \
        --checklist "select the compile flags below" 10 70 5 \
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

clear
