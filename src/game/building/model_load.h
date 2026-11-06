// Loading the model files (building/model_load.c): the building, house, figure, enemy and tax sentiment models.

#ifndef BUILDING_MODEL_LOAD_H
#define BUILDING_MODEL_LOAD_H

#include <stdint.h>

int32_t building_model_load(void);
int32_t figure_model_load(void);
int32_t tax_sentiment_model_load(void);

#endif
