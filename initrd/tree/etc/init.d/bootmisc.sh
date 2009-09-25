#!/bin/sh

. /etc/default/rcS
# Put a nologin in /etc to prevent logging in before system startup complete.
if [ "$DELAYLOGIN" = yes ]
then
  echo "System bootup in progress - please wait" >/etc/nologin
  cp /etc/nologin /etc/nologin.boot
fi

# devfs
if [ ! -r /dev/tty0 ]
then
  cd /dev
  ln -s vc/0 tty0
  ln -s vc/1 tty1
  ln -s vc/2 tty2
  ln -s vc/3 tty3
fi

# Set pseudo-terminal access permissions.
#chmod 666  /dev/tty[p-za-e][0-9a-f]
#chown root /dev/tty[p-za-e][0-9a-f]

# issue
cut -f -3 -d " " < /proc/version >/etc/issue
cp /etc/issue /etc/issue.net
