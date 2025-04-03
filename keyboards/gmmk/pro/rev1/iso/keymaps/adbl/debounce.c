#include "quantum.h"

// Stores the last stable time for each key
static uint16_t key_timer[MATRIX_ROWS][MATRIX_COLS];
#define SPACE_DEBOUNCE_TIME 10  // ms
#define DEFAULT_DEBOUNCE_TIME 5  // ms

void debounce_init(uint8_t num_rows) {
    // Initialize all timers to 0
    for (uint8_t row = 0; row < num_rows; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            key_timer[row][col] = 0;
        }
    }
}

bool debounce(matrix_row_t raw[], matrix_row_t cooked[], bool changed[], uint8_t num_rows) {
    bool did_change = false;
    uint16_t now = timer_read();

    for (uint8_t row = 0; row < num_rows; row++) {
        matrix_row_t row_raw = raw[row];
        matrix_row_t row_cooked = cooked[row];
        matrix_row_t row_changed = 0;

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            bool was_pressed = row_cooked & (1 << col);
            bool is_pressed = row_raw & (1 << col);

            if (was_pressed != is_pressed) {
                uint16_t debounce_time = DEFAULT_DEBOUNCE_TIME;

                // Check if this key is spacebar
                if (keymap_key_to_keycode(0, (keypos_t){ row, col }) == KC_SPC) {
                    debounce_time = SPACE_DEBOUNCE_TIME;
                }

                if (TIMER_DIFF_16(now, key_timer[row][col]) > debounce_time) {
                    key_timer[row][col] = now;
                    if (is_pressed) {
                        cooked[row] |= (1 << col);
                    } else {
                        cooked[row] &= ~(1 << col);
                    }
                    row_changed |= (1 << col);
                    did_change = true;
                }
            } else {
                // Update timer only if no change
                key_timer[row][col] = now;
            }
        }

        changed[row] = row_changed;
    }

    return did_change;
}