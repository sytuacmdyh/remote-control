#ifndef M_MAIN_H
#define M_MAIN_H

#define FIRMWARE_VERSION "V1.0.0"

#define MDNS_INSTANCE "esp home web server"
#define WEB_MOUNT_POINT "/www"
#define MDNS_HOST_NAME "rt-ctrl"

#define PD_CHECK(a, str, goto_tag, ...)                                             \
    do                                                                              \
    {                                                                               \
        if (!(a))                                                                   \
        {                                                                           \
            ESP_LOGE("YZC", "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                          \
        }                                                                           \
    } while (0)

#endif
