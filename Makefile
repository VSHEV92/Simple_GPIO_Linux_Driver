SHELL := /bin/bash

# собрать проект из bsp-файла
bsp:

# собрать проект из xsa-файла и исходников
xsa: Simple_GPIO_HW.xsa
	source /opt/PetaLinux/settings.sh; \
	petalinux-create --type project --template zynq --name petaproject; \
	cd petaproject; \
	petalinux-config --get-hw-description .. --silentconfig; \
	petalinux-create -t apps --template c --name ledstoggle --enable; \
	petalinux-create -t modules --name gpiodriver --enable
	make update_source


# обновить исходники проекта petalinux
update_source:
	cp source/system-user.dtsi petaproject/project-spec/meta-user/recipes-bsp/device-tree/files
	cp source/gpiodriver.c petaproject/project-spec/meta-user/recipes-modules/gpiodriver/files
	cp source/ledstoggle.c petaproject/project-spec/meta-user/recipes-apps/ledstoggle/files
	cp source/ledstoggle.bb petaproject/project-spec/meta-user/recipes-apps/ledstoggle

# создать проект Vivado и xsa-файл
Simple_GPIO_HW.xsa:
	vivado -mode batch -source source/make_hardware.tcl

# очистить проект petalinux
clean_peta:
	rm -Rf petaproject

# очистить все
clean:
	make clean_peta
	rm vivado*
	rm -Rf NA
	rm -Rf .Xil
	rm -Rf Simple_GPIO_Driver_HW
	rm Simple_GPIO_HW.xsa

