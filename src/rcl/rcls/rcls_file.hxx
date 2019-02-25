#pragma once
#include <string>
#include <vector>


namespace rqdq {
namespace rcls {

std::vector<std::string> FindGlob(const std::string& pathpat);

long long GetFileMTime(const std::string& fn);

std::vector<char> GetFileContents(const std::string& fn);
void GetFileContents(const std::string& fn, std::vector<char>& buf);

std::vector<std::string> GetFileLines(const std::string& filename);

}  // close package namespace
}  // close enterprise namespace
