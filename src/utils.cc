#include "utils.hh"

using namespace std;

const string trimStr(const std::string &s) {
  string s1 = s;
  while (!s1.empty() && isspace(s1.back()))
    s1.pop_back();
  while (!s1.empty() && isspace(s1.front()))
    s1.erase(0, 1);
  return s1;
}
