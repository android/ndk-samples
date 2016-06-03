/*
 * Copyright (C) Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "obstacle.hpp"

#define BONUS_PROBABILITY 0.7f

void Obstacle::PutRandomBonus() {
    if (Random(100) * 0.01f > BONUS_PROBABILITY) {
        return;
    }

    bool candidate[OBS_GRID_SIZE][OBS_GRID_SIZE];
    memset(candidate, 0, sizeof(candidate));

    int r, c;
    int i, j;

    // (This is the most deeply indented code I have written in my life)
    // It goes through the grid and marks all the squares that are adjacent to
    // a solid square as candidates for the bonus.
    for (r = 0; r < OBS_GRID_SIZE; r++) {
        for (c = 0; c < OBS_GRID_SIZE; c++) {
            if (grid[c][r]) {
                for (i = r - 1; i <= r + 1; i++) {
                    for (j = c - 1; j <= c + 1; j++) {
                        if (i >= 0 && i < OBS_GRID_SIZE && j >= 0 && j < OBS_GRID_SIZE) {
                            candidate[j][i] = true;
                        }
                    }
                }
            }
        }
    }

    // now we randomly choose one of the candidates
    int r0 = Random(0, OBS_GRID_SIZE);
    int c0 = Random(0, OBS_GRID_SIZE);
    int rd, cd;
    bonusRow = bonusCol = -1;
    for (rd = 0; rd < OBS_GRID_SIZE && bonusRow < 0; rd++) {
        for (cd = 0; cd < OBS_GRID_SIZE; cd++) {
            int my_r = (r0 + rd) % OBS_GRID_SIZE;
            int my_c = (c0 + cd) % OBS_GRID_SIZE;
            if (!grid[my_c][my_r] && candidate[my_c][my_r]) {
                bonusRow = my_r;
                bonusCol = my_c;
                break;
            }
        }
    }
}

