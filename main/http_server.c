#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "main.h"
#include "rmt.h"
#include "http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_netif.h"

#include "main.h"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)

static const char *REST_TAG = "esp-rest";

// 设备列表
device_info_t devices[MAX_DEVICE_LEN];
bool device_list_changed = false;
TimerHandle_t save_timer;
char scratch[SCRATCH_BUFSIZE];

cJSON *get_success_response(cJSON *data)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "code", 0);
    cJSON_AddStringToObject(root, "msg", "ok");
    cJSON_AddItemToObject(root, "data", data);
    return root;
}

cJSON *get_fail_response(char *msg)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "code", 1);
    cJSON_AddStringToObject(root, "msg", msg);
    return root;
}

esp_err_t send_json(httpd_req_t *req, cJSON *json)
{
    char *json_str = cJSON_Print(json);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);
    free(json_str);
    cJSON_Delete(json);
    return ESP_OK;
}

esp_err_t create_device(char *name, char *product)
{
    if (strlen(name) > 10 || strlen(product) > 10 || name == NULL || product == NULL)
    {
        return ESP_FAIL;
    }
    // 检查重名
    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (strcmp(devices[i].name, name) == 0)
        {
            return ESP_FAIL;
        }
    }
    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (devices[i].name[0] == 0)
        {
            strcpy(devices[i].name, name);
            strcpy(devices[i].product, product);
            device_list_changed = true;
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t delete_device(char *name)
{
    if (name == NULL)
    {
        return ESP_FAIL;
    }
    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (strcmp(devices[i].name, name) == 0)
        {
            memset(&devices[i], 0, sizeof(device_info_t));
            device_list_changed = true;
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t get_device_list(httpd_req_t *req)
{
    cJSON *data = cJSON_CreateArray();
    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (devices[i].name[0] == 0)
        {
            continue;
        }
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", devices[i].name);
        cJSON_AddStringToObject(item, "product", devices[i].product);
        cJSON_AddItemToArray(data, item);
    }
    return send_json(req, get_success_response(data));
}

esp_err_t bind_action(char *device_name, char *action_name, ir_nec_scan_code_t cmd)
{
    if (device_name == NULL || action_name == NULL)
    {
        return ESP_FAIL;
    }
    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (strcmp(devices[i].name, device_name) == 0)
        {
            for (int j = 0; j < MAX_ACTION_LEN; j++)
            {
                if (devices[i].actions[j].name[0] == 0)
                {
                    strcpy(devices[i].actions[j].name, action_name);
                    devices[i].actions[j].cmd = cmd;
                    device_list_changed = true;
                    return ESP_OK;
                }
            }
        }
    }
    return ESP_FAIL;
}

esp_err_t get_cmd_list(httpd_req_t *req)
{
    cJSON *data = cJSON_CreateArray();
    for (int i = 0; i < IR_CMD_RECORD_MAX; i++)
    {
        cJSON *item = cJSON_CreateObject();
        int index = (ir_cmd_record_index + i) % IR_CMD_RECORD_MAX;
        if (ir_cmd_record[index].time == 0)
        {
            continue;
        }
        cJSON_AddNumberToObject(item, "addr", ir_cmd_record[index].cmd.address);
        cJSON_AddNumberToObject(item, "cmd", ir_cmd_record[index].cmd.command);
        cJSON_AddNumberToObject(item, "time", ir_cmd_record[index].time);
        cJSON_AddItemToArray(data, item);
    }
    cJSON *root = get_success_response(data);
    return send_json(req, root);
}

cJSON *get_post_json(httpd_req_t *req)
{
    char *buf = NULL;
    int total_len = req->content_len;
    int cur_len = 0;
    int received = 0;
    REST_CHECK(total_len < MAX_POST_BODY_SIZE, "content too long", err);

    buf = calloc(total_len + 1, sizeof(char));
    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        REST_CHECK(received > 0, "Failed to post control value", err);
        cur_len += received;
    }
    buf[total_len] = '\0';
    ESP_LOGI(REST_TAG, "Received body: %s", buf);
    cJSON *root = cJSON_Parse(buf);
    free(buf);
    return root;
err:
    free(buf);
    return NULL;
}

esp_err_t create_device_handler(httpd_req_t *req)
{
    cJSON *root = get_post_json(req);
    if (root == NULL)
    {
        return send_json(req, get_fail_response("post data error"));
    }

    char *name = cJSON_GetStringValue(cJSON_GetObjectItem(root, "name"));
    char *product = cJSON_GetStringValue(cJSON_GetObjectItem(root, "product"));

    if (create_device(name, product) != ESP_OK)
    {
        cJSON_Delete(root);
        return send_json(req, get_fail_response("create device failed"));
    }

    cJSON_Delete(root);
    root = get_success_response(NULL);
    return send_json(req, root);
}

esp_err_t delete_device_handler(httpd_req_t *req)
{
    cJSON *root = get_post_json(req);
    if (root == NULL)
    {
        return send_json(req, get_fail_response("post data error"));
    }

    char *name = cJSON_GetObjectItem(root, "name")->valuestring;

    if (delete_device(name) != ESP_OK)
    {
        cJSON_Delete(root);
        return send_json(req, get_fail_response("delete device failed"));
    }

    cJSON_Delete(root);
    return send_json(req, get_success_response(NULL));
}

esp_err_t bind_action_handler(httpd_req_t *req)
{
    cJSON *root = get_post_json(req);
    if (root == NULL)
    {
        return send_json(req, get_fail_response("post data error"));
    }

    char *device_name = cJSON_GetObjectItem(root, "device")->valuestring;
    char *action_name = cJSON_GetObjectItem(root, "action")->valuestring;
    int addr = cJSON_GetObjectItem(root, "addr")->valueint;
    int cmd = cJSON_GetObjectItem(root, "cmd")->valueint;

    if (bind_action(device_name, action_name, (ir_nec_scan_code_t){.address = addr, .command = cmd}) != ESP_OK)
    {
        cJSON_Delete(root);
        return send_json(req, get_fail_response("bind action failed"));
    }

    cJSON_Delete(root);
    root = get_success_response(NULL);
    return send_json(req, root);
}

esp_err_t do_action_handler(httpd_req_t *req)
{
    cJSON *root = get_post_json(req);
    if (root == NULL)
    {
        return send_json(req, get_fail_response("post data error"));
    }

    char *device_name = cJSON_GetObjectItem(root, "device")->valuestring;
    char *action_name = cJSON_GetObjectItem(root, "action")->valuestring;

    for (int i = 0; i < MAX_DEVICE_LEN; i++)
    {
        if (strcmp(devices[i].name, device_name) == 0)
        {
            for (int j = 0; j < MAX_ACTION_LEN; j++)
            {
                if (strcmp(devices[i].actions[j].name, action_name) == 0)
                {
                    send_cmd(devices[i].actions[j].cmd);
                    cJSON_Delete(root);
                    return send_json(req, get_success_response(NULL));
                }
            }
        }
    }

    cJSON_Delete(root);
    return send_json(req, get_fail_response("do action failed"));
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "version", FIRMWARE_VERSION);
    cJSON_AddStringToObject(data, "chip", "ESP32-S3");
    cJSON_AddStringToObject(data, "sdk", esp_get_idf_version());

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);
    cJSON_AddStringToObject(data, "ip", ip4addr_ntoa(&ip_info.ip));
    cJSON *root = get_success_response(data);
    return send_json(req, root);
}

void save_timer_callback(TimerHandle_t xTimer)
{
    if (device_list_changed)
    {
        device_list_changed = false;
        // write to nvs
        ESP_LOGI(REST_TAG, "delay save device list");
        nvs_handle_t nvs_handle;
        ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));
        ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, "devices", devices, sizeof(devices)));
        ESP_ERROR_CHECK(nvs_commit(nvs_handle));
        nvs_close(nvs_handle);
    }
}

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html"))
    {
        type = "text/html";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".js"))
    {
        type = "application/javascript";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".css"))
    {
        type = "text/css";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".png"))
    {
        type = "image/png";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
    {
        type = "image/x-icon";
    }
    else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
    {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    strlcpy(filepath, WEB_MOUNT_POINT, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/')
    {
        strlcat(filepath, "/index.html", sizeof(filepath));
    }
    else
    {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1)
    {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    ssize_t read_bytes;
    do
    {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, scratch, SCRATCH_BUFSIZE);
        if (read_bytes == -1)
        {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        }
        else if (read_bytes > 0)
        {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, scratch, read_bytes) != ESP_OK)
            {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t start_rest_server()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    save_timer = xTimerCreate("save_timer", pdMS_TO_TICKS(30000), pdFALSE, (void *)0, save_timer_callback);
    REST_CHECK(save_timer != NULL, "create save timer failed", err);

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err);

    // 初始化设备列表
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));
    size_t len = sizeof(devices);
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_get_blob(nvs_handle, "devices", devices, &len));
    nvs_close(nvs_handle);

    // 获取设备信息
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/info",
                                           .method = HTTP_GET,
                                           .handler = system_info_get_handler,
                                       });

    // 获取命令列表
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/getCmdList",
                                           .method = HTTP_GET,
                                           .handler = get_cmd_list,
                                       });

    // 新建设备
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/createDevice",
                                           .method = HTTP_POST,
                                           .handler = create_device_handler,
                                       });

    // 删除设备
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/deleteDevice",
                                           .method = HTTP_POST,
                                           .handler = delete_device_handler,
                                       });

    // 获取设备列表
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/getDeviceList",
                                           .method = HTTP_GET,
                                           .handler = get_device_list,
                                       });

    // 绑定动作
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/bindAction",
                                           .method = HTTP_POST,
                                           .handler = bind_action_handler,
                                       });

    // 执行动作
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/api/rmt/doAction",
                                           .method = HTTP_POST,
                                           .handler = do_action_handler,
                                       });

    /* URI handler for getting web server files */
    httpd_register_uri_handler(server, &(httpd_uri_t){
                                           .uri = "/*",
                                           .method = HTTP_GET,
                                           .handler = rest_common_get_handler,
                                       });

    return ESP_OK;
err:
    return ESP_FAIL;
}
