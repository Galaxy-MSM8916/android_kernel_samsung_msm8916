/*
* sm5705.c - Regulator driver for the sm5705
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include <linux/module.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/sm5705/sm5705.h>
#include <linux/regulator/of_regulator.h>

/* SM5705 regulator IDs */
enum sm5705_regulators {
    SM5705_USBLDO1 = 0,
    SM5705_USBLDO2,

    SM5705_LDO_MAX,
};

struct sm5705_data {
    struct device *dev;
    struct sm5705_dev *iodev;
    int num_regulators;
    struct regulator_dev **rdev;

    u8 saved_states[SM5705_LDO_MAX];
};

struct voltage_map_desc {
    int min;
    int max;
    int step;
    unsigned int n_bits;
};

static const struct voltage_map_desc usbldo1_map_desc = {
    .min = 4700, .max = 4700, .step = 1, .n_bits = 3,
};

static const struct voltage_map_desc usbldo2_map_desc = {
    .min = 4700, .max = 4700, .step = 1, .n_bits = 4,
};

static const struct voltage_map_desc *reg_voltage_map[] = {
    [SM5705_USBLDO1] = &usbldo1_map_desc,
    [SM5705_USBLDO2] = &usbldo2_map_desc,
};

int sm5705_regulator_update_reg
     (struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
    u8 val_before = -1, val_after = -1;
    int ret;

    sm5705_read_reg(i2c, reg, &val_before);

    ret = sm5705_update_reg(i2c, reg, val, mask);
    if (ret)
     pr_err("%s: fail to update reg(%d)\n", __func__, ret);

    sm5705_read_reg(i2c, reg, &val_after);

    pr_info("%s: reg(0x%02x): [0x%02x]+[0x%02x]:[mask 0x%02x]->[0x%02x]\n",
     __func__, reg, val_before, val, mask, val_after);

    return ret;
}

static inline int sm5705_get_rid(struct regulator_dev *rdev)
{
    if (!rdev)
     return -ENODEV;

    if (!rdev->desc)
     return -ENODEV;

    dev_info(&rdev->dev, "func:%s\n", __func__);
    return rdev_get_id(rdev);
}

static bool sm5705_regulator_check_disable(struct regulator_dev *rdev)
{
    int rid = sm5705_get_rid(rdev);
    bool ret = false;

    //Please check below two case
    switch(rid) {
    case SM5705_USBLDO1:
     ret = is_muic_usb_path_ap_usb();
     break;
    case SM5705_USBLDO2:
     ret = is_muic_usb_path_cp_usb();
     break;
    default:
     pr_err("%s: invalid value rid(%d)\n", __func__, rid);
     break;
    }

    if (ret) {
     pr_info("%s: cannot disable regulator(%d)\n", __func__, rid);
     return false;
    }

    return true;
}
/*
static int sm5705_list_voltage_usbldo(struct regulator_dev *rdev,
                  unsigned int selector)
{
    int rid = sm5705_get_rid(rdev);
    dev_info(&rdev->dev, "func:%s\n", __func__);
    
    if (rid == SM5705_USBLDO1 || rid == SM5705_USBLDO2) {
        switch (selector) {
        case 0:
         return 4700000;
        case 1:
         return 4700000;
        default:
         return -EINVAL;
        }
    }

    return -EINVAL;
}
*/
static int sm5705_get_enable_register(struct regulator_dev *rdev,
                 int *reg, int *mask, int *pattern)
{
    int rid = sm5705_get_rid(rdev);
    dev_info(&rdev->dev, "func:%s\n", __func__);

    switch (rid) {
    case SM5705_USBLDO1:
        *reg = SM5705_REG_CHGCNTL7;
        *mask = 0x08;
        *pattern = 0x08;
        break;
    case SM5705_USBLDO2:
         *reg = SM5705_REG_CHGCNTL7;
         *mask = 0x10;
         *pattern = 0x10;
         break;     
    default:
     /* Not controllable or not exists */
    return -EINVAL;
    }

    return 0;
}

static int sm5705_get_disable_register(struct regulator_dev *rdev,
                 int *reg, int *mask, int *pattern)
{
    int rid = sm5705_get_rid(rdev);
    dev_info(&rdev->dev, "func:%s\n", __func__);

    switch (rid) {
    case SM5705_USBLDO1:
        *reg = SM5705_REG_CHGCNTL7;
        *mask = 0x08;
        *pattern = 0x00;
        break;
    case SM5705_USBLDO2:
        *reg = SM5705_REG_CHGCNTL7;
        *mask = 0x10;
        *pattern = 0x00;
        break;     
    default:
    /* Not controllable or not exists */
    return -EINVAL;
    }

    return 0;
}

static int sm5705_reg_is_enabled(struct regulator_dev *rdev)
{
    struct sm5705_data *sm5705 = rdev_get_drvdata(rdev);
    struct i2c_client *i2c = sm5705->iodev->i2c;
    int ret, reg, mask, pattern;
    u8 val;
    
    dev_info(&rdev->dev, "func:%s\n", __func__);
    
    ret = sm5705_get_enable_register(rdev, &reg, &mask, &pattern);
    
    if (ret == -EINVAL)
        return 1;   /* "not controllable" */
    else if (ret)
        return ret;

    ret = sm5705_read_reg(i2c, reg, &val);
    if (ret)
        return ret;

    return (val & mask) == pattern;
}

static int sm5705_reg_enable(struct regulator_dev *rdev)
{
    struct sm5705_data *sm5705 = rdev_get_drvdata(rdev);
    struct i2c_client *i2c = sm5705->iodev->i2c;
    int ret, reg, mask, pattern;
    
    dev_info(&rdev->dev, "func:%s\n", __func__);
    
    ret = sm5705_get_enable_register(rdev, &reg, &mask, &pattern);
    if (ret)
     return ret;

    return sm5705_regulator_update_reg(i2c, reg, pattern, mask);
}

static int sm5705_reg_disable(struct regulator_dev *rdev)
{
    struct sm5705_data *sm5705 = rdev_get_drvdata(rdev);
    struct i2c_client *i2c = sm5705->iodev->i2c;
    int ret, reg, mask, pattern;
    dev_info(&rdev->dev, "func:%s\n", __func__);

    if (!sm5705_regulator_check_disable(rdev))
        return -EINVAL;

    ret = sm5705_get_disable_register(rdev, &reg, &mask, &pattern);
    if (ret)
        return ret;

    return sm5705_regulator_update_reg(i2c, reg, pattern, mask);
}

static int sm5705_reg_enable_suspend(struct regulator_dev *rdev)
{
    dev_info(&rdev->dev, "func:%s\n", __func__);
    return 0;
}

static int sm5705_reg_disable_suspend(struct regulator_dev *rdev)
{
    struct sm5705_data *sm5705 = rdev_get_drvdata(rdev);
    struct i2c_client *i2c = sm5705->iodev->i2c;
    int ret, reg, mask, pattern;
    int rid = sm5705_get_rid(rdev);
    
    dev_info(&rdev->dev, "func:%s\n", __func__);
    
    ret = sm5705_get_disable_register(rdev, &reg, &mask, &pattern);
    if (ret)
        return ret;

    if (rid != SM5705_USBLDO1 && rid != SM5705_USBLDO2)
        return -EINVAL;

    sm5705_read_reg(i2c, reg, &sm5705->saved_states[rid]);

    dev_dbg(&rdev->dev, "%s (%xh -> %xh)\n",
     rdev->desc->name, sm5705->saved_states[rid] & mask,
     (~pattern) & mask);
    
    return sm5705_regulator_update_reg(i2c, reg, pattern, mask);
}

static struct regulator_ops sm5705_usbldo_ops = {
//    .list_voltage = sm5705_list_voltage_usbldo,
    .is_enabled = sm5705_reg_is_enabled,
    .enable = sm5705_reg_enable,
    .disable = sm5705_reg_disable,
    .set_suspend_enable = sm5705_reg_enable_suspend,
    .set_suspend_disable = sm5705_reg_disable_suspend,
};

static struct regulator_desc regulators[] = {
    {
        .name = "USBLDO1",
        .id = SM5705_USBLDO1,
        .ops = &sm5705_usbldo_ops,
        .type = REGULATOR_VOLTAGE,
        .owner = THIS_MODULE,
    }, {
        .name = "USBLDO2",
        .id = SM5705_USBLDO2,
        .ops = &sm5705_usbldo_ops,
        .type = REGULATOR_VOLTAGE,
        .owner = THIS_MODULE,
    },
};

static struct regulator_consumer_supply usbldo1_supply[] = {
    REGULATOR_SUPPLY("usbldo1", NULL),
};

static struct regulator_consumer_supply usbldo2_supply[] = {
    REGULATOR_SUPPLY("usbldo2", NULL),
};

#define USBLDO1_MIN_UV  4700000
#define USBLDO1_MAX_UV  4700000
#define USBLDO2_MIN_UV  4700000
#define USBLDO2_MAX_UV  4700000

#define USBLDO1_APPLY_UV  0
#define USBLDO2_APPLY_UV  0

#if defined(CONFIG_OF)

#define sm5705_init_consumer_supplies(p_data, r_init_data, id)			\
    (r_init_data) = (p_data)->regulators[SM5705_USBLDO##id].initdata;        \
    (r_init_data)->constraints.state_mem.enabled = 1;               \
    (r_init_data)->num_consumer_supplies = ARRAY_SIZE(usbldo## id ##_supply);  \
    (r_init_data)->consumer_supplies = usbldo## id ##_supply;   \
    (r_init_data)->constraints.min_uV = USBLDO## id ##_MIN_UV;   \
    (r_init_data)->constraints.max_uV = USBLDO## id ##_MAX_UV;   \
    (r_init_data)->constraints.apply_uV = USBLDO## id ##_APPLY_UV;

static int sm5705_pmic_dt_parse_pdata(struct platform_device *pdev,
                 struct sm5705_platform_data *pdata)
{
    struct sm5705_dev *iodev = dev_get_drvdata(pdev->dev.parent);
    struct device_node *pmic_np, *regulators_np, *reg_np;
    struct sm5705_regulator_data *rdata;
    struct regulator_init_data *r_initdata;

    unsigned int i;

    pmic_np = of_node_get(iodev->dev->of_node);
    if (!pmic_np) {
         dev_err(&pdev->dev, "could not find pmic sub-node\n");
         return -ENODEV;
    }

    regulators_np = of_find_node_by_name(pmic_np, "regulators");
    if (!regulators_np) {
         dev_err(&pdev->dev, "could not find regulators sub-node\n");
         return -EINVAL;
    }

    /* count the number of regulators to be supported in pmic */
    pdata->num_regulators = 0;

    for_each_child_of_node(regulators_np, reg_np) {
         pdata->num_regulators++;
    }

    rdata = devm_kzalloc(&pdev->dev, sizeof(*rdata) *
             pdata->num_regulators, GFP_KERNEL);
    if (!rdata) {
         of_node_put(regulators_np);
         dev_err(&pdev->dev, "could not allocate memory for regulator data\n");
         return -ENOMEM;
    }

    pdata->regulators = rdata;

    for_each_child_of_node(regulators_np, reg_np) {
     for (i = 0; i < ARRAY_SIZE(regulators); i++) {
         if (!of_node_cmp(reg_np->name, regulators[i].name))
             break;
     }

     if (i == ARRAY_SIZE(regulators)) {
         dev_warn(&pdev->dev, "don't know how to configure regulator %s\n",
              reg_np->name);
         continue;
     }

     rdata->id = i;
     rdata->initdata = of_get_regulator_init_data(&pdev->dev, reg_np);//dts parse
     rdata->reg_node = reg_np;
     rdata++;
    }
    of_node_put(regulators_np);

    /* original shelve source */
    sm5705_init_consumer_supplies(pdata, r_initdata, 1);
    sm5705_init_consumer_supplies(pdata, r_initdata, 2);

    return 0;
}
#else
static int sm5705_pmic_dt_parse_pdata(struct platform_device *pdev,
                 struct sm5705_platform_data *pdata)
{
 return 0;
}
#endif /* CONFIG_OF */

static int sm5705_pmic_probe(struct platform_device *pdev)
{
    struct sm5705_dev *iodev = dev_get_drvdata(pdev->dev.parent);
    struct sm5705_platform_data *pdata = dev_get_platdata(iodev->dev);
    struct regulator_config config = { };
    struct regulator_dev **rdev;
    struct sm5705_data *sm5705;
    struct i2c_client *i2c;
    int i, ret, size;

    //dev_info(&pdev->dev, "%s\n", __func__);
    printk("%s\n", __func__);
    if (!pdata) {
         pr_info("[%s:%d] !pdata\n", __FILE__, __LINE__);
         dev_err(pdev->dev.parent, "No platform init data supplied.\n");
         return -ENODEV;
    }

    if (iodev->dev->of_node) {
         ret = sm5705_pmic_dt_parse_pdata(pdev, pdata);
         if (ret)
             return ret;
    }

    sm5705 = devm_kzalloc(&pdev->dev, sizeof(struct sm5705_data),
             GFP_KERNEL);
    if (!sm5705) {
         pr_info("[%s:%d] if (!sm5705)\n", __FILE__, __LINE__);
         return -ENOMEM;
    }
    size = sizeof(struct regulator_dev *) * pdata->num_regulators;
    sm5705->rdev = devm_kzalloc(&pdev->dev, size, GFP_KERNEL);
    if (!sm5705->rdev) {
         pr_info("[%s:%d] if (!sm5705->rdev)\n", __FILE__, __LINE__);
         kfree(sm5705);
         return -ENOMEM;
    }

    rdev = sm5705->rdev;
    sm5705->dev = &pdev->dev;
    sm5705->iodev = iodev;
    sm5705->num_regulators = pdata->num_regulators;
    platform_set_drvdata(pdev, sm5705);
    i2c = sm5705->iodev->i2c;
    pr_info("[%s:%d] pdata->num_regulators:%d\n", __FILE__, __LINE__,
     pdata->num_regulators);

    for (i = 0; i < pdata->num_regulators; i++) {

     const struct voltage_map_desc *desc;
     int id = pdata->regulators[i].id;
     pr_info("[%s:%d] for in pdata->num_regulators:%d\n", __FILE__,
         __LINE__, pdata->num_regulators);
     desc = reg_voltage_map[id];
     if (id == SM5705_USBLDO1 || id == SM5705_USBLDO2)
         regulators[id].n_voltages = 1;

     config.dev = sm5705->dev;
     config.init_data = pdata->regulators[i].initdata;
     config.driver_data = sm5705;
     config.of_node = pdata->regulators[i].reg_node;

     rdev[i] = regulator_register(&regulators[id], &config);
     if (IS_ERR(rdev[i])) {
         ret = PTR_ERR(rdev[i]);
         dev_err(sm5705->dev, "regulator init failed for %d\n",
             id);
         rdev[i] = NULL;
         goto err;
        }
    }

    //dev_info(&pdev->dev, "%s DONE\n", __func__);
    printk("%s DONE\n", __func__);

    return 0;
    err:
        pr_info("[%s:%d] err - (i = %d):\n", __FILE__, __LINE__, i);
    //for (; i > 0; i--)
    // if (rdev[i])
    //     regulator_unregister(rdev[i-1]);

    //kfree(sm5705);

    return ret;
}

static int sm5705_pmic_remove(struct platform_device *pdev)
{
    struct sm5705_data *sm5705 = platform_get_drvdata(pdev);
    struct regulator_dev **rdev = sm5705->rdev;
    int i;
    dev_info(&pdev->dev, "%s\n", __func__);
    for (i = 0; i < sm5705->num_regulators; i++)
     if (rdev[i])
         regulator_unregister(rdev[i]);

    kfree(sm5705->rdev);
    kfree(sm5705);

    return 0;
}

static const struct platform_device_id sm5705_pmic_id[] = {
    {"sm5705-usbldo", 0},
    {},
};
MODULE_DEVICE_TABLE(platform, sm5705_pmic_id);

static struct platform_driver sm5705_pmic_driver = {
    .driver = {
        .name = "sm5705-usbldo",
        .owner = THIS_MODULE,
        },
    .probe = sm5705_pmic_probe,
    .remove = sm5705_pmic_remove,
    .id_table = sm5705_pmic_id,
};

static int __init sm5705_pmic_init(void)
{
    printk("%s\n",__func__);
    return platform_driver_register(&sm5705_pmic_driver);
}
module_init(sm5705_pmic_init);

static void __exit sm5705_pmic_cleanup(void)
{
    platform_driver_unregister(&sm5705_pmic_driver);
}

module_exit(sm5705_pmic_cleanup);

MODULE_DESCRIPTION("SM5705 Regulator Driver");
MODULE_LICENSE("GPL");

