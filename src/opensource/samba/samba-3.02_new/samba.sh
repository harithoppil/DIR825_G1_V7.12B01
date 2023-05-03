#!/bin/sh

SAMBA_FILE=/etc/smb.conf

if [ ! -n "$2" ]; then
	echo "insufficient arguments!"
	echo "Usage: $0 <netbios_name> <workgroup>"
	echo "Example: $0 RT2880 Ralink"
	exit 0
fi

NETBIOS_NAME="$1"
WORKGROUP="$2"

echo "[global]
netbios name = $NETBIOS_NAME
server string = Samba Server
workgroup = $WORKGROUP
security = user
guest account = guest
log file = /var/log.samba
socket options = TCP_NODELAY SO_RCVBUF=16384 SO_SNDBUF=8192
encrypt passwords = yes
use spne go = no
client use spnego = no
disable spoolss = yes
smb passwd file = /etc/smbpasswd
host msdfs = no
interfaces = lo br0 eth2.1 eth2.2
strict allocate = No
os level = 20
log level = 3
max log size = 100
null passwords = yes
mangling method = hash
dos charset = CP850
unix charset = UTF8
display charset = UTF8
bind interfaces only = yes
" > $SAMBA_FILE

