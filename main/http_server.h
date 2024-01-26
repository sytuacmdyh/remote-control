#ifndef M_HTTP_SERVER_H
#define M_HTTP_SERVER_H

#include "esp_system.h"
#include "rmt.h"

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

#define MAX_POST_BODY_SIZE 10240
#define MAX_DEVICE_LEN 10
#define MAX_ACTION_LEN 20
#define SCRATCH_BUFSIZE 10240

typedef struct
{
    char name[10];
    ir_nec_scan_code_t cmd;
} action_t;

typedef struct
{
    char name[10];
    char product[10];
    action_t actions[20];
} device_info_t;

esp_err_t start_rest_server();

#endif // M_HTTP_SERVER_H
