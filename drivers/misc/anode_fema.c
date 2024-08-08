 #include <linux/err.h>
 #include <linux/module.h>
 #include <linux/types.h>
 #include <linux/platform_device.h>
 #include <linux/io.h>
 #include <linux/of_gpio.h>
 #include <linux/of.h>
 #include <linux/sysfs.h>
 #include <linux/delay.h>


struct anode_fema_priv
{
    struct device *dev;
    
    int pg_reg_5v_gpio;
    char pg_reg_5v_szGpio[32];
    u8 pg_reg_5v_val;
    u32 pg_reg_5v_default;
    

	int tx_rf_enable_gpio;
	char tx_rf_enable_szGpio[32];
    u8 tx_rf_enable_val;
	u32 tx_rf_enable_default;
	
    int rx_rf_enable_gpio;
    char rx_rf_enable_szGpio[32];
    u8 rx_rf_enable_val;
    u32 rx_rf_enable_default;
    
    int pa_disable_gpio;
    char pa_disable_szGpio[32];
    u8 ps_disable_val;
    u32 ps_disable_default;

    int pa_vds_enable_gpio;
    char pa_vds_enable_szGpio[32];
    u8 pa_vds_enable_val;
    u32 pa_vds_enable_default;

    int rf_pal_enable_gpio;
    char rf_pal_enable_szGpio[32];
    u8 rf_pal_enable_val;
    u32 rf_pal_enable_default;

	int eeprom_wp_enable_gpio;
	char eeprom_wp_enable_szGpio[32];
	u8 eeprom_wp_enable_val;
	u32 eeprom_wp_enable_default;
	
};

static ssize_t show_pg_reg_5v(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "Power Good status for 5V reg is in %s state.", ((priv->pg_reg_5v_val)==0)?"bad":"good");
    return sprintf(buf, "%d\n", priv->rf_therm_trip_val);
}

static ssize_t show_rf_pal_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "RF PAL enable is %s.", ((priv->rf_pal_enable_val)==0)?"disabled":"enabled");
    return sprintf(buf, "%d\n", priv->rf_pal_enable_val);
}

static ssize_t set_rf_pal_enable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    u32 val;
    ssize_t ret;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
        return ret;

    priv->rf_pal_enable_val = val ? 1 : 0;
    gpio_set_value_cansleep(priv->rf_pal_enable_gpio, priv->rf_pal_enable_val);
    dev_info(dev, "RF PAL enable is %s now.", ((priv->rf_pal_enable_val)==0)?"disabled":"enabled");
    return count;
}

static ssize_t show_pa_vds_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "PA VDS enable is %s.", ((priv->pa_vds_enable_val)==0)?"disabled":"enabled");
    return sprintf(buf, "%d\n", priv->pa_vds_enable_val);
}

static ssize_t set_pa_vds_enable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    u32 val;
    ssize_t ret;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
        return ret;

    priv->pa_vds_enable_val = val ? 1 : 0;
    gpio_set_value_cansleep(priv->pa_vds_enable_gpio, priv->pa_vds_enable_val);
    dev_info(dev, "PA VDS enable is %s now.", ((priv->pa_vds_enable_val)==0)?"disabled":"enabled");
    return count;
}

static ssize_t show_pa_disable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "PA disable is %s.", ((priv->pa_disable_val)==0)?"disabled":"enabled");
    return sprintf(buf, "%d\n", priv->pa_disable_val);
}

static ssize_t set_pa_disable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    u32 val;
    ssize_t ret;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
        return ret;

    priv->pa_disable_val = val ? 1 : 0;
    gpio_set_value_cansleep(priv->pa_disable_gpio, priv->pa_disable_val);
    dev_info(dev, "PA disable is %s now.", ((priv->pa_disable_val)==0)?"disabled":"enabled");
    return count;
}

static ssize_t show_eeprom_wp_enable(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct anode_fema_priv *priv = dev_get_drvdata(dev);
	dev_info(dev, "FEMA eeprom write protect is %s.", ((priv->eeprom_wp_enable_val)==0)?"disabled":"enabled");
	return sprintf(buf, "%d\n", priv->eeprom_wp_enable_val);
}

static ssize_t set_eeprom_wp_enable(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct anode_fema_priv *priv = dev_get_drvdata(dev);
	u32 val;
	ssize_t ret;

	ret = kstrtouint(buf, 0, &val);
	if (ret)
		return ret;

	priv->eeprom_wp_enable_val = val ? 1 : 0;
	gpio_set_value_cansleep(priv->eeprom_wp_enable_gpio, priv->eeprom_wp_enable_val);
	dev_info(dev, "FEMA EEPROM write protect is %s now.", ((priv->eeprom_wp_enable_val)==0)?"disabled":"enabled");
	return count;
}

static ssize_t show_tx_rf_enable(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct anode_fema_priv *priv = dev_get_drvdata(dev);
	dev_info(dev, "TX is %s", ((priv->tx_rf_enable_val)==0)?"disabled":"enabled");
	return sprintf(buf, "%d\n", priv->tx_rf_enable_val);
}

static ssize_t set_tx_rf_enable(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct anode_fema_priv *priv = dev_get_drvdata(dev);
	u32 val;
	ssize_t ret;

	ret = kstrtouint(buf, 0, &val);
	if (ret)
		return ret;

	priv->tx_rf_enable_val = val ? 1 : 0;

	gpio_set_value_cansleep(priv->tx_rf_enable_gpio, priv->tx_rf_enable_val);
	dev_info(dev, "TX is %s now.", ((priv->tx_rf_enable_val)==0)?"disabled":"enabled");
	return count;
}

static ssize_t show_rx_rf_enable(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    dev_info(dev, "RX is %s", ((priv->rx_rf_enable_val)==0)?"disabled":"enabled");
    return sprintf(buf, "%d\n", priv->rx_rf_enable_val);
}

static ssize_t set_rx_rf_enable(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct anode_fema_priv *priv = dev_get_drvdata(dev);
    u32 val;
    ssize_t ret;

    ret = kstrtouint(buf, 0, &val);
    if (ret)
        return ret;

    priv->rx_rf_enable_val = val ? 1 : 0;

    gpio_set_value_cansleep(priv->rx_rf_enable_gpio, priv->rx_rf_enable_val);
    dev_info(dev, "RX is %s now.", ((priv->rx_rf_enable_val)==0)?"disabled":"enabled");
    return count;
}

static DEVICE_ATTR(pg_reg_5v, S_IRUGO, show_pg_reg_5v, NULL);
static DEVICE_ATTR(eeprom_wp_enable, S_IWUSR | S_IRUGO, show_eeprom_wp_enable, set_eeprom_wp_enable);
static DEVICE_ATTR(tx_rf_enable, S_IWUSR | S_IRUGO, show_tx_rf_enable, set_tx_rf_enable);
static DEVICE_ATTR(rx_rf_enable, S_IWUSR | S_IRUGO, show_rx_rf_enable, set_rx_rf_enable);
static DEVICE_ATTR(pa_vds_enable, S_IWUSR | S_IRUGO, show_pa_vds_enable, set_pa_vds_enable);
static DEVICE_ATTR(pa_disable, S_IWUSR | S_IRUGO, show_pa_disable, set_pa_disable);
static DEVICE_ATTR(rf_pal_enable, S_IWUSR | S_IRUGO, show_rf_pal_enable, set_rf_pal_enable);


static struct attribute *anode_fema_attrs[] = {
    &dev_attr_pg_reg_5v.attr,
    &dev_attr_eeprom_wp_enable.attr,
	&dev_attr_tx_rf_enable.attr,
	&dev_attr_rx_rf_enable.attr,
    &dev_attr_pa_disable.attr,
    &dev_attr_pa_vds__enable.attr,
    &dev_attr_rf_pal_enable.attr,
	NULL,
};

static const struct attribute_group anode_fema_attr_group = {
	.attrs = anode_fema_attrs,
};

int anode_fema_parse_dt(struct platform_device *pdev)
{
	int ret = 0; 
	int nr = 0;
	int i = 0; 
	struct device_node *np = pdev->dev.of_node;
	struct anode_fema_priv *priv = dev_get_drvdata(&pdev->dev);
	if (!np) {
		return -EINVAL;
	}
	
    /* PG 5V regulator */
    priv->pg_reg_5v_gpio = of_get_named_gpio(np, "pg-reg-5v", 0);
    if (priv->pg_reg_5v_gpio < 0) {
        dev_err(&pdev->dev, "Can't read gpio pg-reg-5v\n");
        return -EINVAL;
    }

	/* EEPROM Write protect */
	priv->eeprom_wp_enable_gpio = of_get_named_gpio(np, "eeprom-wp-enable", 0);
	if (priv->eeprom_wp_enable_gpio < 0) {
		dev_err(&pdev->dev, "Can't read gpio eeprom-wp-enable\n");
		return -EINVAL;
	}
    
	/* EEPROM Write protect default value */
	ret = of_property_read_u32(np, "eeprom-wp-enable-default", &priv->eeprom_wp_enable_default);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't read eeprom-wp-enable-default.\n");
		return ret;
	}
	
	dev_info(&pdev->dev, "FEMA EEPROM write protect value read is %d\n", priv->eeprom_wp_enable_default);
	priv->eeprom_wp_enable_val = priv->eeprom_wp_enable_default;
	
	/* TX RF Chain Enable */
	priv->tx_rf_enable_gpio = of_get_named_gpio(np, "tx-rf-enable", 0);
	if (priv->tx_rf_enable_gpio < 0) {
		dev_err(&pdev->dev, "Can't get gpio tx-rf-enable\n");
		return -EINVAL;
	}
	
	/* TX RF chain enable default value */
	ret = of_property_read_u32(np, "tx-rf-enable-default", &priv->tx_rf_enable_default);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't read tx-rf-enable-default.\n");
		return ret;
	}
	priv->tx_rf_enable_val = priv->tx_rf_enable_default;
	dev_info(&pdev->dev, "TX RF chain enable value read is %d\n", priv->tx_rf_enable_default);

    /* RX RF Chain Enable */
    priv->rx_rf_enable_gpio = of_get_named_gpio(np, "rx-rf-enable", 0);
    if (priv->rx_rf_enable_gpio < 0) {
        dev_err(&pdev->dev, "Can't get gpio rx-rf-enable\n");
        return -EINVAL;
    }

    /* RX RF chain enable default value */
    ret = of_property_read_u32(np, "rx-rf-enable-default", &priv->rx_rf_enable_default);
    if (ret < 0) {
        dev_err(&pdev->dev, "Can't read rx-rf-enable-default.\n");
        return ret;
    }
    priv->rx_rf_enable_val = priv->rx_rf_enable_default;
    dev_info(&pdev->dev, "RX RF chain enable value read is %d\n", priv->rx_rf_enable_default);    

    /* PA Disable */
    priv->pa_disable_gpio = of_get_named_gpio(np, "pa-disable", 0);
    if (priv->pa_disable_gpio < 0) {
        dev_err(&pdev->dev, "Can't get gpio pa-disable\n");
        return -EINVAL;
    }

    /* PA disable default value */
    ret = of_property_read_u32(np, "pa-disable-default", &priv->pa_disable_default);
    if (ret < 0) {
        dev_err(&pdev->dev, "Can't read pa-disable-default.\n");
        return ret;
    }
    priv->pa_disable_val = priv->pa_disable_default;
    dev_info(&pdev->dev, "PA disable value read is %d\n", priv->pa_disable_default);

    /* PA VDS Enable */
    priv->pa_vds_enable_gpio = of_get_named_gpio(np, "pa-vds-enable", 0);
    if (priv->pa_vds_enable_gpio < 0) {
        dev_err(&pdev->dev, "Can't get gpio pa-vds-enable\n");
        return -EINVAL;
    }

    /* PA VDS enable default value */
    ret = of_property_read_u32(np, "pa-vds-enable-default", &priv->pa_vds_enable_default);
    if (ret < 0) {
        dev_err(&pdev->dev, "Can't read pa-vds-enable-default.\n");
        return ret;
    }
    priv->pa_vds_enable_val = priv->pa_vds_enable_default;
    dev_info(&pdev->dev, "PA VDS enable value read is %d\n", priv->pa_vds_enable_default);

    /* RF PAL Enable */
    priv->rf_pal_enable_gpio = of_get_named_gpio(np, "rf-pal-enable", 0);
    if (priv->rf_pal_enable_gpio < 0) {
        dev_err(&pdev->dev, "Can't get gpio rf-pal-enable\n");
        return -EINVAL;
    }

    /* RF PAL enable default value */
    ret = of_property_read_u32(np, "rf-pal-enable-default", &priv->rf_pal_enable_default);
    if (ret < 0) {
        dev_err(&pdev->dev, "Can't read rf-pal-enable-default.\n");
        return ret;
    }
    priv->rf_pal_enable_val = priv->rf_pal_enable_default;
    dev_info(&pdev->dev, "RF PAL enable value read is %d\n", priv->rf_pal_enable_default);

	return 0;
}

static int anode_fema_probe(struct platform_device *pdev)
{ 
	struct anode_fema_priv *priv;
	int ret;
	int i = 0;	
    	priv = devm_kzalloc(&pdev->dev, sizeof(struct anode_fema_priv), GFP_KERNEL);
	if (!priv) {
		dev_err(priv->dev, "Unable to allocate memory.\n");
		return -ENOMEM;
	}

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);

	ret = anode_fema_att_parse_dt(pdev);
	if (ret) {	
		dev_err(&pdev->dev, "Parsing failed for fema-gpios.");
		return ret;
	}
    
    /* PG Reg 5V */    
    sprintf(priv->pg_reg_5p_szGpio, "pg_reg_5v");
    ret = gpio_request(priv->pg_reg_5v_gpio, priv->pg_reg_5v_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->pg_reg_5v_gpio, priv->pg_reg_5v_szGpio);
        return ret;
    }

    ret = gpio_direction_input(priv->pg_reg_5v_gpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->pg_reg_5v_gpio, ret);
        return ret;
    }
    dev_info(&pdev->dev, "Power good for 5V regulator is configured.");

   	/* EEPROM write protect enable */
	sprintf(priv->eeprom_wp_enable_szGpio, "eeprom_wp_enable");
	ret = gpio_request(priv->eeprom_wp_enable_gpio, priv->eeprom_wp_enable_szGpio);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
			priv->eeprom_wp_enable_gpio, priv->eeprom_wp_enable_szGpio);
		return ret;
	}

	ret = gpio_direction_output(priv->eeprom_wp_enable_gpio, priv->eeprom_wp_enable_val);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
			priv->eeprom_wp_enable_gpio, ret);
		return ret;
	}
	dev_info(&pdev->dev, "Setting default FEMA EEPROM write protect enable value to %d.",priv->eeprom_wp_enable_val);
	
	/* TX Chain enable */
	sprintf(priv->tx_rf_enable_szGpio, "tx_rf_enable");
	ret = gpio_request(priv->tx_rf_enable_gpio, priv->tx_rf_enable_szGpio);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
			priv->tx_rf_enable_gpio, priv->tx_rf_enable_szGpio);
		return ret;
	}

	ret = gpio_direction_output(priv->tx_rf_enable_gpio, priv->tx_rf_enable_val);
	if ( ret )
	{
		dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
			priv->tx_rf_enable_gpio, ret);
		return ret;
	}
	dev_info(&pdev->dev, "Setting default TX RF Chain enable value to %d.",priv->tx_rf_enable_val);	
   		
    /* RX Chain enable */
    sprintf(priv->rx_rf_enable_szGpio, "rx_rf_enable");
    ret = gpio_request(priv->rx_rf_enable_gpio, priv->rx_rf_enable_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->rx_rf_enable_gpio, priv->rx_rf_enable_szGpio);
        return ret;
    }

    ret = gpio_direction_output(priv->rx_rf_enable_gpio, priv->rx_rf_enable_val);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->rx_rf_enable_gpio, ret);
        return ret;
    }
    dev_info(&pdev->dev, "Setting default RX RF Chain enable value to %d.",priv->rx_rf_enable_val)    
    
    /* PA Disable  */
    sprintf(priv->pa_disable_szGpio, "pa_disable");
    ret = gpio_request(priv->pa_disable_gpio, priv->pa_disable_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->pa_disable_gpio, priv->pa_disable_szGpio);
        return ret;
    }

    ret = gpio_direction_output(priv->pa_disable_gpio, priv->pa_disable_val);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->pa_disable_gpio, ret);
        return ret;
    }
    dev_info(&pdev->dev, "Setting default PA disable value to %d.",priv->pa_disable_val);
    
    /* PA VDS enable  */
    sprintf(priv->pa_vds_enable_szGpio, "pa_vds_enable");
    ret = gpio_request(priv->pa_vds_enable_gpio, priv->pa_vds_enable_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->pa_vds_enable_gpio, priv->pa_vds_enable_szGpio);
        return ret;
    }

    ret = gpio_direction_output(priv->pa_vds_enable_gpio, priv->pa_vds_enable_val);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->pa_vds_enable_gpio, ret);
        return ret;
    }
    dev_info(&pdev->dev, "Setting default PA VDS enable value to %d.",priv->pa_vds_enable_val);
    
    /* RF PAL enable  */
    sprintf(priv->rf_pal_enable_szGpio, "rf_pal_enable");
    ret = gpio_request(priv->rf_pal_enable_gpio, priv->rf_pal_enable_szGpio);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not obtain GPIO %d: %s\n",
            priv->rf_pal_enable_gpio, priv->rf_pal_enable_szGpio);
        return ret;
    }

    ret = gpio_direction_output(priv->rf_pal_enable_gpio, priv->rf_pal_enable_val);
    if ( ret )
    {
        dev_err(&pdev->dev, "Could not configure GPIO %d direction: %d\n",
            priv->rf_pal_enable_gpio, ret);
        return ret;
    }
    dev_info(&pdev->dev, "Setting default RF PAL enable value to %d.",priv->rf_pal_enable_val);

	dev_info(&pdev->dev, "Setting Sysfs for ANode Fema board devices.");

	ret = sysfs_create_group(&priv->dev->kobj, &anode_fema_attr_group);
	if (ret) {
		dev_err(priv->dev, "unable to create sysfs files\n");
		return ret;
	}
	return 0;
}

static int anode_fema_remove(struct platform_device *pdev)
{
	struct anode_fema_priv *priv = platform_get_drvdata(pdev);

	sysfs_remove_group(&priv->dev->kobj, &anode_fema_attr_group);
	return 0;
}

static const struct of_device_id anode_fema_of_match[] = {
	{ .compatible = "ukama,fema-gpios", },
	{}
};
MODULE_DEVICE_TABLE(of, anode_fema_of_match);

static struct platform_driver anode_fema_driver = {
	.driver = {
		.name = "anode_fema",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(anode_fema_of_match),
	},
	.probe = anode_fema_probe,
	.remove = anode_fema_remove,
};

module_platform_driver(anode_fema_driver);

MODULE_DESCRIPTION("Ukama Fema board Driver");
MODULE_AUTHOR("<vishal@ukama.com>");
MODULE_LICENSE("GPL");

		
