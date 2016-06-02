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
#ifndef _mygame_alphabet_inl
#define _mygame_alphabet_inl

#define ALPHABET_GLYPH_COLS 5 
#define ALPHABET_GLYPH_ROWS 9 

static const char *ALPHABET_ART[] = {
    NULL, // chr 0
    NULL, // chr 1
    NULL, // chr 2
    NULL, // chr 3
    NULL, // chr 4
    NULL, // chr 5
    NULL, // chr 6
    NULL, // chr 7
    NULL, // chr 8
    NULL, // chr 9
    NULL, // chr 10
    NULL, // chr 11
    NULL, // chr 12
    NULL, // chr 13
    NULL, // chr 14
    NULL, // chr 15
    NULL, // chr 16
    NULL, // chr 17
    NULL, // chr 18
    NULL, // chr 19
    NULL, // chr 20
    NULL, // chr 21
    NULL, // chr 22
    NULL, // chr 23
    NULL, // chr 24
    NULL, // chr 25
    NULL, // chr 26
    NULL, // chr 27
    NULL, // chr 28
    NULL, // chr 29
    NULL, // chr 30
    NULL, // chr 31
    NULL, // chr 32
    "+---+\n" // chr 33, !
    "|   |\n"
    "+   +\n"
    " ` / \n"
    "  +  \n"
    " +-+ \n"
    " | | \n"
    " +-+ \n"
    "     \n",
    NULL, // chr 34, "
    NULL, // chr 35, #
    NULL, // chr 36, $
    NULL, // chr 37, %
    NULL, // chr 38, &
    "  +  \n" // chr 39, '
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n",
    NULL, // chr 40, (
    NULL, // chr 41, )
    NULL, // chr 42, *
    "     \n" // chr 43, +
    "  +  \n"
    "  |  \n"
    "+-+-+\n"
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n"
    "     \n",
    "     \n" // chr 44, ,
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "   + \n"
    "  /  \n"
    " +   \n"
    "     \n",
    "     \n" // chr 45, -
    "     \n"
    "     \n"
    "+---+\n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n",
    "     \n" // chr 46, .
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    " +-+ \n"
    " | | \n"
    " +-+ \n"
    "     \n",
    "     \n" // chr 47, /
    "     \n"
    "    +\n"
    "   / \n"
    "  /  \n"
    " /   \n"
    "+    \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 48, 0
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "   + \n" // chr 49, 1
    "   | \n"
    "   | \n"
    "   | \n"
    "   | \n"
    "   | \n"
    "   + \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 50, 2
    "    |\n"
    "    |\n"
    "+---+\n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 51, 3
    "    |\n"
    "    |\n"
    "+---+\n"
    "    |\n"
    "    |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+   +\n" // chr 52, 4
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "    |\n"
    "    |\n"
    "    +\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 53, 5
    "|    \n"
    "|    \n"
    "+---+\n"
    "    |\n"
    "    |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 54, 6
    "|    \n"
    "|    \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 55, 7
    "    |\n"
    "    |\n"
    "    |\n"
    "    |\n"
    "    |\n"
    "    +\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 56, 8
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 57, 9
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "    |\n"
    "    |\n"
    "+---+\n"
    "     \n"
    "     \n",
    " +-+ \n" // chr 58, :
    " | | \n"
    " +-+ \n"
    "     \n"
    " +-+ \n"
    " | | \n"
    " +-+ \n"
    "     \n"
    "     \n",
    NULL, // chr 59, ;
    NULL, // chr 60, <
    NULL, // chr 61, =
    NULL, // chr 62, >
    "+---+\n" // chr 63, ?
    "    |\n"
    "  +-+\n"
    "  |  \n"
    "  +  \n"
    " +-+ \n"
    " | | \n"
    " +-+ \n"
    "     \n",
    NULL, // chr 64, @
    "+---+\n" // chr 65, A
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 66, B
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 67, C
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+-+  \n" // chr 68, D
    "|  ` \n"
    "|   +\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 69, E
    "|    \n"
    "|    \n"
    "+--+ \n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 70, F
    "|    \n"
    "|    \n"
    "+--+ \n"
    "|    \n"
    "|    \n"
    "+    \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 71, G
    "|    \n"
    "|    \n"
    "| +-+\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+   +\n" // chr 72, H
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "+-+-+\n" // chr 73, I
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "+-+-+\n"
    "     \n"
    "     \n",
    "+-+-+\n" // chr 74, J
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "+ |  \n"
    "| |  \n"
    "+-+  \n"
    "     \n"
    "     \n",
    "+  + \n" // chr 75, K
    "| /  \n"
    "|/   \n"
    "+    \n"
    "|`   \n"
    "| `  \n"
    "+  + \n"
    "     \n"
    "     \n",
    "+    \n" // chr 76, L
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+-+-+\n" // chr 77, M
    "| | |\n"
    "| | |\n"
    "| | |\n"
    "| + |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "+-+  \n" // chr 78, N
    "|  ` \n"
    "|   +\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "  +  \n" // chr 79, O
    " / ` \n"
    "+   +\n"
    "|   |\n"
    "+   +\n"
    " ` / \n"
    "  +  \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 80, P
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|    \n"
    "|    \n"
    "+    \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 81, Q
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "| + |\n"
    "|  `|\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+---+\n" // chr 82, R
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|`   \n"
    "| `  \n"
    "+  + \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 83, S
    "|    \n"
    "|    \n"
    "+---+\n"
    "    |\n"
    "    |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+-+-+\n" // chr 84, T
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n",
    "+   +\n" // chr 85, U
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+   +\n" // chr 86, V
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    " ` / \n"
    "  +  \n"
    "     \n"
    "     \n",
    "+   +\n" // chr 87, W
    "|   |\n"
    "|   |\n"
    "| + |\n"
    "| | |\n"
    "| | |\n"
    "+-+-+\n"
    "     \n"
    "     \n",
    "+   +\n" // chr 88, X
    " ` / \n"
    "  +  \n"
    "  |  \n"
    "  +  \n"
    " / ` \n"
    "+   +\n"
    "     \n"
    "     \n",
    "+   +\n" // chr 89, Y
    "|   |\n"
    "|   |\n"
    "+-+-+\n"
    "  |  \n"
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n",
    "+---+\n" // chr 90, Z
    "   / \n"
    "  /  \n"
    " /   \n"
    "+    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+-+  \n" // chr 91, [
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+-+  \n"
    "     \n"
    "     \n",
    "     \n" // chr 92, backslash
    "+    \n"
    " `   \n"
    "  `  \n"
    "   ` \n"
    "    +\n"
    "     \n"
    "     \n"
    "     \n",
    "  +-+\n" // chr 93, ]
    "    |\n"
    "    |\n"
    "    |\n"
    "    |\n"
    "    |\n"
    "  +-+\n"
    "     \n"
    "     \n",
    "  +  \n" // chr 94, ^
    " / ` \n"
    "+   +\n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n",
    "     \n" // chr 95, _
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "     \n"
    "+---+\n"
    "     \n"
    "     \n",
    NULL,     // chr 96, `
    "     \n" // chr 97, a
    "     \n"
    "+---+\n"
    "    |\n"
    "+---+\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+    \n" // chr 98, b
    "|    \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "     \n" // chr 99, c
    "     \n"
    "+---+\n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "    +\n" // chr 100, d
    "    |\n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "     \n" // chr 101, e
    "     \n"
    "+---+\n"
    "|   |\n"
    "+---+\n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "+--+ \n" // chr 102, f
    "|    \n"
    "|    \n"
    "+-+  \n"
    "|    \n"
    "|    \n"
    "+    \n"
    "     \n"
    "     \n",
    "     \n" // chr 103, g
    "     \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "    |\n"
    "+---+\n",
    "+    \n" // chr 104, h
    "|    \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "     \n" // chr 105, i
    "     \n"
    "  +  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n",
    "     \n" // chr 106, j
    "     \n"
    "  +  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "+ |  \n"
    "| |  \n"
    "+-+  \n",
    " +   \n" // chr 107, k
    " |   \n"
    " | + \n"
    " |/  \n"
    " +   \n"
    " |`  \n"
    " + + \n"
    "     \n"
    "     \n",
    "  +  \n" // chr 108, l
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  |  \n"
    "  +  \n"
    "     \n"
    "     \n",
    "     \n" // chr 109, m
    "     \n"
    "+-+-+\n"
    "| | |\n"
    "| | |\n"
    "| | |\n"
    "+ + +\n"
    "     \n"
    "     \n",
    "     \n" // chr 110, n
    "     \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+   +\n"
    "     \n"
    "     \n",
    "     \n" // chr 111, o
    "     \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "     \n" // chr 112, p
    "     \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "|    \n"
    "+    \n",
    "     \n" // chr 113, q
    "     \n"
    "+---+\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "    |\n"
    "    +\n",
    "     \n" // chr 114, r
    "     \n"
    "+---+\n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+    \n"
    "     \n"
    "     \n",
    "     \n" // chr 115, s
    "     \n"
    "+---+\n"
    "|    \n"
    "+---+\n"
    "    |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "+    \n" // chr 116, t
    "|    \n"
    "+--+ \n"
    "|    \n"
    "|    \n"
    "|    \n"
    "+---+\n"
    "     \n"
    "     \n",
    "     \n" // chr 117, u
    "     \n"
    "+   +\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "     \n"
    "     \n",
    "     \n" // chr 118, v
    "     \n"
    "+   +\n"
    "|   |\n"
    "+   +\n"
    " ` / \n"
    "  +  \n"
    "     \n"
    "     \n",
    "     \n" // chr 119, w
    "     \n"
    "+   +\n"
    "| + |\n"
    "+ | +\n"
    "| | |\n"
    "+-+-+\n"
    "     \n"
    "     \n",
    "     \n" // chr 120, x
    "     \n"
    "+   +\n"
    " ` / \n"
    "     \n"
    " / ` \n"
    "+   +\n"
    "     \n"
    "     \n",
    "     \n" // chr 121, y
    "     \n"
    "+   +\n"
    "|   |\n"
    "|   |\n"
    "|   |\n"
    "+---+\n"
    "    |\n"
    "+---+\n",
    "     \n" // chr 122, z
    "     \n"
    "+---+\n"
    "   / \n"
    "  /  \n"
    " /   \n"
    "+---+\n"
    "     \n"
    "     \n",
    NULL,     // chr 123, {
    NULL,     // chr 124, |
    NULL,     // chr 125, }
    NULL,     // chr 126, ~
    NULL      // chr 127, weird DEL thing
};

#endif

