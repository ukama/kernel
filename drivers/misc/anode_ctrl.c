 #include <linux/err.h>
 #include <linux/module.h>
 #include <linux/types.h>
 #include <linux/platform_device.h>
 #include <linux/io.h>
 #include <linux/of_gpio.h>
 #include <linux/of.h>
 #include <linux/sysfs.h>

struct anode_ctrl_priv
{
    struct device *dev;
	
    int eeprom_wp_enable_gpio;
	char eeprom_wp_enable_szGpio[32];
	u8 eeprom_wp_enable_val;
	u32 eeprom_wp_enable_default;

    int psu_pgood_gpio;
    char psu_pgood_szGpio[32];
    u8 psu_pgood_val;
    u32 psu_pgood_default;
    
    int ctrlr_therm_alert_gpio;
    char ctrlr_therm_alert_szGpio[32];
    u8 ctrlr_therm_alert_val;
    u32 ctrlr_therm_alert_default;
};

static ssize_t show_ctrlr_therm_alert(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_ctrl_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "Controller thermal alert is in %s state.", ((priv->ctrlr_therm_alert_val)==0)?"active":"inactive");
    return sprintf(buf, "%d\n", priv->ctrlr_therm_alert_val);
}

static ssize_t show_psu_pgood_stat(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_ctrl_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "PSU powergood  status is in %s state.", ((priv->rf_therm_alert_val)==0)?"bad":"good");
    return sprintf(buf, "%d\n", priv->psu_pgood_val);
}
	
static ssize_t show_ctrlr_eeprom_wp(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct anode_ctrl_priv *priv = dev_get_drvdata(dev);
	dev_info(dev, "Controller EEPROM write protect is %s.", ((priv->eeprom_wp_enable_val)==0)?"disabled":"enabled");
	return sprintf(buf, "%d\n", priv->eeprom_wp_enable_val);
}

static ssize_t set_ctrlr_eeprom_wp(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct anode_ctrl_priv *priv = dev_get_drvdata(dev);
	u32 val;
	ssize_t ret;

	ret = kstrtouint(buf, 0, &val);
	if (ret)
		return ret;

	priv->eeprom_wp_enable_val = val ? 1 : 0;

	gpio_set_value_cansleep(priv->eeprom_wp_enable_gpio, priv->eeprom_wp_enable_val);
	dev_info(dev, "Controller EEPROM write protect is %s now.", ((priv->eeprom_wp_enable_val)==0)?"disabled":"enabled");
	return count;
}

static DEVICE_ATTR(ctrlr_therm_alert, S_IRUGO, show_ctrlr_therm_alert, NULL);
static DEVICE_ATTR(psu_pgood, S_IRUGO, show_psu_pgood_stat, NULL);
static DEVICE_ATTR(ctrlr_eeprom_wp, S_IWUSR | S_IRUGO, show_ctrlr_eeprom_wp, set_ctrlr_eeprom_wp);

static struct attribute *anode_ctrl_attrs[] = {
	&dev_ctrlr_therm_alert.attr,
	&dev_psu_pgood.attr,
	&dev_attr_ctrlr_eeprom_wp.attr,
	NULL,
};

static const struct attribute_group anode_ctrl_attr_group = {
	.attrs = anode_ctrl_attrs,
};

int anode_ctrl_parse_dt(struct platform_device *pdev)
{
	int ret = 0; 
	struct device_node *np = pdev->dev.of_node;
	struct anode_ctrl_priv *priv = dev_get_drvdata(&pdev->dev);
	if (!np) {
		return -EINVAL;
	}
	
	/* controller thermal alert */
	priv->ctrlr_therm_alert_gpio = of_get_named_gpio(np, "controller-therm-alert", 0);
	if (priv->ctrlr_therm_alert_gpio < 0) {
		dev_err(&pdev->dev, "Can't read gpio ctrlr_therm_alert_gpio\n");
		return -EINVAL;
	}

    /* controller power good status */ 
    priv->ctrlr_psu_pgood_gpio = of_get_named_gpio(np, "psu-pgood", 0);
    if (priv->ctrlr_psu_pgood_gpio < 0) {
        dev_err(&pdev->dev, "Can't read gpio psu-pgood.\n");
        return -EINVAL;
    }     

	/* Controller board EEPROM write protect */
	priv->ctrlr_eeprom_wp_enable_gpio = of_get_named_gpio(np, "eeprom-wp-enable", 0);
	if (priv->ctrlr_eeprom_wp_enable_gpio < 0) {
		dev_err(&pdev->dev, "Can't read Controller board gpio eeprom-wp-enable\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "eeprom-wp-enable-default", &priv->ctlr_eeprom_wp_enable_default);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't read Controller EEPROM write protect default value from device tree.\n");
		return ret;
	}

	priv->ctrlr_eeprom_wp_enable_val = priv->ctrlr_eeprom_wp_enable_default;
	dev_info(&pdev->dev, "Controller board EEPROM write protect default value read is %d\n", priv->rf_eeprom_wp_en_default);

	return 0;
}

static int anode_ctrl_probe(struct platform_device *pdev)
{ 
	struct anode_ctrl_priv *priv;
	int ret;
	
    	priv = devm_kzalloc(&pdev->dev, sizeof(struct anode_ctrl_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(priv->dev, "Unable to allocate memory.\n");
		return -ENOMEM;
	}

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);

	ret = anode_ctrl_parse_dt(pdev);
	if (ret) {	
		dev_err(&pdev->dev, "Parsing failed for controller-gpios node.");
		return ret;
	}	
	
	dev_info(&pdev->dev, "Configure default Anode controls on boot.");
   	
	/* Configuring Controller thermal trip */
	sprintf(priv->rf_pwr_dis_szGpio, "ctrlr_therm_alert");
	ret = gpio_request(priv->ctrlr_therm_alert_gpio, priv->ctrlr_therm_alert_szGpio);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
			priv->ctrlr_therm_alert_gpio, priv->ctrlr_therm_alert_szGpio);
		return ret;
	}

	ret = gpio_direction_input(priv->ctrlr_therm_alert_gpio);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
			priv->ctrlr_therm_alert_gpio, ret);
		return ret;
	}
	
	dev_info(&pdev->dev, "Controller thermal trip configured.");
   	
    /* Configure PSU_PGOOD */
    sprintf(priv->rf_pwr_dis_szGpio, "psu_pgood");
    ret = gpio_request(priv->psu_pgood_gpio, priv->psu_pgood_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->psu_pgood_gpio, priv->psu_pgood_szGpio);
        return ret;
    }

    ret = gpio_direction_input(priv->psu_pgood_gpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->psu_pgood_gpio, ret);
        return ret;
    }

    dev_info(&pdev->dev, "Controller PSU PGOOD status pin configured.");

	/* Configuring Controller board EEPROM */
	sprintf(priv->ctrl_eeprom_wp_enable_szGpio, "ctrl_eeprom_wp");
	ret = gpio_request(priv->ctrlr_eeprom_wp_enable_gpio, priv->ctrlr_eeprom_wp_enable_szGpio);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
			priv->ctrlr_eeprom_wp_enable_gpio, priv->ctrlr_eeprom_wp_enable_szGpio);
		return ret;
	}

	ret = gpio_direction_output(priv->ctrlr_eeprom_wp_enable_gpio, priv->ctrlr_eeprom_wp_enable_val);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
			priv->ctrlr_eeprom_wp_enable_gpio, ret);
		return ret;
	}

	dev_info(&pdev->dev, "Controller board EEPROM write protect is %s.",(priv->ctrlr_eeprom_wp_enable_val==0)?"disabled":"enabled");	
	
	dev_info(&pdev->dev, "Setting Sysfs for Anode Controller Board.");
	
	ret = sysfs_create_group(&priv->dev->kobj, &ocfema_ctrl_attr_group);
	if (ret) {
		dev_err(priv->dev, "unable to create sysfs files\n");
		return ret;
	}
	return 0;
}

static int anode_ctrl_remove(struct platform_device *pdev)
{
	struct anode_ctrl_priv *priv = platform_get_drvdata(pdev);
	sysfs_remove_group(&priv->dev->kobj, &anode_ctrl_attr_group);
	return 0;
}

static const struct of_device_id anode_of_match[] = {
	{ .compatible = "ukama,controller-gpios", },
	{}
};
MODULE_DEVICE_TABLE(of, anode_of_match);

static struct platform_driver anode_ctrl_driver = {
	.driver = {
		.name = "anode_ctrlr",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(anode_of_match),
	},
	.probe = anode_ctrl_probe,
	.remove = anode_ctrl_remove,
};

module_platform_driver(anode_ctrl_driver);

MODULE_DESCRIPTION("Anode Controller Driver");
MODULE_AUTHOR("<vishal@ukama.com>");
MODULE_LICENSE("GPL");
