// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#define pr_fmt(fmt) "<ALS/PS> " fmt

#include "inc/alsps_factory.h"

struct alsps_factory_private {
	uint32_t gain;
	uint32_t sensitivity;
	struct alsps_factory_fops *fops;
};

static struct alsps_factory_private alsps_factory;

static int alsps_factory_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int alsps_factory_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static long alsps_factory_unlocked_ioctl(struct file *file, unsigned int cmd,
					 unsigned long arg)
{
	long err = 0;
	void __user *ptr = (void __user *)arg;
	int data = 0;
	uint32_t enable = 0;
	int threshold_data[2] = {0, 0};
	//moto add
	uint32_t als_value[3] = {0, 0,0};
	int als_cali = 0;
	MOT_ALSPS_DATA_GET mot_alsps_data_get;
	MOT_ALSPS_DATA_SET mot_alsps_data_set;
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok((void __user *)arg,
                                _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok((void __user *)arg,
				_IOC_SIZE(cmd));
	if (err) {
		pr_debug("access error: %08X, (%2d, %2d)\n", cmd,
			  _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch (cmd) {
	case ALSPS_SET_PS_MODE:
		if (copy_from_user(&enable, ptr, sizeof(enable)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_enable_sensor != NULL) {
			err = alsps_factory.fops->ps_enable_sensor(enable, 200);
			if (err < 0) {
				pr_err("ALSPS_SET_PS_MODE fail!\n");
				return -EINVAL;
			}
			pr_debug(
				"ALSPS_SET_PS_MODE, enable: %d, sample_period:%dms\n",
				enable, 200);
		} else {
			pr_err("ALSPS_SET_PS_MODE NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_GET_PS_RAW_DATA:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_get_raw_data != NULL) {
			err = alsps_factory.fops->ps_get_raw_data(&data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_PS_RAW_DATA read data fail!\n");
				return -EINVAL;
			}
			if (copy_to_user(ptr, &data, sizeof(data)))
				return -EFAULT;
		} else {
			pr_err("ALSPS_GET_PS_RAW_DATA NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_SET_ALS_MODE:
		if (copy_from_user(&enable, ptr, sizeof(enable)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->als_enable_sensor != NULL) {
			err = alsps_factory.fops->als_enable_sensor(enable,
								    200);
			if (err < 0) {
				pr_err("ALSPS_SET_ALS_MODE fail!\n");
				return -EINVAL;
			}
			pr_debug(
				"ALSPS_SET_ALS_MODE, enable: %d, sample_period:%dms\n",
				enable, 200);
		} else {
			pr_err("ALSPS_SET_ALS_MODE NULL\n");
			return -EINVAL;
		}
		return 0;
        //moto add
        case ALSPS_GET_ALS_DATA:
            if (alsps_factory.fops != NULL && alsps_factory.fops->als_get_data != NULL) {
                err = alsps_factory.fops->als_get_data(als_value);
                if (err < 0) {
                    pr_err("ALSPS_GET_ALS_DATA read data fail!\n");
                    return -EINVAL;
                }
                if (copy_to_user(ptr, als_value, sizeof(als_value)))
                    return -EFAULT;
            } else {
                pr_err("ALSPS_GET_ALS_DATA NULL\n");
                return -EINVAL;
            }
            return 0;
	case ALSPS_GET_ALS_RAW_DATA:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->als_get_raw_data != NULL) {
			err = alsps_factory.fops->als_get_raw_data(&data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_ALS_RAW_DATA read data fail!\n");
				return -EINVAL;
			}
			if (copy_to_user(ptr, &data, sizeof(data)))
				return -EFAULT;
		} else {
			pr_err("ALSPS_GET_ALS_RAW_DATA NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_ALS_ENABLE_CALI:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->als_enable_calibration != NULL) {
			err = alsps_factory.fops->als_enable_calibration();
			if (err < 0) {
				pr_err("ALSPS_ALS_ENABLE_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_ALS_ENABLE_CALI NULL\n");
			return -EINVAL;
		}
		return 0;
	//moto add
	case ALSPS_GET_ALS_DATA_MOT:
		if (alsps_factory.fops != NULL &&
			alsps_factory.fops->als_get_data_mot != NULL) {
			err = alsps_factory.fops->als_get_data_mot(mot_alsps_data_get.data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_ALS_DATA_MOT read data fail!\n");
				return -EINVAL;
			}
			if (copy_to_user(ptr, &mot_alsps_data_get, sizeof(mot_alsps_data_get)))
				return -EFAULT;
		} else {
			pr_err("ALSPS_GET_ALS_DATA_MOT NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_SET_ALS_DATA_MOT:
		if (copy_from_user(&mot_alsps_data_set, ptr, sizeof(mot_alsps_data_set)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->als_set_data_mot != NULL) {
			err = alsps_factory.fops->als_set_data_mot(mot_alsps_data_set.data);
			if (err < 0) {
				pr_err("ALSPS_SET_ALS_DATA_MOT FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_SET_ALS_DATA_MOT NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_ALS_SET_CALI:
		if (copy_from_user(&als_cali, ptr, sizeof(als_cali)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->als_set_cali != NULL) {
			err = alsps_factory.fops->als_set_cali(als_cali);
			if (err < 0) {
				pr_err("ALSPS_ALS_SET_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_ALS_SET_CALI NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_GET_PS_TEST_RESULT:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_get_data != NULL) {
			err = alsps_factory.fops->ps_get_data(&data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_PS_TEST_RESULT read data fail!\n");
				return -EINVAL;
			}
			if (copy_to_user(ptr, &data, sizeof(data)))
				return -EFAULT;
		} else {
			pr_err("ALSPS_GET_PS_TEST_RESULT NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_GET_PS_THRESHOLD_HIGH:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_get_threshold != NULL) {
			err = alsps_factory.fops->ps_get_threshold(
				threshold_data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_PS_THRESHOLD_HIGH read data fail!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_GET_PS_THRESHOLD_HIGH NULL\n");
			return -EINVAL;
		}
		if (copy_to_user(ptr, &threshold_data[0],
				 sizeof(threshold_data[0])))
			return -EFAULT;
		return 0;
	case ALSPS_GET_PS_THRESHOLD_LOW:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_get_threshold != NULL) {
			err = alsps_factory.fops->ps_get_threshold(
				threshold_data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_PS_THRESHOLD_HIGH read data fail!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_GET_PS_THRESHOLD_HIGH NULL\n");
			return -EINVAL;
		}
		if (copy_to_user(ptr, &threshold_data[1],
				 sizeof(threshold_data[1])))
			return -EFAULT;
		return 0;
	case ALSPS_SET_PS_THRESHOLD:
		if (copy_from_user(threshold_data, ptr, sizeof(threshold_data)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_set_threshold != NULL) {
			err = alsps_factory.fops->ps_set_threshold(
				threshold_data);
			if (err < 0) {
				pr_err("ALSPS_SET_PS_THRESHOLD fail!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_SET_PS_THRESHOLD NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_IOCTL_SET_CALI:
		if (copy_from_user(&data, ptr, sizeof(data)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_set_cali != NULL) {
			err = alsps_factory.fops->ps_set_cali(data);
			if (err < 0) {
				pr_err("ALSPS_IOCTL_SET_CALI fail!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_IOCTL_SET_CALI NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_IOCTL_GET_CALI:
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_get_cali != NULL) {
			err = alsps_factory.fops->ps_get_cali(&data);
			if (err < 0) {
				pr_err("ALSPS_IOCTL_GET_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_IOCTL_GET_CALI NULL\n");
			return -EINVAL;
		}
		if (copy_to_user(ptr, &data, sizeof(data)))
			return -EFAULT;
		return 0;
	case ALSPS_IOCTL_ALS_GET_CALI:
		if (alsps_factory.fops != NULL &&
			alsps_factory.fops->als_get_cali != NULL) {
			err = alsps_factory.fops->als_get_cali(&data);
			if (err < 0) {
				pr_err("ALSPS_IOCTL_ALS_GET_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_IOCTL_ALS_GET_CALI NULL\n");
			return -EINVAL;
		}
		if (copy_to_user(ptr, &data, sizeof(data)))
			return -EFAULT;
		return 0;
	case ALSPS_IOCTL_CLR_CALI:
		if (copy_from_user(&data, ptr, sizeof(data)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
			alsps_factory.fops->ps_clear_cali != NULL) {
			err = alsps_factory.fops->ps_clear_cali();
			if (err < 0) {
				pr_err("ALSPS_IOCTL_CLR_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_IOCTL_CLR_CALI NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_PS_ENABLE_CALI:
		if (alsps_factory.fops != NULL &&
			alsps_factory.fops->ps_enable_calibration != NULL) {
			err = alsps_factory.fops->ps_enable_calibration();
			if (err < 0) {
				pr_err("ALSPS_PS_ENABLE_CALI FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_PS_ENABLE_CALI NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_IOCTL_SELF_TEST:
		if (alsps_factory.fops != NULL && alsps_factory.fops->do_self_test != NULL) {
			err = alsps_factory.fops->do_self_test();
			if (err < 0) {
				pr_err("ALSPS_IOCTL_SELF_TEST FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_IOCTL_SELF_TEST NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_GET_PS_DATA_MOT:
		if (alsps_factory.fops != NULL &&
			alsps_factory.fops->ps_get_data_mot != NULL) {
			err = alsps_factory.fops->ps_get_data_mot(mot_alsps_data_get.data);
			if (err < 0) {
				pr_err(
					"ALSPS_GET_PS_DATA_MOT read data fail!\n");
				return -EINVAL;
			}
			if (copy_to_user(ptr, &mot_alsps_data_get, sizeof(mot_alsps_data_get)))
				return -EFAULT;
		} else {
			pr_err("ALSPS_GET_PS_DATA_MOT NULL\n");
			return -EINVAL;
		}
		return 0;
	case ALSPS_SET_PS_DATA_MOT:
		if (copy_from_user(&mot_alsps_data_set, ptr, sizeof(mot_alsps_data_set)))
			return -EFAULT;
		if (alsps_factory.fops != NULL &&
		    alsps_factory.fops->ps_set_data_mot != NULL) {
			err = alsps_factory.fops->ps_set_data_mot(mot_alsps_data_set.data);
			if (err < 0) {
				pr_err("ALSPS_SET_PS_DATA_MOT FAIL!\n");
				return -EINVAL;
			}
		} else {
			pr_err("ALSPS_SET_PS_DATA_MOT NULL\n");
			return -EINVAL;
		}
		return 0;
	default:
		pr_err("unknown IOCTL: 0x%08x\n", cmd);
		return -ENOIOCTLCMD;
	}
	return 0;
}
#ifdef CONFIG_COMPAT
static long alsps_factory_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	long err = 0;

	void __user *arg32 = compat_ptr(arg);

	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;

	switch (cmd) {
	case COMPAT_ALSPS_SET_PS_MODE:
	case COMPAT_ALSPS_GET_PS_RAW_DATA:
	case COMPAT_ALSPS_SET_ALS_MODE:
	case COMPAT_ALSPS_GET_ALS_DATA://moto add
	case COMPAT_ALSPS_GET_ALS_RAW_DATA:
	case COMPAT_ALSPS_GET_PS_TEST_RESULT:
	case COMPAT_ALSPS_GET_PS_THRESHOLD_HIGH:
	case COMPAT_ALSPS_GET_PS_THRESHOLD_LOW:
	case COMPAT_ALSPS_SET_PS_THRESHOLD:
	case COMPAT_ALSPS_IOCTL_SET_CALI:
	case COMPAT_ALSPS_IOCTL_GET_CALI:
	case COMPAT_ALSPS_IOCTL_ALS_GET_CALI:
	case COMPAT_ALSPS_IOCTL_CLR_CALI:
	case COMPAT_ALSPS_ALS_ENABLE_CALI:
	case COMPAT_ALSPS_PS_ENABLE_CALI:
	case COMPAT_ALSPS_IOCTL_SELF_TEST:
	case COMPAT_ALSPS_IOCTL_ALS_SET_CALI:
	case COMPAT_ALSPS_SET_ALS_CALIDATA://moto add
		err = file->f_op->unlocked_ioctl(file, cmd,
						 (unsigned long)arg32);
		break;
	default:
		pr_err("unknown IOCTL: 0x%08x\n", cmd);
		err = -ENOIOCTLCMD;
		break;
	}

	return err;
}
#endif

static const struct file_operations _alsps_factory_fops = {
	.open = alsps_factory_open,
	.release = alsps_factory_release,
	.unlocked_ioctl = alsps_factory_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = alsps_factory_compat_ioctl,
#endif
};

static struct miscdevice alsps_factory_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "als_ps",
	.fops = &_alsps_factory_fops,
};

int alsps_factory_device_register(struct alsps_factory_public *dev)
{
	int err = 0;

	if (!dev || !dev->fops)
		return -1;
	alsps_factory.gain = dev->gain;
	alsps_factory.sensitivity = dev->sensitivity;
	alsps_factory.fops = dev->fops;
	err = misc_register(&alsps_factory_device);
	if (err) {
		pr_err("alsps_factory_device register failed\n");
		err = -1;
	}
	return err;
}

int alsps_factory_device_deregister(struct alsps_factory_public *dev)
{
	alsps_factory.fops = NULL;
	misc_deregister(&alsps_factory_device);
	return 0;
}
