#!/bin/bash
if false
then
# based on a script from http://invisible-island.net/xterm/xterm.faq.html
# on my system, the following line can be replaced by the line below it
#tput u6 > /dev/tty
#echo -en "\033[6n" > /dev/tty
tput u7 > /dev/tty    # when TERM=xterm (and relatives)
IFS=';' read -r -d R -a pos
# change from one-based to zero based so they work with: tput cup $row $col
row=$((${pos[0]:2} - 1))    # strip off the esc-[
col=$((${pos[1]} - 1))
fi

function getu7() {
#exec < /dev/tty
#oldstty=$(stty -g)
#stty raw -echo min 0
  tput u7
  IFS=';' read -r -d R -a pos
#stty $oldstty
}

function getrow () {
  [ -z $pos ] && getu7
  echo $((${pos[0]:2} - 1))
}

function getcol () {
  [ -z $pos ] && getu7
  echo $((${pos[1]} - 1))
}

function getpos () {
  [ -z $pos ] && getu7
  echo $((${pos[1]} - 1)), $((${pos[0]:2} - 1))
}

#getrow

tput u7; IFS=';' read -r -d R -a pos;row=$((${pos[0]:2} - 1))

function centertext () {
  [ -z $row ] && {
    tput u7
    IFS=';' read -r -d R -a pos
    row=${pos[0]:2}
  }

  c=$(tput cols)

  for k in "$@"
  do
    let row++
    let x=($c-${#k})/2
    tput cup $row $x 
    echo "$k"
  done

  tput sgr0
}

while true
do
row=-1
tput bold setab 123
centertext "  tet  " " toto2 " "  tutu " "  tata "
row=-1
tput bold setab 124
centertext "  tet  " " toto2 " "  tutu " "  tata "
done
