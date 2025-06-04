#ifndef __PAYLOAD_H_
#define __PAYLOAD_H_
#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(push, 1)

enum command {
    CMD_UNUSED,
    CMD_PUMP_WORK_VOLUME,
    CMD_PUMP_WORK_TIME,
    CMD_PUMP_CALLIBRATE,

    CMD_TOTAL
};

struct payload {
    unsigned long long timestamp;
    unsigned char command;
    unsigned long pin;
    double volume;
    unsigned long time;
};

#pragma pack(pop)
#ifdef __cplusplus
}
#endif
#endif