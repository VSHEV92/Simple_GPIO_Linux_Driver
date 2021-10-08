### Простой драйвер для Xilinx GPIO AXI

------

Пример простого Linux platform драйвера на примере IP-ядра Xilinx GPIO AXI и petalinux длч платы PYNQ Z1

------

##### Makefile:

- **bsp**: собрать проект petalinux из bsp-файла
- **xsa**: собрать проект petalinux из xsa-файла и исходников
- **update_source**: обновить исходники проекта petalinux 
- **Simple_GPIO_HW.xsa**: создать проект Vivado и xsa-файл
- **clean_peta**: очистить проект petalinux
- **clean**: очистить проекты Vivado и petalinux 

При сборке проекта из xsa-файла нужно вручную добавить библитеку libgpiod в rootfs!

------

##### Содержимое папки source

- **make_hardware.tcl** - скрипт для создания Vivado проекта и xsa-файла
- **system-user.dtsi** - модернизация device-tree для подключения собственного драйвера
- **gpiodriver.c** - исходный код GPIO драйвера
- **ledstoggle.c** -  приложение для проверки драйвера
- **ledstoggle.bb** -  рецепт yocto для сборки приложения

------

