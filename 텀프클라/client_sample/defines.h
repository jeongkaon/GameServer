#pragma once

constexpr int SCREEN_WIDTH = 20;
constexpr int SCREEN_HEIGHT = 20;
constexpr int TILE_WIDTH = 64 / 2;
constexpr int WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr int WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;
constexpr int SPRITE_WIDTH = 64 / 2;
constexpr int SPRITE_HEIGHT = 384 / 2 / 4;

constexpr int SPRITE_MON_HEIGHT = 256 / 4 / 2;

//채팅창 관련
const int CHATBOX_WIDTH = 400;
const int CHATBOX_HEIGHT = 200;
const int CHATBOX_MARGIN = 10;


const float speed = 64.0f;