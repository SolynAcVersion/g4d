#include "zip.h"
#include <complex>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <include/ylheader.h>
#include <iostream>
#include <minizip/unzip.h>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> YlFile::readTextFromFile(std::string path) {
  std::vector<std::string> res;
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << "Can't read files: " << path << std::endl;
    return res;
  }
  std::string line;
  while (std::getline(ifs, line)) {
    res.push_back(line);
  }
  ifs.close();
  return res;
}

std::vector<fs::path> listFilesAndDirectories(fs::path path) {
  if (!fs::exists(path) || !fs::is_directory(path)) {
    std::cerr << "Path not valid: " << path << std::endl;
    return {};
  }
  std::vector<fs::path> res;
  for (const auto &p : fs::directory_iterator(path)) {
    res.push_back(p.path());
  }
  return res;
}

void zipDirContents(const char *zipF, const char *sourceD) {
  zipFile zf = zipOpen(zipF, APPEND_STATUS_CREATE);
  if (zf == NULL) {
    throw std::runtime_error("Unable to create zip file");
  }

  if (!fs::exists(sourceD) || !fs::is_directory(sourceD)) {
    zipClose(zf, NULL);
    throw std::runtime_error("Source directory does not exist");
  }

  for (const auto &entry : fs::recursive_directory_iterator(sourceD)) {
    if (!fs::is_regular_file(entry.path()))
      continue;

    fs::path relative = fs::relative(entry.path(), sourceD);
    std::string zipPath = relative.string();
    std::replace(zipPath.begin(), zipPath.end(), '\\', '/');

    FILE *inFile = fopen(entry.path().string().c_str(), "rb");
    if (inFile == NULL) {
      zipClose(zf, NULL);
      throw std::runtime_error("Cannot open file: " + entry.path().string());
    }

    if (zipOpenNewFileInZip(zf, zipPath.c_str(), NULL, NULL, 0, NULL, 0, NULL,
                            Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
      fclose(inFile);
      zipClose(zf, NULL);
      throw std::runtime_error("Cannot create file in zip: " + zipPath);
    }

    char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inFile)) > 0) {
      if (zipWriteInFileInZip(zf, buffer, (unsigned int)bytesRead) != ZIP_OK) {
        fclose(inFile);
        zipCloseFileInZip(zf);
        zipClose(zf, NULL);
        throw std::runtime_error("Write failed: " + zipPath);
      }
    }

    fclose(inFile);
    zipCloseFileInZip(zf);
  }

  zipClose(zf, NULL);
}

void unzipFile(const char *zipFile, const char *outputDir) {
  unzFile uf = unzOpen(zipFile);
  if (uf == NULL) {
    throw std::runtime_error("Unable to open zip file");
  }
  unz_global_info globalInfo;
  if (unzGetGlobalInfo(uf, &globalInfo) != UNZ_OK) {
    unzClose(uf);
    throw std::runtime_error("Unable to access globalInfo");
  }
  if (unzGoToFirstFile(uf) != UNZ_OK) {
    unzClose(uf);
    throw std::runtime_error("Unable to access to the first file");
  }
  do {
    char filename[512];
    unz_file_info fileInfo;
    if (unzGetCurrentFileInfo(uf, &fileInfo, filename, sizeof(filename), NULL,
                              0, NULL, 0) != UNZ_OK) {
      unzClose(uf);
      throw std::runtime_error("Unable to get current file info");
    }
    fs::path filePath = fs::path(outputDir) / filename;
    if (unzOpenCurrentFile(uf) != UNZ_OK) {
      unzClose(uf);
      throw std::runtime_error("Unable to open current file");
    }
    fs::create_directories(filePath.parent_path());
    FILE *outputFile = fopen(filePath.string().c_str(), "wb");
    if (outputFile == NULL) {
      unzCloseCurrentFile(uf);
      unzClose(uf);
      throw std::runtime_error("Unable to open output file");
    }
    char buffer[4096];
    int bytesRead;
    do {
      bytesRead = unzReadCurrentFile(uf, buffer, sizeof(buffer));
      if (bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, outputFile);
      }
    } while (bytesRead > 0);
    fclose(outputFile);
    unzCloseCurrentFile(uf);
  } while (unzGoToNextFile(uf) == UNZ_OK);
  unzClose(uf);
}

void format_xml(fs::path path) {
  for (const auto &p : fs::recursive_directory_iterator(path)) {
    if (!p.is_regular_file())
      continue;
    if (p.path().extension() != ".xml")
      continue;
    std::string cmd = "xmllint --format \"" + p.path().string() + "\" -o \"" +
                      p.path().string() + "\"";
    runCmdAndReceOutput("%s", cmd.c_str());
  }
}

GitForDoc::GitForDoc(fs::path path) {
  if (!fs::is_regular_file(path)) {
    throw std::runtime_error("Path not existing!");
  }
  if (path.extension().string() == ".doc" ||
      path.extension().string() == ".docx") {
    _docPath = path;
    _gitPath = path.parent_path() / path.stem() / ".git";
    _tmpPath = path.parent_path() / path.stem() / ".tmp";
    if (fs::is_directory(_tmpPath)) {
      fs::remove_all(_tmpPath);
    }
    fs::create_directories(_tmpPath);
    try {
      unzipFile(_docPath.c_str(), _tmpPath.c_str());
    } catch (...) {
      fs::remove_all(_tmpPath);
      throw;
    }
    format_xml(_tmpPath);
  }
}

void GitForDoc::zipToDoc() {
  zipDirContents(_docPath.c_str(), _tmpPath.c_str());
}

std::vector<std::string> runCmdAndReceOutput(const char *cmd, ...) {
  FILE *fp;
  char buffer[10240];
  char buffer_2[4096];
  va_list args;
  va_start(args, cmd);
  vsnprintf(buffer, sizeof(buffer), cmd, args);
  va_end(args);
  printf("%s\n", buffer);
  std::vector<std::string> res;
  fp = popen(buffer, "r");
  if (!fp) {
    return res;
  }
  std::string line;
  while (fgets(buffer_2, sizeof(buffer_2), fp)) {
    line = buffer_2;
    res.push_back(line);
  }
  pclose(fp);
  return res;
}

void printVecStr(const std::vector<std::string> vecstr) {
  for (auto &i : vecstr) {
    printf("%s\n", i.c_str());
  }
}
