#ifndef __PAYLOAD_H_
#define __PAYLOAD_H_
#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
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
    uint64_t timestamp;
    uint8_t  command;
    uint32_t pin;
    double   volume;
    uint32_t time;
};

#pragma pack(pop)
#ifdef __cplusplus
}
#endif
#endif