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
#ifndef endlesstunnel_ascii_to_geom_hpp
#define endlesstunnel_ascii_to_geom_hpp

#include "engine.hpp"

/* Converts ASCII art into a Vbo/Ibo pair. Useful for retro-looking drawings/text!
 * scale is the size of each character. The center of the rendering will be 0,0.
 *
 * Examples:
 *
 * Square:
 *     +---+
 *     |   |
 *     +---+
 *
 * Triangle:
 *     +-----+
 *      `   /
 *       ` /
 *        +
 *
 * The + sign represents a vertex; lines are represented by -, /, ` and |.
 */
SimpleGeom* AsciiArtToGeom(const char *art, float scale);

#endif

