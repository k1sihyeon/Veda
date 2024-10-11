#!bin /bin/bash

sudo rmmod -f gpio_module
sudo insmod gpio_module.ko
sudo mknod /dev/gpioled c 200 0
sudo chmod 666 /dev/gpioled
