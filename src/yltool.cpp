#include "../include/ylheader.h"

ImVec2 YlTool::arr2vec2(float *arr) {
  return ImVec2(arr[0], arr[1]);
}

ImVec4 YlTool::arr2vec4(float *arr) {
  return ImVec4(arr[0], arr[1], arr[2], arr[3]);
}
