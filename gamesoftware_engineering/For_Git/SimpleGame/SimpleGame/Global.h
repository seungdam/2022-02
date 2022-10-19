#pragma once

#define MAX_OBJECT_COUNT 1000

struct GSEVec2 {
	float x = 0.0f; float y = 0.0f;
};

struct GSEVec3{
	float x = 0.0f; float y = 0.0f;  float z = 0.0f;
};

struct GSEKeyBoardMapper {
	bool W_key = false;
	bool A_key = false;
	bool S_key = false;
	bool D_key = false;
};