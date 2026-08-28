#include "main.h"

int32_t interpolate(mdatainterpolate_t *D)
{
    uint16_t dy1 = 0, dy2 = 0, dx1 = 0, dx2 = 0;

    // Clamp X (column) to valid range
    D->x = D->real_column;
    if (D->x < (int16_t)*(D->data_column))
        D->x = (int16_t)*(D->data_column);
    else if (D->x > (int16_t)*(D->data_column + (D->max_column - 1)))
        D->x = (int16_t)*(D->data_column + (D->max_column - 1));

    // Clamp Y (row) to valid range
    D->y = D->real_row;
    if (D->y < (int16_t)*(D->data_row + (D->max_row - 1)))
        D->y = (int16_t)*(D->data_row + (D->max_row - 1));
    else if (D->y > (int16_t)*(D->data_row))
        D->y = (int16_t)*(D->data_row);

    // Find X indices (column)
    D->ix1 = (int16_t)*(D->data_column);
    for (uint8_t i = 0; i < D->max_column; i++)
    {
        if ((int16_t)*(D->data_column + i) > (int32_t)D->real_column)
        {
            D->ix2 = (int16_t)*(D->data_column + i);
            dx2 = i;
            break;
        }
        else
        {
            D->ix1 = (int16_t)*(D->data_column + i);
            dx1 = i;
            D->ix2 = (int16_t)*(D->data_column + i);
            dx2 = i;
        }
    }

    // Find Y indices (row)
    D->iy1 = *((uint16_t *)D->data_row + D->max_row - 1);
    dy1 = D->max_row - 1;
    for (uint8_t i = D->max_row - 1; i > 0; i--)
    {
        if ((int16_t)*(D->data_row + i) > (int16_t)D->real_row)
        {
            D->iy2 = (int16_t)*(D->data_row + i);
            dy2 = i;
            break;
        }
        else
        {
            D->iy1 = (int16_t)*(D->data_row + i);
            dy1 = i;
            D->iy2 = (int16_t)*(D->data_row + i);
            dy2 = i;
        }
    }

    // Get corner values from table
    D->v11 = (int16_t)*((uint8_t *)D->datatune + (dy1 * D->max_column) + dx1);
    D->v12 = (int16_t)*((uint8_t *)D->datatune + (dy1 * D->max_column) + dx2);
    D->v21 = (int16_t)*((uint8_t *)D->datatune + (dy2 * D->max_column) + dx1);
    D->v22 = (int16_t)*((uint8_t *)D->datatune + (dy2 * D->max_column) + dx2);

    // Calculate fractional positions
    if (D->ix2 != D->ix1)
        D->subX = (float)(D->x - D->ix1) / (float)(D->ix2 - D->ix1);
    else
        D->subX = D->x - D->ix1;

    if (D->iy2 != D->iy1)
        D->subY = (float)(D->y - D->iy1) / (float)(D->iy2 - D->iy1);
    else
        D->subY = (float)D->y - D->iy1;

    // Bilinear interpolation. Keep fractional values until the final result.
    float row1 = (float)D->v11 + ((float)(D->v21 - D->v11) * D->subY);
    float row2 = (float)D->v12 + ((float)(D->v22 - D->v12) * D->subY);
    float result = row1 + ((row2 - row1) * D->subX);

    return (int32_t)(result + 0.5F);
}

short calInterPolate2D(void)
{
    mdatainterpolate_t table;

    table.data_column = pump.xrpm;
    table.data_row = pump.yduty;
    table.datatune = (uint16_t *)pump.mem_save.tabletune;
    table.max_column = 12;
    table.max_row = 6;
    table.real_column = pump.rpm;
    table.real_row = pump.duty;
    
    return (short)interpolate(&table);
}