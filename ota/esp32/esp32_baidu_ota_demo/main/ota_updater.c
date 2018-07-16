#include "sys_dep.h"
#include "ota_updater.h"
#include "ota_mq.h"


int ota_updater_init(OtaUpdater *apt, const char *cur_ver, OtaUpdaterCb cb, void* userData)
{
    if (!cur_ver || !cb) {
        return -1;
    }
    apt->cur_version = strdup(cur_ver);
    apt->cb = cb;
    apt->userData = userData;

    apt->ota_mq_handler =  get_ota_mq_handler(); //获得ota 升级操作对象
    if (!apt->ota_mq_handler) {
        printf("get_ota_mq_hander() error\n");
        goto ERR_FREE_VER;
    }
    if (apt->ota_mq_handler->ota_mq_begin(&apt->ota_ctx)) { //开始升级
        printf("ota_mq_begin() error\n");
        goto ERR_FREE_VER;
    }

    return 0;
ERR_FREE_VER:
    free(apt->cur_version);
    return -1;
}



static int compare_version(const char * cur_ver, const char *fw_ver)
{
    char curs[10];
    int cur;
    char fws[10];
    int fw;
    char *tmp;
    
    
    int i;
    char *ver_str;
    char* cur_ver_str;
    
    ver_str = fw_ver;
    cur_ver_str = cur_ver;
    
    for(i = 0; i < 2 ;i ++) {
        tmp = strchr(ver_str, '.');
        if(NULL == tmp) {
            printf("don't upgrade\n");
			return 0;
        }
        memcpy(fws, ver_str, tmp - ver_str);
        fws[tmp - ver_str] = '\0';
        fw = atoi(fws);
		printf("fw %d\n", fw);
        ver_str = tmp + 1;
        
        
        tmp = strchr(cur_ver_str, '.');
        if(NULL == tmp) {
            printf("do not update\n");
			return 0;   
        }
        memcpy(curs, cur_ver_str, tmp - cur_ver_str);
        curs[tmp - cur_ver_str] = '\0';
        cur = atoi(curs);    
		printf("cur %d\n", cur);
        cur_ver_str = tmp + 1;
        if(fw > cur) {
			printf("update\n");
            return 1;
        } else if(fw == cur) {
			printf("continue\n");
		} else {
			printf("do not update\n");
            return 0;			
		}
    } 
    fw = atoi(ver_str);
    cur = atoi(cur_ver_str);  
	printf("fw %d\n", fw);
	printf("cur %d\n", cur);
    if(fw > cur) {
		printf("[%s:%d]update\n", __FILE__, __LINE__);
        return 1;
    } else {
		printf("[%s:%d]do not update\n", __FILE__, __LINE__);
		return 0;   		
	}
	printf("[%s:%d]do not update\n", __FILE__, __LINE__);
    return 0;   
}

int ota_updater_set_fw_version(OtaUpdater *apt, const char *fw_version)
{
    if (!fw_version || !apt->cb) {
        goto ERR_OUT;
    }
    if (compare_version(apt->cur_version, fw_version) == 0) {
        apt->cb(apt, OTA_UPDATER_CB_FW_ALREADY_NEW, apt->userData);
    }
    return 0;
ERR_OUT:
    if (apt->cb) {
        apt->cb(apt, OTA_UPDATER_CB_ERR, apt->userData);
    }
    return -1;
}

int ota_updater_set_fw_size(OtaUpdater *apt, size_t fw_size)
{
    if (!apt->cb) {
        return -1;
    }
    apt->remain_len = apt->fw_size =  fw_size;
    return 0;
}


int ota_updater_eat_data(OtaUpdater* apt, uint8_t *data, size_t data_len)
{
    int len;
    unsigned char md5_cur[16];
    
    if (!apt->cb) {
        return -1;
    }
    
    //写ota二进制数据到 ota partition
    len = (apt->remain_len >= data_len) ? data_len : apt->remain_len;

    //计算 md5sum 值
    if (apt->needChksum) {
        md5_update(&apt->md5_ctx, data, len);
    }

    if (apt->ota_mq_handler->ota_mq_write_data(apt->ota_ctx, data, len) < 0) {
        printf("ota_mq_write_data() error\n");
        apt->cb(apt, OTA_UPDATER_CB_ERR, apt->userData);
        return -1;
    }
    printf("esp_ota_write success, progress: %d%%\n", 100 - 100 * apt->remain_len / apt->fw_size);
    apt->remain_len -= len;
    if (apt->remain_len <= 0) {
        printf("ota write finish!\n");
        //固件下载完毕，计算得到最终的 md5sum 值
        if (apt->needChksum) {
            md5_finish(&apt->md5_ctx, md5_cur); 

            //与 profile 中的值进行比较
            if (!memcmp(apt->md5_fw, md5_cur, 16)) { 
                printf("md5 check sum successfylly!\n");
                apt->cb(apt, OTA_UPDATER_FINISH, apt->userData);
            } else {
                printf("md5 check sum fail!\n");
                apt->cb(apt, OTA_UPDATER_CB_CHECKSUM_ERR, apt->userData);
            }
        } else {
            apt->cb(apt, OTA_UPDATER_FINISH, apt->userData);
        }
    }
    return 0;
}


static void hex_to_decimal(const char *hex_string, unsigned char *arr, size_t arr_len)
{
    const char *pos = hex_string;
    int count;

    for (count = 0; count < arr_len; count++) {
        sscanf(pos, "%2hhx", &arr[count]);
        pos += 2;
    }

    printf("0x");
    for(count = 0; count < arr_len; count++)
        printf("%02x", arr[count]);
    printf("\n");
}


int ota_updater_set_md5_chksum(OtaUpdater* apt, const char *md5_str)
{
    if (!apt->cb) {
        return -1;
    }
    if (!md5_str) {
        apt->needChksum = 0;
        return 0;
    }

    apt->needChksum = 1;
    
    // 将profile中的md5sum 16进制字符串转化成数组
    hex_to_decimal(md5_str, apt->md5_fw, sizeof(apt->md5_fw)); 

    //初始化 md5 ctx
    md5_init(&apt->md5_ctx);
    md5_starts(&apt->md5_ctx);
    return 0;
}

int ota_updater_end(OtaUpdater* apt, bool reboot)
{
    if (!apt->cb) {
        return -1;
    }
    //结束升级 清理缓存结构
    if (apt->needChksum) {
        md5_free(&apt->md5_ctx);
    }
    
    if (apt->ota_mq_handler->ota_mq_end(apt->ota_ctx, reboot) < 0) { //reboot
        printf("ota_mq_end() error\n");
    }
    if (apt->cur_version) {
        free(apt->cur_version);
    }
    memset(apt, 0, sizeof(*apt));
    return 0;
}
