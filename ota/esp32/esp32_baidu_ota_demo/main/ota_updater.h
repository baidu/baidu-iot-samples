#ifndef __OTA_UPDATER_H__
#define __OTA_UPDATER_H__

#include "sys_dep.h"
#include "ota_mq.h"
#include "md5.h"
typedef struct _OtaUpdater OtaUpdater;

typedef enum {
	OTA_UPDATER_CB_ERR = -1,
	OTA_UPDATER_CB_FW_ALREADY_NEW,
	OTA_UPDATER_CB_CHECKSUM_ERR,
	OTA_UPDATER_FINISH,
}OtaUpdaterCbSt;

typedef void(*OtaUpdaterCb)(OtaUpdater* apt, OtaUpdaterCbSt st, void* userData);

struct _OtaUpdater {
    OtaMqHandler* ota_mq_handler;
	void *ota_ctx;
	int needChksum;
	md5_context md5_ctx;
	unsigned char md5_fw[16];

	char *cur_version;
	size_t fw_size;
	size_t remain_len;
	
	OtaUpdaterCb cb;
	void *userData;
};


int ota_updater_init(OtaUpdater *apt, const char *cur_ver, OtaUpdaterCb cb, void* userData);
int ota_updater_set_fw_version(OtaUpdater *apt, const char *fw_ver);
int ota_updater_set_fw_size(OtaUpdater *apt, size_t fw_len);
int ota_updater_set_md5_chksum(OtaUpdater* apt, const char *md5_str);
int ota_updater_eat_data(OtaUpdater* apt, uint8_t *bin_data, size_t data_len);
int ota_updater_end(OtaUpdater* apt, bool reboot);

#endif