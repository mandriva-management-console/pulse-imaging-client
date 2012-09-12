# -*- coding: utf-8; -*-
#
# (c) 2005-2007 Ludovic Drolez, Linbox FAS
# (c) 2010 Mandriva, http://www.mandriva.com
#
# $Id$
#
# This file is part of Pulse 2.
#
# Pulse 2 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Pulse 2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pulse 2.  If not, see <http://www.gnu.org/licenses/>.

# Global variables which can be interesting for the postinst scripts

. /usr/lib/revolib.sh
. /etc/netinfo.sh

MAC=`cat /etc/shortmac`
HOSTNAME="unknown_host"
# LRS
[ -f /revoinfo/$MAC/hostname ] && HOSTNAME=`cat /revoinfo/$MAC/hostname | tr : /`
# Pulse 2
[ -f /revoinfo/hostname ] && HOSTNAME=`cat /revoinfo/hostname | tr : /`
# FIXME : should also try using network stack (get_hostname)
HOSTNAME=`basename $HOSTNAME`
IPSERVER=$Next_server

export PATH=$PATH:/opt/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/lib

CHNTPWBIN=/opt/bin/chntpw

REGNUM=0

#
# Strip the 2 leading directories of a Win/DOS path
#
Strip2 ()
{
    echo $1 | cut -f 3- -d \\
}


#
# Find the Windows config directory
# return it in the CONFIGDIR variable
#
GetWinConfigDir ()
{
    # XP paths
    for i in /mnt/[wW][iI][nN]*/[Ss][Yy][Ss][Tt][Ee][Mm]32/[Cc][Oo][Nn][Ff][Ii][Gg]/[Ss][Oo][Ff][Tt][Ww][Aa][Rr][Ee]
    do
	if [ -d ${i%/*} -a -f $i ] ;then
	    CONFIGDIR=${i%/*}
	    SOFTWAREDIR=$i
	    return 0
	fi
    done
    CONFIGDIR=
    SOFTWAREDIR=
    echo "Could not find the registry..."
    return 1
}

#
# Add a key in the win registry
#
RegistryAddString ()
{
    KEY=$1
    NAM=$2
    VAL=$3

    Strip2 $KEY
    KEY2=`Strip2 $KEY`

    GetWinConfigDir

    # build the script that will be sent to chntpw
    SCRIPT="cd $KEY2
nv 1 $NAM
ed $NAM
$VAL
ls
q
y
"

    case $KEY in
    HKEY_LOCAL_MACHINE\\Software\\*)
	echo "*** modifying $KEY in $SOFTWAREDIR ***"
	echo "$SCRIPT" | $CHNTPWBIN -e $SOFTWAREDIR
	;;
    *)
	echo "*** modifying $KEY not yet supported ***"
	;;
    esac
}

#
# Add a key(s) in the win registry
#
RegistryAddKey ()
{
    KEY=$1
    shift
    SCRIPT=""

    Strip2 $KEY
    KEY2=`Strip2 $KEY`

    GetWinConfigDir

    # build the script that will be sent to chntpw
    SCRIPT="cd $KEY2"
    for D in $@
    do
	if [ -z $D ]; then
	    break
	fi
	SCRIPT="$SCRIPT
nk $D
cd $D"
    done
    SCRIPT="$SCRIPT
ls
q
y
"
    case $KEY in
    HKEY_LOCAL_MACHINE\\Software\\*)
	echo "*** modifying $KEY in $SOFTWAREDIR ***"
	echo "$SCRIPT" | $CHNTPWBIN -e $SOFTWAREDIR
	;;
    *)
	echo "*** modifying $KEY not yet supported ***"
	;;
    esac

}

#
# Modify the registry to run a program once after login (run as superuser)
# Example: RegistryAddRunOnce myprog.exe
#
RegistryAddRunOnce ()
{
    REGNUM=$(($REGNUM+1))
    RegistryAddString "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunOnce" LRS$REGNUM $1
}

RegistryAddRun ()
{
    REGNUM=$(($REGNUM+1))
    RegistryAddString "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run" LRS$REGNUM $1
}

RegistryAddRunServicesOnce ()
{
    # Win 2000. Not for XP.
    REGNUM=$(($REGNUM+1))
    RegistryAddString "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce" LRS$REGNUM $1
}

#
# Copy a sysprep configuration to file and substitute the hostname
# Example: CopySysprepInf /revoinfo/sysprep.inf
# If the extension is .xml it assumes that the target is Windows Vista/Seven/2008
# If the extension is .inf the file will be copied to c:\Sysprep.inf (Windows XP)
#
CopySysprepInf ()
{
    SYSPREP_FILE=$1

    # Warning ! There's a ^M after $HOSTNAME for DOS compatibility
    SYSPREP=sysprep
    [ -d /mnt/Sysprep ] && SYSPREP=Sysprep

    if [ ${SYSPREP_FILE#*.} == "xml" ]; then
        rm -f /mnt/Windows/System32/sysprep/*.xml
        sed -e "s/<ComputerName>.*$/<ComputerName>${HOSTNAME}<\/ComputerName>"`echo -e "\015"`"/" < $SYSPREP_FILE > /mnt/Windows/Panther/unattend.xml
    fi

    if [ ${SYSPREP_FILE#*.} == "inf" ]; then
        rm -f /mnt/$SYSPREP/[Ss]ysprep.inf
        sed -e "s/^[	]*[Cc]omputer[Nn]ame[^\n\r]*/ComputerName=$HOSTNAME"`echo -e "\015"`"/" < $SYSPREP_FILE >/mnt/$SYSPREP/Sysprep.inf
    fi
}

#
# Return the name of the Nth partition
#
GetNPart ()
{
    I=1
    C4=""
    # read /proc/partitions and find the Nth entry
    while [ $I -le $1 ] ;do
	read C1 C2 C3 C4 C5 || break
	if echo "$C4"|grep -q "^[a-z]*[0-9]$"; then
	    I=$(($I+1))
	fi
    done < /proc/partitions
    [ "$C4" ] && echo /dev/$C4
}

#
# Get the start sector for partition NUM on disk DISK
#
GetPartStart ()
{
    DISK=${1}
    NUM=${2}

    FS=`parted -s $DISK unit s print | grep "^[[:space:]]*${NUM}[[:space:]]\+" | sed 's/^\s\+//g' | sed 's/  */,/g' | cut -f 2 -d ,`
    echo ${FS}

}

#
# Get the filesystem type for partition NUM on disk DISK
#
GetPartFileSystem ()
{
    DISK=${1}
    NUM=${2}

    L=`parted -s $DISK unit s print | grep "^[[:space:]]*${NUM}[[:space:]]\+" | sed 's/^\s\+//g' | sed 's/  */,/g' | cut -f 6 -d ,`
    echo ${L}

}

#
# Return "yes" if the partition is bootable
#
IsPartBootable ()
{
    DISK=$1
    NUM=$2

    parted -s $DISK print | grep "^[[:space:]]*${NUM}[[:space:]]\+" | grep -q boot && echo "yes"
}

#
# Set the boot partition flag
#
SetPartBootable ()
{
    DISK=$1
    NUM=$2

    parted -s $DISK set $NUM boot on
}

#
# Resize the Nth partition
#
Resize ()
{
    NUM=$1
    SZ=$2

    P=`GetNPart $NUM`
    D=`PartToDisk $P`
    S=`GetPartStart $D $NUM`
    FS=`GetPartFileSystem $D $NUM`
    BOOT=`IsPartBootable $D $NUM`
    if [ -z $P ]; then
      echo "*** ERROR: Partition number is empty. Aborting resize."
      return 1
    elif [ -z $D ]; then
      echo "*** ERROR: Unable to find disk corresponding to partition ${P}. Aborting resize."
      return 1
    elif [ -z $S ]; then
      echo "*** ERROR: Unable to get start sector of partition ${P}. Aborting resize."
      return 1
    elif [ -z $FS ]; then
      echo "*** ERROR: Unable to identify filesystem of partition ${FS}. Aborting resize."
      return 1
    elif [ -z $BOOT ]; then
      echo "*** ERROR: Unable to figure out if partition ${FS} is bootable or not. Aborting resize."
      return 1
    else
      if [ "$FS" = "ntfs" ]; then
        parted -s $D rm $NUM mkpart primary ntfs $S $SZ
        [ "$BOOT" = "yes" ] && SetPartBootable $D $NUM
        yes|ntfsresize -f $P
        ntfsresize --info --force $P
      else
        parted -s $D resize $NUM $S $SZ
      fi
    fi
}

#
# Maximize the Nth partition
#
ResizeMax ()
{
    Resize $1 100%
}

#
# return the disk device related to the part device
# /dev/hda1 -> /dev/hda
#
PartToDisk ()
{
    echo $1|
    sed 's/[0-9]*$//'
}

#
# Mount the target device as /mnt
#
Mount ()
{
  # Check if parameter is a real number
  if echo "${1}" | grep -q "^[0-9]\+" ;then
    # Check if ${1} is lower or equal than numbers of partitions and not zero
    partnumber=`grep '[a-z]\+[0-9]\+$' /proc/partitions | wc -l | sed 's/ //g'`
    if [ ${1} -le ${partnumber} ] && [ ${1} -gt 0 ]; then
      # Get partition name according to it's number
      partname=`grep '[a-z]\+[0-9]\+$' /proc/partitions | head -n ${1} | tail -n 1 | awk '{print $NF}'`
      # Looks being a real block device ?
      if [ -b /dev/${partname} ]; then
        mountdisk /dev/${partname}
      else
        echo "*** ERROR: partition number ${1} (resolved as ${partname}) not found"
        return 1
      fi
    else
      echo "*** ERROR: partition number ${1} invalid (from 1 to ${partnumber})"
      return 1
    fi
  else
    echo "*** ERROR: Invalid partition number (${1})"
    return 1
  fi
}

#
# Try to find and mount the "system" disk
#
MountSystem ()
{
  # Number of partitions
  partnumber=`grep '[a-z]\+[0-9]\+$' /proc/partitions | wc -l | sed 's/ //g'`
  for num in `seq 1 ${partnumber}`; do
    Mount ${num}
    # Does it looks like being a Windows ?
    if [ -d /mnt/WINDOWS ]; then
      echo "*** INFO: WINDOWS found on partition number ${num}"
      return 0
    # Or some Unix disk ?
    elif [ -d /mnt/bin ] && [ -d /mnt/etc ] && [ -d /mnt/var ] && [ -d /mnt/home ]; then
      echo "*** INFO: Unix found on partition number ${num}"
      return
    fi
  done
  # Got there ? Nothing found...
  echo "*** ERROR: Unable to find a system disk"
  umount /mnt >/dev/null 2>&1
  return 1
}

#
# newsid.exe based commands
#
ChangeSID ()
{
    mkdir /mnt/tmp
    unix2dos <<EOF >/mnt/tmp/newsid.bat
\\tmp\\newsid.exe /a
EOF
    chmod 755 /mnt/tmp/newsid.bat
    cp -f /opt/winutils/newsid.exe /mnt/tmp/
    RegistryAddRunOnce '\tmp\newsid.bat'
}

ChangeSIDAndName ()
{
    mkdir /mnt/tmp
    unix2dos <<EOF >/mnt/tmp/newsid.bat
\\tmp\\newsid.exe /a /d 30 $HOSTNAME
EOF
    chmod 755 /mnt/tmp/newsid.bat
    cp -f /opt/winutils/newsid.exe /mnt/tmp/
    RegistryAddRunOnce '\tmp\newsid.bat'
}
