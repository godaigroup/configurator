#!/bin/bash

###############################################################
#
# requirements:
#   expect / sed / awk
#
# notes:
#  - this is all permanent configuration settings.
#    there are no termporary settings configured here.
#    so a reboot is needed in order for the changes to be active
#    some things might work if doing a service restart
#
# todo:
#  - need detection script on network functions if interface definition there already
#  - need detection script on kvm nested function
#  - determine if can use this script for builder?
#       they are somewhat the same thing but not really b/c one is for the global livecd and another is for installed instances
#
###############################################################
#
# functions available:
#   f_hostname <name>
#
#   f_network_static "<interface>" "<ip>" "<netmask>" "<gateway>" "<dns server>"
#   f_network_dhcp "<interface>"
#   f_network_bridge_static "<bridge name>" "<interface>" "<ip>" "<netmask>" "<gateway>" "<dns server>"
#
#   f_service_enable <service name>
#   f_service_disable <service name>
#   f_service_start <service name>
#   f_service_stop <service name>
#   f_service_restart <service name>
#   f_ssh_rootlogin_allow
#   f_ssh_rootlogin_deny
#   f_ssh_emptypass_allow
#   f_ssh_emptypass_deny
#   f_ssh_resetkeys
#   f_ssh_port <port>
#   f_kvm_nested_enable
#
#   f_login_emptypasswd_allow
#   f_login_emptypasswd_deny
#
#   f_useradd <user> <password>
#   f_user_emptypass <user>
#   f_user_shell <user> <shell path>
#   f_user_redirectshell <user> "<ssh options>" "<user@host>"
#   f_usergroup <user> <group>
#   f_user_hushlogin <user>
#
#   f_rebootnow
#
###############################################################

PATH=/bin:/sbin:/usr/bin:/usr/sbin

function f_hostname {
 echo ' - setting hostname: ' $1
 echo $1 > /etc/hostname
 sed -i '/^127\.0\.0\.1/ s/$/ '$1'/' /etc/hosts
}

function f_network_static {
echo "
auto $1
iface $1 inet static
  address $2" >> /etc/network/interfaces

if [ -n "$3" ];
then
 echo "  netmask $3" >> /etc/network/interfaces;
else
 echo "  #netmask x.x.x.x" >> /etc/network/interfaces;
fi

if [ -n "$4" ];
then
 echo "  gateway $4" >> /etc/network/interfaces;
else
 echo "  #gateway x.x.x.x" >> /etc/network/interfaces;
fi

if [ -n "$5" ];
then
 echo "  dns-nameservers $5" >> /etc/network/interfaces;
else
 echo "  #dns-nameservers x.x.x.x" >> /etc/network/interfaces;
fi
}

function f_network_dhcp {
echo "
auto $1
iface $1 inet dhcp" >> /etc/network/interfaces
}

function f_network_bridge_static {
echo "
auto $1
iface $1 inet static
  address $3" >> /etc/network/interfaces

if [ -n "$4" ];
then
 echo "  netmask $4" >> /etc/network/interfaces;
else
 echo "  #netmask x.x.x.x" >> /etc/network/interfaces;
fi

if [ -n "$5" ];
then
 echo "  gateway $5" >> /etc/network/interfaces;
else
 echo "  #gateway x.x.x.x" >> /etc/network/interfaces;
fi

if [ -n "$6" ];
then
 echo "  dns-nameservers $6" >> /etc/network/interfaces;
else
 echo "  #dns-nameservers x.x.x.x" >> /etc/network/interfaces;
fi

echo "  bridge_ports $2"  >> /etc/network/interfaces
echo "  bridge_stp on
  bridge_maxwait 0" >> /etc/network/interfaces
}

function f_service_enable {
 echo ' - enable service: ' $1
 update-rc.d $1 enable
}

function f_service_disable {
echo ' - disable service: ' $1
 update-rc.d $1 disable
}

function f_service_start {
echo ' - starting service: ' $1
service $1 start
}

function f_service_stop {
echo ' - stopping service: ' $1
service $1 stop
}

function f_service_restart {
echo ' - restarting service: ' $1
service $1 restart
}

function f_ssh_rootlogin_allow {
sed -i "s/PermitRootLogin.*/PermitRootLogin yes/" /etc/ssh/sshd_config
}

function f_ssh_rootlogin_deny {
sed -i "s/PermitRootLogin.*/PermitRootLogin no/" /etc/ssh/sshd_config
}

function f_ssh_emptypass_allow {
sed -i "s/PermitEmptyPasswords .*/PermitEmptyPasswords yes/" /etc/ssh/sshd_config
}

function f_ssh_emptypass_deny {
sed -i "s/PermitEmptyPasswords .*/PermitEmptyPasswords no/" /etc/ssh/sshd_config
}

function f_ssh_port {
sed -i "s/Port.*/Port $1/" /etc/ssh/sshd_config
}

function f_ssh_resetkeys {
rm -f /etc/ssh/ssh_host_*
dpkg-reconfigure openssh-server
}

function f_login_emptypasswd_allow {
sed -i "s/nullok_secure$/nullok/" /etc/pam.d/common-auth
}

function f_login_emptypasswd_deny {
sed -i "s/nullok$/nullok_secure/" /etc/pam.d/common-auth
}

function f_kvm_nested_enable {
echo "options kvm-intel nested=1" >> /etc/modprobe.d/dist.conf
}

function f_useradd {
echo ' - adding user: ' $1
USER=$1
PASS=$2
expect <<EOF
spawn adduser $USER
expect "UNIX password:"
send "$PASS\r"
expect "UNIX password:"
send "$PASS\r"
expect "[]:"
send "\r"
expect "[]:"
send "\r"
expect "[]:"
send "\r"
expect "[]:"
send "\r"
expect "[]:"
send "\r"
expect "information correct?"
send "\r"
expect eof
exit
EOF
}

function f_user_emptypass {
echo ' - empty password for user: ' $1
usermod -p "" $1
}

function f_user_shell {
echo ' - new shell for : ' $1
echo ' -            to : ' $2
#usermod -s shell user - but to kept syntax with others
usermod -s $2 $1
}

function f_user_redirectshell {
echo ' - redirect shell for : ' $1
echo ' -             ssh to : ' $3
echo ' -            options : ' $2
VART=`cat /etc/passwd | grep ^$1 | cut -d":" -f6`
TSRC=$VART/$1_redirectshell.c
TBIN=$VART/$1_redirectshell
if [ -e "$DEST" ]
then
  rm -f $DEST
fi
cp -f redirectshell.c $TSRC
sed -i "s/ssh /ssh $2/" $TSRC
sed -i "s/user@host/$3/" $TSRC
cc -o $TBIN $TSRC
chmod 755 $TBIN
chown $1:$1 $TBIN
chown $1:$1 $TSRC
f_user_shell $1 $TBIN
}

function f_user_hushlogin {
echo ' - hushlogin for : ' $1
VART=`cat /etc/passwd | grep ^$1 | cut -d":" -f6`
touch $VART/.hushlogin
chown $1:$1 $VART/.hushlogin
}

function f_usergroup {
adduser $1 $2
}

function f_rebootnow {
shutdown -r now
}
