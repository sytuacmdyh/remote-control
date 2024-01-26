#ifndef M_RMT_H
#define M_RMT_H

#include "esp_err.h"
#include "ir_nec_encoder.h"

#define IR_CMD_RECORD_MAX 20

typedef struct
{
    ir_nec_scan_code_t cmd;
    time_t time;
} ir_cmd_record_item_t;

extern ir_cmd_record_item_t ir_cmd_record[IR_CMD_RECORD_MAX];
extern uint8_t ir_cmd_record_index;

void initialise_rmt();

esp_err_t send_cmd(ir_nec_scan_code_t cmd);

#endif
