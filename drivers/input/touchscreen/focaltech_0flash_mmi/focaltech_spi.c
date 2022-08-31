#include "focaltech_core.h"
#include "focaltech_flash.h"

int fts_write(u8 *writebuf, u32 writelen) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_write_v1_1_1(writebuf, writelen);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_write_v1_1(writebuf, writelen);
	return ret;
}

int fts_write_reg(u8 addr, u8 value) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_write_reg_v1_1_1(addr, value);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_write_reg_v1_1(addr, value);
	return ret;
}

int fts_read(u8 *cmd, u32 cmdlen, u8 *data, u32 datalen) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_read_v1_1_1(cmd, cmdlen, data, datalen);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_read_v1_1(cmd, cmdlen, data, datalen);
	return ret;
}

int fts_read_reg(u8 addr, u8 *value) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_read_reg_v1_1_1(addr, value);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_read_reg_v1_1(addr, value);
	return ret;
}

int fts_bus_init(struct fts_ts_data *ts_data) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_bus_init_v1_1_1(ts_data);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_bus_init_v1_1(ts_data);
	return ret;
}

int fts_bus_exit(struct fts_ts_data *ts_data) {
	int ret;
	if ((strcmp(fts_chip_name, "ft8756") == 0) || (strcmp(fts_chip_name, "ft8009")) == 0)
		ret = fts_bus_exit_v1_1_1(ts_data);
	else if ((strcmp(fts_chip_name, "ft8719")) == 0)
		ret = fts_bus_exit_v1_1(ts_data);
	return ret;
}
