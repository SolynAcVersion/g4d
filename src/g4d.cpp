#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <include/ylheader.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

void checkDocPath(const std::string &path) {
  fs::path p(path);
  if (!fs::exists(p) || !fs::is_regular_file(p)) {
    throw std::runtime_error("Invalid document path: " + path);
  }
  if (access(p.c_str(), R_OK) != 0) {
    throw std::runtime_error("Cannot read document: " + path);
  }
}

std::string getConfigPath() {
  char buf[4096];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (len != -1) {
    buf[len] = '\0';
    fs::path p(buf);
    return (p.parent_path() / ".g4d.conf").string();
  }
  return "";
}

std::string readDefaultDoc() {
  std::string cfg = getConfigPath();
  if (cfg.empty() || !fs::exists(cfg))
    return "";
  std::ifstream f(cfg);
  if (!f.is_open())
    return "";
  std::string path;
  std::getline(f, path);
  return path;
}

void writeDefaultDoc(const std::string &path) {
  std::string cfg = getConfigPath();
  if (cfg.empty())
    throw std::runtime_error("Cannot find config path");
  std::ofstream f(cfg);
  if (!f.is_open())
    throw std::runtime_error("Cannot write config: " + cfg);
  f << path << std::endl;
}

void cmd_help() {
  printf("Usage:\n"
         "  g4d <command> [options] <document>\n\n"
         "Commands:\n"
         "  init <document>                       Initialize a Git repository "
         "for the document\n"
         "  file <document>                       Set default document (stored "
         "in .g4d.conf)\n"
         "  diff [options] <document>             Show changes\n"
         "  add [files...] <document>             Stage changes\n"
         "  commit [-m message] <document>        Commit changes\n"
         "  status <document>                     Show working tree status\n"
         "  log [options] <document>              Show commit history\n"
         "  reset [options] <commit> <document>   Reset to one commit\n"
         "  revert [options] <commit> <document>  Revert a commit\n\n"
         "Note: If your file path contains spaces or special characters, or "
         "you need to use quote for messages, use \\ before them.\n\n"
         "Examples:\n"
         "  g4d commit -m \\\"commit\\ with \\space\\\"\n\n"
         "Usage Examples:\n"
         "  g4d init ./test/doc1.docx\n"
         "  g4d file ./test/doc1.docx\n"
         "  g4d diff ./test/doc1.docx\n"
         "  g4d add document.xml ./test/doc1.docx\n"
         "  g4d commit -m \\\"Initial\\ commit\\\" ./test/doc1.docx\n"
         "  g4d status ./test/doc1.docx\n"
         "  g4d log --oneline ./test/doc1.docx\n"
         "  g4d reset --hard 77068a546b9a0d30b675ad6854b104dd5376e91d "
         " (after using g4d file for pointing out the specific file)\n\n"
         "BTW, we currently only support .doc or .docx files.\n");
}

std::vector<std::string> cmd_init(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[2]);
  auto ret =
      runCmdAndReceOutput("git init --separate-git-dir=%s %s",
                          docFile._gitPath.c_str(), docFile._tmpPath.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_diff(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git diff ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret =
      runCmdAndReceOutput("cd %s && %s-- .tmp",
                          docFile._gitPath.parent_path().c_str(), cmd.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_add(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git add ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s .tmp", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_commit(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git commit ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s .tmp", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_status(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git status ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_log(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git log ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_reset(int argc, char *argv[]) {

  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git reset ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  docFile.zipToDoc();
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> cmd_revert(int argc, char *argv[]) {
  checkDocPath(argv[argc - 1]);
  GitForDoc docFile = GitForDoc(argv[argc - 1]);
  std::string cmd = "git revert ";
  for (int i = 2; i < argc - 1; i++)
    cmd = cmd + argv[i] + " ";
  auto ret = runCmdAndReceOutput(
      "cd %s && %s", docFile._gitPath.parent_path().c_str(), cmd.c_str());
  docFile.zipToDoc();
  fs::remove_all(docFile._tmpPath);
  return ret;
}

std::vector<std::string> G4D::main_no(int argc, char *argv[]) {
  try {
    if (argc == 1) {
      cmd_help();
      return {};
    }
    if (!std::strcmp(argv[1], "help")) {
      cmd_help();
      return {};
    }
    if (!std::strcmp(argv[1], "file")) {
      if (argc != 3)
        throw std::runtime_error("Usage: g4d file <file path>");
      std::string path = argv[2];
      checkDocPath(path);
      writeDefaultDoc(path);
      std::cout << "Default document set to: " << path << std::endl;
      return {};
    }
    if (!std::strcmp(argv[1], "init")) {
      if (argc != 3)
        throw std::runtime_error("Usage: g4d init <document>");
      cmd_init(argc, argv);
      return {};
    }

    int no_path = 0;
    std::string docPath;
    docPath = argv[argc - 1];
    if (!fs::exists(docPath)) {
      docPath = readDefaultDoc();
      no_path = 1;
      if (docPath.empty())
        throw std::runtime_error("No document specified and no default set.");
      std::cout << "Path " + std::string(argv[argc - 1]) +
                       " not found. Using default path: " + docPath
                << std::endl;
    }

    checkDocPath(docPath);
    std::vector<char *> new_argv;
    for (int i = 0; i < argc - 1; ++i)
      new_argv.push_back(argv[i]);
    if (no_path) {
      new_argv.push_back(argv[argc - 1]);
    }
    new_argv.push_back(const_cast<char *>(docPath.c_str()));
    new_argv.push_back(nullptr);

    int new_argc = new_argv.size() - 1;
    if (!std::strcmp(argv[1], "diff"))
      return cmd_diff(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "add"))
      return cmd_add(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "commit"))
      return cmd_commit(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "status"))
      return cmd_status(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "log"))
      return cmd_log(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "reset"))
      return cmd_reset(new_argc, new_argv.data());
    else if (!std::strcmp(argv[1], "revert"))
      return cmd_revert(new_argc, new_argv.data());
    else
      cmd_help();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return {};
  }
  return {};
}
