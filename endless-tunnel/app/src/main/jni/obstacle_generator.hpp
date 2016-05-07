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
#ifndef endlesstunnel_obstacle_generator_hpp
#define endlesstunnel_obstacle_generator_hpp

#include "engine.hpp"
#include "obstacle.hpp"

// Generates obstacles given a difficulty level.
class ObstacleGenerator {
    private:
        int mDifficulty;
    public:
        ObstacleGenerator() {
            mDifficulty = 0;
        }

        void SetDifficulty(int dif) {
            mDifficulty = dif;
        }

        // generate a new obstacle.
        void Generate(Obstacle *result);

    private:
        void GenEasy(Obstacle *result);
        void GenMedium(Obstacle *result);
        void GenIntermediate(Obstacle *result);
        void GenHard(Obstacle *result);

        void FillRow(Obstacle *result, int row);
        void FillCol(Obstacle *result, int col);
};

#endif

