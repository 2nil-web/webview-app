#!/usr/bin/bash

# Pour éviter que les accents dans les noms de fichier/dossier ne se transforme en ?
export LANG=fr_FR.UTF-8

dbg_bak () {
  echo "bak $1 ==> $2 exclude_for_famille.txt"
}

dbg_bakUser () {
  echo "bakUser $1 ==> $2 exclude_for_user.txt"
}

bak () {
 /usr/bin/rsync --progress -avu --chmod=755 --chown=nobody:nogroup -e "ssh -i $HOME/.ssh/id_rsa" --exclude=*~ --exclude=*.o --exclude=.AndroidStudio2.3 --exclude=.android --exclude=VirtualBox\ VMs --exclude=.dl --exclude=Backup --exclude=NoBackup/ "$1" "$2"
}

bakUser () {
  /usr/bin/rsync --progress -avu --chmod=755 --chown=nobody:nogroup -e "ssh -i $HOME/.ssh/id_rsa" --exclude=Nextcloud --exclude=OneDrive --exclude=OneDrive\ -\ AKKA --exclude=*~ --exclude=*.o --exclude=*.mp3 --exclude=*.ogg --exclude=*.m4a --exclude=.AndroidStudio2.3 --exclude=.android --exclude=VirtualBox\ VMs --exclude=.dl --exclude=Backup --exclude=NoBackup --exclude=AppData --exclude=AFaire --exclude=Downloads --exclude=NL --exclude=bad_roms --exclude=Audio --exclude=Lecture --exclude=Bibliothéque\ Calibre --exclude=ManiaPlanet --exclude=TmForever --exclude=ntuser.dat* --exclude=NTUSER.DAT* --exclude=Nextcloud* --exclude=OneDrive* --exclude=casal/vim/ "$1" "$2"
}

dbg="dbg_"
dbg=""
while true
do
  src=$(/usr/bin/cygpath $1)
  dst=$(/usr/bin/cygpath $2)
  typ=$3

  msg="Sauvegarde user $src ==> $dst"
  echo -e "\n$msg"
  if [ "$typ" = "fam" ]
  then
    ${dbg}bak "$src" "$dst"
  else
    ${dbg}bakUser "$src" "$dst"
  fi
  printf %${#msg}s | tr " " "="
  echo

  shift 3
  [ -z "$1" ] && break;
done

