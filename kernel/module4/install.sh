#!bin /bin/bash

sudo rmmod -f gpioirq_module
sudo insmod gpioirq_module.ko
sudo mknod /dev/gpioled c 200 0
sudo chmod 666 /dev/gpioled

dmesg | tail
