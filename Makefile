all: Simple_GPIO_HW.xsa

Simple_GPIO_HW.xsa:
	vivado -mode batch -source source/make_hardware.tcl

clean:
	rm vivado*
	rm -Rf NA
	rm -Rf .Xil
	rm -Rf Simple_GPIO_Driver_HW
	rm Simple_GPIO_HW.xsa

