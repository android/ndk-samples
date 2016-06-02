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
#ifndef endlesstunnel_game_consts_h
#define endlesstunnel_game_consts_h

// Render settings
#define RENDER_FOV 45.0f
#define RENDER_NEAR_CLIP 0.1f
#define RENDER_FAR_CLIP 200.0f

// Size of the tunnel
#define TUNNEL_HALF_W 10.0f
#define TUNNEL_HALF_H 10.0f

// length of each tunnel section
#define TUNNEL_SECTION_LENGTH 150.0f

// number of tunnel sections to render ahead
#define RENDER_TUNNEL_SECTION_COUNT 4

// An obstacle is a grid of boxes. This indicates how many boxes by how many boxes this grid is.
#define OBS_GRID_SIZE 5

// This is how wide each of the grid cells are
#define OBS_CELL_SIZE ((2*TUNNEL_HALF_W)/(float)OBS_GRID_SIZE)

// size of each obstacle box
#define OBS_BOX_SIZE (0.8f * OBS_CELL_SIZE)

// size of bonus box
#define OBS_BONUS_SIZE (0.3f * OBS_CELL_SIZE)

// at what distance (in tunnel sections) from the start position do obstacles start to appear?
#define OBS_START_SECTION 4

// once a tunnel section is this far behind the player, delete it
#define SHIFT_THRESH 20.0f

// maximum delta T between two frames
#define MAX_DELTA_T 0.05f

// player's speed
#define PLAYER_SPEED 80.0f

// how much the player's speed increases when we go up one difficulty level
#define PLAYER_SPEED_INC_PER_LEVEL 10.0f

// player's lateral speed
#define PLAYER_MAX_LAT_SPEED 40.0f

// bounds on player's movement
#define PLAYER_MAX_X TUNNEL_HALF_W - 1.0f
#define PLAYER_MIN_X -(PLAYER_MAX_X)
#define PLAYER_MAX_Z TUNNEL_HALF_H - 1.0f
#define PLAYER_MIN_Z -(PLAYER_MAX_Z)

// touch control sensivity (ship displacement caused by dragging the screen by a length
// equivalent to its height).
#define TOUCH_CONTROL_SENSIVITY (TUNNEL_HALF_W * 5)

// joystick control sensivity (maximum velocity attained per axis)
#define JOYSTICK_CONTROL_SENSIVITY 20.0f

// how many points equal a raise in difficulty level?
#define SCORE_PER_LEVEL 500

// settings for rendering the score to the screen
#define SCORE_POS_X 0.15f
#define SCORE_POS_Y 0.92f
#define SCORE_FONT_SCALE 0.8f


// scale of the signs that appear onscreen
#define SIGN_FONT_SCALE 0.9f

// scale of menu items
#define MENUITEM_FONT_SCALE 0.8f

// how long do signs stay onscreen
#define SIGN_DURATION 2.0f
#define SIGN_DURATION_BONUS 1.0f
#define SIGN_DURATION_GAME_OVER 5.0f

// duration of a text sign's zoom-in animation
#define SIGN_ANIM_DUR 0.2f

// how big the life symbol (heart) is
#define LIFE_ICON_SCALE 0.004f
#define LIFE_LINE_WIDTH 3

// where do we start drawing the life icons? (negative to mean counting from right side of screen)
#define LIFE_POS_X -0.4f
#define LIFE_POS_Y SCORE_POS_Y
#define LIFE_SPACING_X 0.08f
#define LIFE_SCALE_Y 1.5f

// how many lives the player has
#define PLAYER_LIVES 4

// how many points player gets for picking up a bonus
#define BONUS_POINTS 50

// roll speeds for each level (how fast the chamber turns)
#define ROLL_SPEEDS { 0.0f, 0.1f, 0.0f, -0.1f, 0.0f, 0.2f, 0.0f, -0.2f }

// recipes for synthesizing our very advanced sound effects:
#define TONE_LEVEL_UP "d100 f500. f600. f700. f600. f700. f800."
#define TONE_CRASHED "a100 d15 f0. a40 d75 f0. a30 f0. a20 f0. a70 d100 f400. a0. a70. a0. a70."
#define TONE_GAME_OVER "a100 d15 f0. a40 d75 f0. a30 f0. a20 f0. a70 d200 f400. a0. f350 a70. " \
        "a0. f300 a70. a0. f250 a70. a0. f200 a70."
#define TONE_AMBIENT_0 "d100 f300."
#define TONE_AMBIENT_1 "d100 f200."

// player's acceleration, in units per second squared
#define PLAYER_ACCELERATION_NEGATIVE_SPEED 10.0f  // used when speed is negative
#define PLAYER_ACCELERATION_POSITIVE_SPEED 40.0f  // used when speed is positive

// the speed after colliding with an obstacle
#define PLAYER_SPEED_AFTER_COLLISION -20.0f

// how far beind the obstacle the player is placed upon collision
#define PLAYER_RECEDE_AFTER_COLLISION 2.0f

// how long the heart meter blinks for when you've just lost a life
#define BLINKING_HEART_DURATION 2.0f

// how long does the game take to end after we show Game Over
#define GAME_OVER_EXPIRE 5.0f

// UI transition animation duration
#define TRANSITION_DURATION 0.25f

// for the purposes of testing for close calls with obstacles, by how much do we move
// the player to test for collisions?
#define CLOSE_CALL_CALC_DELTA (OBS_CELL_SIZE*0.2f)

// menu item pulse animation settings
#define MENUITEM_PULSE_AMOUNT 1.1f
#define MENUITEM_PULSE_PERIOD 0.5f

// save file name
#define SAVE_FILE_NAME "tunnel.dat"

// checkpoint (save progress) every how many levels?
#define LEVELS_PER_CHECKPOINT 4

#endif

