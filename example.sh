#!/bin/bash

. `dirname $0`/functions.inc

#f_hostname somehostname

### networking - f_network_static <interface> <ip> <netmask> <gateway> <dns server>
#f_network_static eth0 192.168.1.200 255.255.255.0 192.168.1.1 8.8.8.8
#f_network_static eth0 0.0.0.0 "" "" ""
#f_network_static eth1 192.168.2.200 255.255.255.0 "" ""
#f_network_dhcp eth0
#f_network_bridge_static br0 eth1 192.168.3.200 255.255.255.0 "" ""
#f_service_restart networking

### ssh
#f_service_enable ssh
#f_ssh_rootlogin_deny
#f_ssh_port 2222
#f_ssh_resetkeys

### kvm / qemu
#f_kvm_nested_enable
#f_service_enable libvirt-guests
#f_service_enable libvirt-bin

### users
#f_useradd guest ahardpassword
#f_usergroup guest sudo

#f_rebootnow

