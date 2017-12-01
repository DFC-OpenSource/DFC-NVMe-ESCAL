#!/bin/bash

sudo rmmod nvme
sudo rmmod nvme-core
sudo insmod nvme-core.ko
sudo insmod nvme.ko

