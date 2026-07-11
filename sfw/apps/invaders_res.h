#ifndef INVADERS_RES_H
#define INVADERS_RES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t type;
    bool on;
}aliens;

extern const uint16_t *en[];
extern const uint16_t crab_a[];
extern const uint16_t crab_b[];
extern const uint16_t ufo_a[];
extern const uint16_t ufo_b[];
extern const uint16_t octopus_a[];
extern const uint16_t octopus_b[];
extern const uint16_t shuttle_a[];
extern const uint16_t shuttle_b[];
extern const uint16_t bul_a[];
extern const uint16_t bul_b[];

#endif