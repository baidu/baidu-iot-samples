
#ifndef __BAIDU_IOT_OTA_MQ_H___
#define __BAIDU_IOT_OTA_MQ_H___


typedef struct {
    /**
     * Begin ota update
     * @param[in, out] ctx user defined ctx structure, used for stroing some state info
     * @retval 0: success, -1: fail
     */
    int (*ota_mq_begin)(void **ctx);

    /**
     * Write binary data to ota partition in flash
     * @param[in] ctx      user defined ctx structure, used for stroing some state info
     * @param[in] data     ota binary data to write
     * @param[in] data_len data len need to write
     * @retval 0: success, -1: fail
     */
    int (*ota_mq_write_data)(void *ctx, uint8_t *data, size_t data_len);

    /**
     * End this ota operation, you need to do some cleaning here
     * @param[in] ctx      user defined ctx structure, used for stroing some state info
     * @param[in] reboot   flag indicates whether or not reboot system after finishing ota operation
     * @retval 0: success, -1: fail
     */
    int (*ota_mq_end)(void *ctx, bool reboot);
} OtaMqHandler;


/**
 * Get ota mq handler.
 * @retval OtaMqHandler if success, or NULL indicate failure
 */
OtaMqHandler* get_ota_mq_handler();

#endif


