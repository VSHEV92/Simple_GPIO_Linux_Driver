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
	struct my_gpio_instance *ip = gpiochip_get_data(gc);


	pr_info("Set direction of line %d to output and value to %d\n", gpio, value);
	
	ip->dir = ip->dir | (1<<gpio);
	writel(ip->dir, ip->regs + GPIO_DIR_OFFSET);

	return 0;
}

/**
 * установить gpio line как вход
 */
static int mygpio_dir_in(struct gpio_chip *gc, unsigned int gpio){	
	struct my_gpio_instance *ip = gpiochip_get_data(gc);

	pr_info("Set direction of line %d to input\n", gpio);

	ip->dir = ip->dir & ~(1<<gpio);
	writel(ip->dir, ip->regs + GPIO_DIR_OFFSET);

	return 0;
}

/**
 * установить значение gpio line
 */
static void mygpio_data_set(struct gpio_chip *gc, unsigned int gpio, int value){
	struct my_gpio_instance *ip = gpiochip_get_data(gc);

	pr_info("Set line %d to value %d\n", gpio, value);

	if (value)
		ip->data = ip->data | (1<<gpio);
	else
		ip->data = ip->data & ~(1<<gpio);

	writel(ip->data, ip->regs + GPIO_DATA_OFFSET);
}

/**
 * считать значение gpio line
 */
static int mygpio_data_get(struct gpio_chip *gc, unsigned int gpio){
	struct my_gpio_instance *ip = gpiochip_get_data(gc);
	
	pr_info("Get line %d value\n", gpio);

	return readl(ip->regs + GPIO_DATA_OFFSET);
}

/**
 * добавление модуля в ядро
 */
static int my_xlnx_gpio_probe(struct platform_device *pdev){
	int status = 0;
	struct resource *res;
	struct my_gpio_instance *ip_core;
	struct device_node *np = pdev->dev.of_node;
	
	dev_info(&pdev->dev, "Module Probe Function\n");

	// получить ресурс с адресами регистров
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "No resource defined\n");
		return -EBUSY;
	}

	// выделение памяти для структуры Xilinx GPIO
	ip_core = devm_kzalloc(&pdev->dev, sizeof(*ip_core), GFP_KERNEL);
	if (!ip_core)
		return -ENOMEM;
	platform_set_drvdata(pdev, ip_core);

	// получить число gpio line
	if (of_property_read_u32(np, "gpio-width", &ip_core->gpio_width))
		dev_dbg(&pdev->dev, "Missing pio_width property\n");

	// получить адрес ядра Xilinx GPIO
	ip_core->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ip_core->regs)) {
		dev_err(&pdev->dev, "Failed to ioremap memory resource\n");
		return PTR_ERR(ip_core->regs);
	}
	
	// получения источника тактового сигнала
    ip_core->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(ip_core->clk))
		return PTR_ERR(ip_core->clk);

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
		clk_disable_unprepare(ip_core->clk);
		return status;
	}

	// устанавливаем начальные значения
	ip_core->dir = 0;
	ip_core->data = 0;
	writel(ip_core->dir, ip_core->regs + GPIO_DIR_OFFSET);
	writel(ip_core->data, ip_core->regs + GPIO_DATA_OFFSET);

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
