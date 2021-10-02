SHELL := /bin/bash

xsa: Simple_GPIO_HW.xsa
	source /opt/PetaLinux/settings.sh
	petalinux-create --type project --template zynq --name petaproject
	cd petaproject;	petalinux-config --get-hw-description .. --silentconfig

Simple_GPIO_HW.xsa:
	vivado -mode batch -source source/make_hardware.tcl

clean_peta:
	rm -Rf petaproject

clean:
	make clean_peta
	rm vivado*
	rm -Rf NA
	rm -Rf .Xil
	rm -Rf Simple_GPIO_Driver_HW
	rm Simple_GPIO_HW.xsa

