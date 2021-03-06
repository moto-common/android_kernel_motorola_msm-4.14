/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"msm-dsi-display:[%s] " fmt, __func__

#include <linux/list.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/err.h>

#include "msm_drv.h"
#include "sde_connector.h"
#include "msm_mmu.h"
#include "dsi_display.h"
#include "dsi_panel.h"
#include "dsi_ctrl.h"
#include "dsi_ctrl_hw.h"
#include "dsi_drm.h"
#include "dsi_clk.h"
#include "dsi_pwr.h"
#include "sde_dbg.h"
#include "dsi_parser.h"
#include "dsi_display_mot_ext.h"

static struct dsi_display_early_power *g_early_power = NULL;

static void dsi_display_early_power_on_work(struct work_struct *work)
{
	struct dsi_display *display = NULL;
	struct dsi_display_early_power *early_power = NULL;
	int rc = 0;

	pr_info("+++++ early_power_state: %d\n", (g_early_power!=NULL)?g_early_power->early_power_state:-1);
	early_power = container_of(work, struct dsi_display_early_power, early_on_work);
	if (!early_power) {
		pr_warning("failed to get early_power\n");
		return;
	}
	__pm_wakeup_event(&early_power->early_wake_src, 500);
	display = early_power->display;
	if (!display || !display->panel || !display->is_primary ||
	    (display->panel->panel_mode != DSI_OP_VIDEO_MODE) ||
	    atomic_read(&display->panel->esd_recovery_pending)) {
		pr_warning("Invalid recovery use case, early_power_state: %d\n", early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}

	mutex_lock(&display->display_lock);
	if (display->is_dsi_display_prepared) {
		pr_warning("already prepared, early_power_state: %d\n", early_power->early_power_state);
		mutex_unlock(&display->display_lock);
		__pm_relax(&early_power->early_wake_src);
		return;
	}
	mutex_unlock(&display->display_lock);

	early_power->early_power_state = DSI_EARLY_POWER_BEGIN;
	rc = dsi_display_prepare(display);
	if (rc) {
		pr_err("DSI display prepare failed, rc=%d. early_power_state %d\n", rc, early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}

	mutex_lock(&display->display_lock);
	if (display->panel->panel_initialized) {
		mutex_unlock(&display->display_lock);
		pr_warning("already enabled, early_power_state: %d\n", early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}
	mutex_unlock(&display->display_lock);

	early_power->early_power_state = DSI_EARLY_POWER_PREPARED;
	rc = dsi_display_enable(display);
	if (rc) {
		pr_err("DSI display enable failed, rc=%d. early_power_state %d\n", rc, early_power->early_power_state);
		(void)dsi_display_unprepare(display);
	}
	early_power->early_power_state = DSI_EARLY_POWER_INITIALIZED;
	queue_delayed_work(early_power->early_power_workq, &early_power->early_off_work, HZ);
	__pm_relax(&early_power->early_wake_src);

	pr_info("----- early_power_state: %d\n", early_power->early_power_state);
}

static void dsi_display_early_power_off_work(struct work_struct *work)
{
	struct dsi_display *display = NULL;
	struct dsi_display_early_power *early_power = NULL;
	int rc = 0;
	struct delayed_work *dw = to_delayed_work(work);

	pr_info("+++++ early_power_state: %d\n", (g_early_power!=NULL)?g_early_power->early_power_state:-1);
	early_power = container_of(dw, struct dsi_display_early_power, early_off_work);
	if (!early_power) {
		pr_warning("failed to get early_power\n");
		return;
	}
	__pm_wakeup_event(&early_power->early_wake_src, 500);
	display = early_power->display;
	if (!display || !display->panel ||
	    (display->panel->panel_mode != DSI_OP_VIDEO_MODE) ||
	    atomic_read(&display->panel->esd_recovery_pending)) {
		pr_warning("Invalid recovery use case, early_power_state: %d\n", early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}
	if (early_power->early_power_state != DSI_EARLY_POWER_INITIALIZED) {
		pr_info("%s ----- no need, return.  early_power_state %d\n", __func__, early_power->early_power_state);
		//early_power->early_power_state = DSI_EARLY_POWER_IDLE;
		__pm_relax(&early_power->early_wake_src);
		return;
	}

	rc = dsi_display_disable(display);
	if (rc) {
		pr_err("DSI display disable failed, rc=%d. early_power_state %d\n", rc, early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}

	mutex_lock(&display->display_lock);
	if (early_power->early_power_state == DSI_EARLY_POWER_FORBIDDEN) {
		pr_warning("----- set_mode called, early_power_state: %d, is_dsi_display_prepared %d\n",
			early_power->early_power_state, display->is_dsi_display_prepared);
		mutex_unlock(&display->display_lock);
		__pm_relax(&early_power->early_wake_src);
		return;
	}
	mutex_unlock(&display->display_lock);

	rc = dsi_display_unprepare(display);
	if (rc) {
		pr_err("DSI display unprepare failed, rc=%d. early_power_state %d\n", rc, early_power->early_power_state);
		__pm_relax(&early_power->early_wake_src);
		return;
	}

	mutex_lock(&display->display_lock);
	if (early_power->early_power_state != DSI_EARLY_POWER_FORBIDDEN) {
		g_early_power->early_power_state = DSI_EARLY_POWER_IDLE;
	}
	mutex_unlock(&display->display_lock);

	__pm_relax(&early_power->early_wake_src);

	pr_info("----- early_power_state: %d\n", early_power->early_power_state);
}

void ext_dsi_display_early_power_on(void)
{
	// If previous work has not finished, skip
	if (g_early_power == NULL || g_early_power->early_power_state == DSI_EARLY_POWER_FORBIDDEN) {
		//pr_info("==== g_early_power %p, early_power_state: %d\n",
		//	g_early_power, g_early_power->early_power_state);
		return;
	}
        if (g_early_power->early_power_state != DSI_EARLY_POWER_IDLE) {
		//pr_info("==== early_power_state: %d is not IDLE\n",
		//	g_early_power->early_power_state);
		return;
        }

	//pr_info("#### display(%p), early_power_state: %d\n", g_early_power->display, g_early_power->early_power_state);
	queue_work(g_early_power->early_power_workq, &g_early_power->early_on_work);
}
EXPORT_SYMBOL(ext_dsi_display_early_power_on);

static ssize_t dsi_display_early_power_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct dsi_display *display;

	display = dev_get_drvdata(dev);
	if (!display) {
		pr_err("Invalid display\n");
		return -EINVAL;
	}

	rc = snprintf(buf, PAGE_SIZE, "%d\n", display->early_power.early_power_state);
	pr_info("%s: early_power_state %d\n", __func__,
		display->early_power.early_power_state);

	return rc;
}

static ssize_t dsi_display_early_power_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc = 0;
	struct dsi_display *display;

	display = dev_get_drvdata(dev);
	if (!display) {
		pr_err("Invalid display\n");
		return -EINVAL;
	}

	if (display->panel->panel_initialized) {
		pr_err("panel already initialized\n");
		return -ENOTSUPP;
	}
	pr_info("%s: early_power_state %d\n", __func__,
		display->early_power.early_power_state);

	ext_dsi_display_early_power_on();

	return rc;

}

static DEVICE_ATTR(dsi_display_early_power, 0644,
			dsi_display_early_power_read,
			dsi_display_early_power_write);

static struct attribute *dsi_display_early_power_fs_attrs[] = {
	&dev_attr_dsi_display_early_power.attr,
	NULL,
};
static struct attribute_group dsi_display_early_power_fs_attrs_group = {
	.attrs = dsi_display_early_power_fs_attrs,
};

static int dsi_display_sysfs_ext_init(struct dsi_display *display)
{
	int rc = 0;
	struct device *dev = &display->pdev->dev;

	rc = sysfs_create_group(&dev->kobj,
		&dsi_display_early_power_fs_attrs_group);

	return rc;

}

static int dsi_display_sysfs_ext_deinit(struct dsi_display *display)
{
	struct device *dev = &display->pdev->dev;

	sysfs_remove_group(&dev->kobj,
		&dsi_display_early_power_fs_attrs_group);

	return 0;

}

int dsi_display_ext_init(struct dsi_display *display)
{
	int rc = 0;

	g_early_power = &display->early_power;
	g_early_power->display = display;

	dsi_display_sysfs_ext_init(display);

	display->is_dsi_display_prepared = false;
	display->is_primary = false;
	g_early_power->early_power_state = DSI_EARLY_POWER_STATE_NUM;
	g_early_power->early_power_workq = create_singlethread_workqueue("dsi_early_power_workq");
	if (!g_early_power->early_power_workq) {
		pr_err("failed to create dsi early power workq!\n");
		dsi_display_sysfs_ext_deinit(display);
		return 0;
	}

	wakeup_source_init(&g_early_power->early_wake_src, "dsi_early_wakelock");

	INIT_WORK(&g_early_power->early_on_work,
				dsi_display_early_power_on_work);
	INIT_DELAYED_WORK(&g_early_power->early_off_work,
				dsi_display_early_power_off_work);
	return rc;
}

