#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/driver.h>

struct gnss_regulator {
    struct regulator_dev *rdev;
    struct gpio_desc *gpiod;
};

static int gnss_regulator_enable(struct regulator_dev *rdev)
{
    struct gnss_regulator *data = rdev_get_drvdata(rdev);
    
    gpiod_set_value_cansleep(data->gpiod, 1);
    return 0;
}

static int gnss_regulator_disable(struct regulator_dev *rdev)
{
    struct gnss_regulator *data = rdev_get_drvdata(rdev);
    
    gpiod_set_value_cansleep(data->gpiod, 0);
    return 0;
}

static int gnss_regulator_is_enabled(struct regulator_dev *rdev)
{
    struct gnss_regulator *data = rdev_get_drvdata(rdev);
    
    return gpiod_get_value_cansleep(data->gpiod);
}

static const struct regulator_ops gnss_regulator_ops = {
    .enable = gnss_regulator_enable,
    .disable = gnss_regulator_disable,
    .is_enabled = gnss_regulator_is_enabled,
};

static const struct regulator_desc gnss_desc = {
    .name = "gnss-3v3",
    .id = -1,
    .ops = &gnss_regulator_ops,
    .type = REGULATOR_VOLTAGE,
    .owner = THIS_MODULE,
};

static int gnss_regulator_probe(struct platform_device *pdev)
{
    struct regulator_config config = {0};
    struct gnss_regulator *data;
    int ret;

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_HIGH);
    if (IS_ERR(data->gpiod)) {
        dev_err(&pdev->dev, "Failed to get GPIO\n");
        return PTR_ERR(data->gpiod);
    }

    config.dev = &pdev->dev;
    config.driver_data = data;
    config.of_node = pdev->dev.of_node;

    data->rdev = devm_regulator_register(&pdev->dev, &gnss_desc, &config);
    if (IS_ERR(data->rdev)) {
        dev_err(&pdev->dev, "Failed to register regulator\n");
        return PTR_ERR(data->rdev);
    }

    return 0;
}

static const struct of_device_id gnss_regulator_of_match[] = {
    { .compatible = "regulator-fixed", },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, gnss_regulator_of_match);

static struct platform_driver gnss_regulator_driver = {
    .probe = gnss_regulator_probe,
    .driver = {
        .name = "gnss-regulator",
        .of_match_table = gnss_regulator_of_match,
    },
};

module_platform_driver(gnss_regulator_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Custom GNSS 3.3V Regulator Driver");