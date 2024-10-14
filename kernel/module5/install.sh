#!bin /bin/bash

# sudo rmmod -f gpiotimer_module
sudo insmod gpiotimer_module.ko
sudo mknod /dev/gpioled c 200 0
sudo chmod 666 /dev/gpioled

dmesg | tail
