#pragma once
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <minizip/unzip.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <zip.h>
#include <zlib.h>
namespace fs = std::filesystem;

class YlFile {
public:
  std::vector<std::string> readTextFromFile(std::string path);
  std::vector<fs::path> listFilesAndDirectories(fs::path path);
};

class YlTool {
public:
  ImVec2 arr2vec2(float *arr);
  ImVec4 arr2vec4(float *arr);
};

class GitForDoc {
public:
  GitForDoc(fs::path path);

  void zipToDoc();

  fs::path _docPath, _gitPath, _tmpPath;
};
std::vector<std::string> runCmdAndReceOutput(const char *cmd, ...)
    __attribute__((format(printf, 1, 2)));
void printVecStr(const std::vector<std::string> vecstr);

class G4D {
public:
  std::vector<std::string> main_no(int argc, char *argv[]);
  std::vector<std::string> main_no(const std::vector<std::string> &args) {
    std::vector<char *> c_args;
    for (const auto &arg : args) {
      c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    return main_no(static_cast<int>(c_args.size()) - 1, c_args.data());
  }
};
