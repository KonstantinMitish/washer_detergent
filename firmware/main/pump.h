/*************************************************************
 * Copyright (C) 2025
 *    Konstantin Mitish
 *************************************************************/

/* FILE NAME   : pump.c
 * PURPOSE     : Pump logic handler
 * PROGRAMMER  : KM6.
 * LAST UPDATE : 06.06.2025.
 *
 * No part of this file may be changed without agreement of
 * Konstantin Mitish
 */
#ifndef __PUMP_H_
#define __PUMP_H_

#include <stdbool.h>
#include <sys/types.h>

struct pump_data {
    double speed;
};

bool pump_init();

bool pump_callibrate(int pin, double new_speed);

bool pump_work_time(int pin, uint32_t time_ms);

bool pump_work_volume(int pin, double volume);

#endif /* __PUMP_H_ */