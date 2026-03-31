#include "imgui.h"
#include "ylheader.h"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <json/reader.h>
#include <json/value.h>
#include <string>
#include <vector>

void SetupBlueWhiteTheme(ImGuiStyle &style) {
  // 样式参数：圆角、边框、间距等
  style.WindowRounding = 6.0f;
  style.FrameRounding = 4.0f;
  style.GrabRounding = 4.0f;
  style.ScrollbarRounding = 6.0f;
  style.TabRounding = 4.0f;
  style.ChildRounding = 4.0f;
  style.PopupRounding = 4.0f;

  style.WindowBorderSize = 1.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.TabBorderSize = 0.0f;

  style.WindowPadding = ImVec2(8, 8);
  style.FramePadding = ImVec2(6, 4);
  style.ItemSpacing = ImVec2(8, 6);
  style.ItemInnerSpacing = ImVec2(6, 4);
  style.IndentSpacing = 20.0f;
  style.ScrollbarSize = 14.0f;
  style.GrabMinSize = 10.0f;

  // 颜色定义
  ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  ImVec4 lightGray = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
  ImVec4 midGray = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  ImVec4 darkGray = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  ImVec4 blue = ImVec4(0.20f, 0.60f, 1.00f, 1.00f);
  ImVec4 blueHovered = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);
  ImVec4 blueActive = ImVec4(0.10f, 0.50f, 0.90f, 1.00f);
  ImVec4 border = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
  ImVec4 shadow = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);

  // 背景 & 窗口
  style.Colors[ImGuiCol_WindowBg] = white;
  style.Colors[ImGuiCol_ChildBg] = white;
  style.Colors[ImGuiCol_PopupBg] = white;
  style.Colors[ImGuiCol_MenuBarBg] = lightGray;

  // 标题栏
  style.Colors[ImGuiCol_TitleBg] = blue;
  style.Colors[ImGuiCol_TitleBgActive] = blueHovered;
  style.Colors[ImGuiCol_TitleBgCollapsed] = midGray;

  // 边框
  style.Colors[ImGuiCol_Border] = border;
  style.Colors[ImGuiCol_BorderShadow] = shadow;

  // 文本
  style.Colors[ImGuiCol_Text] = darkGray;
  style.Colors[ImGuiCol_TextDisabled] = midGray;
  style.Colors[ImGuiCol_TextSelectedBg] = blue;

  // 按钮
  style.Colors[ImGuiCol_Button] = blue;
  style.Colors[ImGuiCol_ButtonHovered] = blueHovered;
  style.Colors[ImGuiCol_ButtonActive] = blueActive;

  // 头部（如折叠头、标签页头）
  style.Colors[ImGuiCol_Header] = lightGray;
  style.Colors[ImGuiCol_HeaderHovered] = blueHovered;
  style.Colors[ImGuiCol_HeaderActive] = blueActive;

  // 框架（如输入框、组合框）
  style.Colors[ImGuiCol_FrameBg] = lightGray;
  style.Colors[ImGuiCol_FrameBgHovered] = white;
  style.Colors[ImGuiCol_FrameBgActive] = white;

  // 滑块/滚动条
  style.Colors[ImGuiCol_SliderGrab] = blue;
  style.Colors[ImGuiCol_SliderGrabActive] = blueActive;
  style.Colors[ImGuiCol_ScrollbarBg] = lightGray;
  style.Colors[ImGuiCol_ScrollbarGrab] = midGray;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = blueHovered;
  style.Colors[ImGuiCol_ScrollbarGrabActive] = blueActive;

  // 勾选框、单选框
  style.Colors[ImGuiCol_CheckMark] = blue;

  // 分隔线
  style.Colors[ImGuiCol_Separator] = border;
  style.Colors[ImGuiCol_SeparatorHovered] = blueHovered;
  style.Colors[ImGuiCol_SeparatorActive] = blueActive;

  // 标签页
  style.Colors[ImGuiCol_Tab] = lightGray;
  style.Colors[ImGuiCol_TabHovered] = blueHovered;
  style.Colors[ImGuiCol_TabActive] = white;
  style.Colors[ImGuiCol_TabUnfocused] = lightGray;
  style.Colors[ImGuiCol_TabUnfocusedActive] = white;

  // 工具提示
  style.Colors[ImGuiCol_PopupBg] = white;
  style.Colors[ImGuiCol_PlotLines] = blue;
  style.Colors[ImGuiCol_PlotLinesHovered] = blueHovered;
  style.Colors[ImGuiCol_PlotHistogram] = blue;
  style.Colors[ImGuiCol_PlotHistogramHovered] = blueHovered;

  // 其他
  style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
  style.Colors[ImGuiCol_DragDropTarget] = blue;
  style.Colors[ImGuiCol_NavHighlight] = blue;
  style.Colors[ImGuiCol_NavWindowingHighlight] = blue;
}

char console_text[10240] = "";

void updateConsoleText(const std::vector<std::string> &logLines) {
  console_text[0] = '\0';
  int offset = 0;
  int remaining = sizeof(console_text);

  for (const auto &line : logLines) {
    int written =
        snprintf(console_text + offset, remaining, "%s\n", line.c_str());
    if (written < 0 || written >= remaining) {
      break;
    }
    offset += written;
    remaining -= written;
  }
}

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

  GLFWwindow *window = glfwCreateWindow(1280, 720, "G4D", NULL, NULL);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImFont *font1 = io.Fonts->AddFontFromFileTTF(
      "./fontf.ttf", 20.0f, NULL,
      ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
  ImFont *font2 = io.Fonts->AddFontFromFileTTF(
      "./fontf.ttf", 78.0f, NULL,
      ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
  ImFont *font3 = io.Fonts->AddFontFromFileTTF(
      "./fontf.ttf", 12.0f, NULL,
      ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
  ImGuiStyle &style = ImGui::GetStyle();

  style.FrameRounding = 10.0f;
  style.WindowRounding = 8.0f;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  char testWord[256] = "test word";
  float roundingRatio = 8.0f;

  char doc_path[1024] = "";
  char commit_message_text[2048] = "Default Commit Message";
  char revert_message_text[2048] = "Default Revert Message";
  char revert_git_id[512] = "";

  bool configure_window_enabled = false;

  YlTool ylTool;
  YlFile ylFile;

  G4D g4d;

  std::string lang_file = "./data/zh_cn.json";
  std::ifstream i18nifs(lang_file);
  Json::Reader i18nreader;
  Json::Value i18nobj;
  i18nreader.parse(i18nifs, i18nobj);
  ImGui::CreateContext();
  ImGuiIO &io_cont = ImGui::GetIO();

  ImGuiStyle &style_cont = ImGui::GetStyle();
  SetupBlueWhiteTheme(style_cont);
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin(i18nobj["Title"].asCString(), NULL);

    ImGui::PushFont(font1);

    ImGui::InputText(i18nobj["doc_path_desc"].asCString(), doc_path,
                     sizeof(doc_path));

    if (ImGui::Button(i18nobj["select_file_btn"].asCString())) {
      if (fs::is_regular_file(doc_path)) {
        std::vector<std::string> doc_now = ylFile.readTextFromFile(doc_path);
        auto res = g4d.main_no({"g4d", "file", doc_path});
        updateConsoleText(res);
      } else {
        doc_path[0] = '\0';
      }
    }

    if (ImGui::Button(i18nobj["init_btn"].asCString())) {
      auto res = g4d.main_no({"g4d", "init", doc_path});
      updateConsoleText(res);
    }

    if (ImGui::Button(i18nobj["add_btn"].asCString())) {
      auto res = g4d.main_no({"g4d", "add"});
      updateConsoleText(res);
    }

    ImGui::InputText(i18nobj["commit_msg_inputtext"].asCString(),
                     commit_message_text, sizeof(commit_message_text));

    if (ImGui::Button(i18nobj["commit_btn"].asCString())) {
      if (!strcmp(commit_message_text, "")) {
        strcpy(commit_message_text, "Default Commit Message");
      }
      auto res = g4d.main_no({"g4d", "commit", "-m",
                              "\"" + std::string(commit_message_text) + "\""});
      updateConsoleText(res);
    }

    if (ImGui::Button(i18nobj["log_btn"].asCString())) {
      auto res = g4d.main_no({"g4d", "log"});
      updateConsoleText(res);
    }
    ImGui::SameLine();

    if (ImGui::Button(i18nobj["status_btn"].asCString())) {
      auto res = g4d.main_no({"g4d", "status"});
      updateConsoleText(res);
    }
    ImGui::SameLine();

    if (ImGui::Button(i18nobj["diff_btn"].asCString())) {
      auto ret = g4d.main_no({"g4d", "diff"});
      std::vector<std::string> res;

      for (int i = 0; i < ret.size(); i++) {
        if (ret[i][0] == '@' && ret[i][1] == '@' &&
            strstr(ret[i - 1].c_str(), "document.xml") != nullptr) {
          for (int j = i + 1; j < ret.size(); j++) {
            res.push_back(ret[j]);
          }
          break;
        }
      }

      updateConsoleText(res);
    }

    ImGui::InputText(i18nobj["revert_git_id"].asCString(), revert_git_id,
                     sizeof(revert_git_id));
    ImGui::InputText(i18nobj["revert_message_text"].asCString(),
                     revert_message_text, sizeof(revert_message_text));

    if (ImGui::Button(i18nobj["revert_btn"].asCString())) {
      auto res =
          g4d.main_no({"g4d", "revert", revert_git_id,
                       "-m \"" + std::string(revert_message_text) + "\""});

      updateConsoleText(res);
    }

    ImGui::Spacing();

    ImGui::PushFont(font3);
    ImGui::InputTextMultiline(
        "##hidden", console_text, IM_ARRAYSIZE(console_text),
        ImGui::CalcTextSize(console_text), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text(i18nobj["config_window_here"].asCString());

    ImGui::Spacing();

    if (ImGui::Button(i18nobj["config_btn"].asCString())) {
      configure_window_enabled = !configure_window_enabled;
    }

    ImGui::PopFont();
    ImGui::End();

    if (configure_window_enabled) {
      ImGui::Begin(i18nobj["config_window_here"].asCString());

      ImGui::ColorEdit4("Color", color, ImGuiColorEditFlags_AlphaBar);

      ImGui::Separator();

      ImGui::PushStyleColor(ImGuiCol_Text, ylTool.arr2vec4(color));

      ImGui::InputText(i18nobj["cfgwindow_test_text"].asCString(), testWord,
                       sizeof(testWord));

      ImGui::Separator();
      ImGui::PushFont(font2);
      ImGui::Text(testWord);
      ImGui::PopFont();
      ImGui::PopStyleColor();

      ImGui::Separator();
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::SliderFloat(i18nobj["roundingRatio"].asCString(), &roundingRatio,
                         0.0f, 15.0f);
      style.WindowRounding = roundingRatio;
      style.FrameRounding = roundingRatio;

      ImGui::End();
    }

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
