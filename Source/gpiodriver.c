#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/gpio/driver.h>
#include <linux/clk.h>

#define GPIO_DATA_OFFSET   (0x0)
#define GPIO_DIR_OFFSET    (0x4)

/**
 * Структура с описанием параметров ядра Xilinx GPIO
 */
struct my_gpio_instance {
	struct gpio_chip gc;
	struct clk *clk;
	void __iomem *regs;
	u32 gpio_width;
	u32 data;
	u32 dir;
};

/**
 * установить gpio line как выход
 */
static int mygpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int value){
	printk("Set direction of line %d to output and value to %d\n", gpio, value);
	return 0;
}

/**
 * установить gpio line как вход
 */
static int mygpio_dir_in(struct gpio_chip *gc, unsigned int gpio){
	printk("Set direction of line %d to input\n", gpio);
	return 0;
}

/**
 * установить значение gpio line
 */
static void mygpio_data_set(struct gpio_chip *gc, unsigned int gpio, int value){
	struct my_gpio_instance *ip = gpiochip_get_data(gc);

	printk("Set line %d to value %d\n", gpio, value);

	if (value)
		writel(0xFFFF, ip->regs + GPIO_DATA_OFFSET);
	else
		writel(0x0000, ip->regs + GPIO_DATA_OFFSET);
}

/**
 * считать значение gpio line
 */
static int mygpio_data_get(struct gpio_chip *gc, unsigned int gpio){
	u32 val;
	struct my_gpio_instance *ip = gpiochip_get_data(gc);
	
	printk("Get line %d value\n", gpio);

	val = readl(ip->regs + GPIO_DATA_OFFSET);

	return 0;
}

/**
 * добавление модуля в ядро
 */
static int my_xlnx_gpio_probe(struct platform_device *pdev){
	int status = 0;
	struct my_gpio_instance *ip_core;
	struct device_node *np = pdev->dev.of_node;
	
	dev_info(&pdev->dev, "Module Probe Function\n");

	// выделение памяти для структуры Xilinx GPIO
	ip_core = devm_kzalloc(&pdev->dev, sizeof(*ip_core), GFP_KERNEL);
	if (!ip_core)
		return -ENOMEM;
	platform_set_drvdata(pdev, ip_core);

	// получить число gpio line
	if (of_property_read_u32(np, "gpio-width", &ip_core->gpio_width))
		dev_dbg(&pdev->dev, "Missing pio_width property\n");

	// получить адрес ядра Xilinx GPIO
	ip_core->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(ip_core->regs)) {
		dev_err(&pdev->dev, "Failed to ioremap memory resource\n");
		return PTR_ERR(ip_core->regs);
	}
	
	// получения источника тактового сигнала
    ip_core->clk = devm_clk_get_optional(&pdev->dev, NULL);
	if (IS_ERR(ip_core->clk))
		return dev_err_probe(&pdev->dev, PTR_ERR(ip_core->clk), "Input clock not found.\n");

	// включение тактового сигнала
	status = clk_prepare_enable(ip_core->clk);
	if (status < 0) {
		dev_err(&pdev->dev, "Failed to prepare clk\n");
		return status;
	}

	// инициализация полей струкуры gpio_chip
	ip_core->gc.base = -1;                         // назначить начальный номер gpio line автоматически
	ip_core->gc.ngpio = ip_core->gpio_width;       
	ip_core->gc.label = "my_xlnx_gpio"; 
	ip_core->gc.direction_input = mygpio_dir_in;
	ip_core->gc.direction_output = mygpio_dir_out;
	ip_core->gc.get = mygpio_data_get;
	ip_core->gc.set = mygpio_data_set;
	
	// добавляем ядро в список gpio устройств
	status = devm_gpiochip_add_data(&pdev->dev, &ip_core->gc, ip_core);
	if (status) {
		dev_err(&pdev->dev, "Failed to add GPIO chip\n");
		return status;
	}

	return 0;
}

/**
 * удаление модуля из ядра
 */
static int my_xlnx_gpio_remove(struct platform_device *pdev){
	struct my_gpio_instance *ip = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "Module Remove Function\n");
	clk_disable_unprepare(ip->clk);
    return 0;
}

/**
 * таблица совместимости драйвера
 */
static const struct of_device_id my_xlnx_gpio_of_match[] = {
	{.compatible = "vshev92,my_xlnx_gpio"},
	{}
};
MODULE_DEVICE_TABLE(of, my_xlnx_gpio_of_match);

/**
 * простой драйвер для управления ядро Xilinx GPIO 
 */
static struct platform_driver my_xlnx_gpio_driver = {
	.driver = {
		.name = "my_xlnx_gpio_driver",
		.of_match_table = my_xlnx_gpio_of_match,
	},
	.probe = my_xlnx_gpio_probe,
	.remove = my_xlnx_gpio_remove,
};
module_platform_driver(my_xlnx_gpio_driver);

MODULE_AUTHOR("VSHEV92");
MODULE_DESCRIPTION("Simple driver for Xilinx GPIO IP Core");
MODULE_LICENSE("GPL");
