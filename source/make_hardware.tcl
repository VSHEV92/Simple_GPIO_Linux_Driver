# создание проекта
create_project Simple_GPIO_Driver_HW Simple_GPIO_Driver_HW -part xc7z020clg400-1
set_property board_part www.digilentinc.com:pynq-z1:part0:1.0 [current_project]

# создание block design
create_bd_design "Simple_GPIO_HW"

# добавление zynq
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" apply_board_preset "1" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]

# добавление gpio
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0
set_property -dict [list CONFIG.C_GPIO_WIDTH {4} CONFIG.GPIO_BOARD_INTERFACE {leds_4bits}] [get_bd_cells axi_gpio_0]

# подключение ядер
apply_bd_automation -rule xilinx.com:bd_rule:board -config { Board_Interface {leds_4bits ( 4 LEDs ) } Manual_Source {Auto}}  [get_bd_intf_pins axi_gpio_0/GPIO]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config { Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} Master {/processing_system7_0/M_AXI_GP0} Slave {/axi_gpio_0/S_AXI} ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}}  [get_bd_intf_pins axi_gpio_0/S_AXI]

# проверка и созранение block design
validate_bd_design
regenerate_bd_layout
save_bd_design
close_bd_design [get_bd_designs Simple_GPIO_HW]

# создание hdl_wrapper
make_wrapper -files [get_files Simple_GPIO_Driver_HW/Simple_GPIO_Driver_HW.srcs/sources_1/bd/Simple_GPIO_HW/Simple_GPIO_HW.bd] -top
add_files -norecurse Simple_GPIO_Driver_HW/Simple_GPIO_Driver_HW.srcs/sources_1/bd/Simple_GPIO_HW/hdl/Simple_GPIO_HW_wrapper.v
update_compile_order -fileset sources_1

# запуск синтеза 
launch_runs synth_1 -jobs 6
wait_on_run synth_1

# запуск имплементации
launch_runs impl_1 -jobs 6
wait_on_run impl_1

# генерация bitstream
launch_runs impl_1 -to_step write_bitstream -jobs 6
wait_on_run impl_1

# создание xsa файла
write_hw_platform -fixed -force  -include_bit -file Simple_GPIO_HW.xsa