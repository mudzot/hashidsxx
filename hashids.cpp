#include "hashids.h"

#include <algorithm>

#define RATIO_SEPARATORS 3.5
#define RATIO_GUARDS 12

#include <iostream>

namespace hashidsxx {

const static std::string separators("cfhistuCFHISTU");

Hashids::Hashids(const std::string &salt, const std::string alphabet, unsigned int min_length)
: _salt(salt)
, _alphabet(alphabet)
, _min_length(min_length)
{
  std::for_each(separators.begin(), separators.end(), [this](char c) {
    if (_alphabet.find(c) != std::string::npos) _separators.push_back(c);
  });
  _alphabet.erase(std::remove_if(_alphabet.begin(), _alphabet.end(), [this](char c) { return _separators.find(c) != std::string::npos; }), _alphabet.end());
  if (_alphabet.size() + _separators.size() < 16) throw std::runtime_error("Alphabet must contain at least 16 unique characters");

  _separators = _reorder(_separators, _salt);

  int min_separators = (int) std::ceil((float) _alphabet.length() / RATIO_SEPARATORS);

  if (_separators.empty() || _separators.length() < min_separators) {
    if (min_separators == 1) min_separators = 2;
    if (min_separators > _separators.length()) {
      int split_at = min_separators - _separators.length();
      _separators.append(_alphabet.substr(0, split_at));
      _alphabet = _alphabet.substr(split_at);
    };
  };

  _alphabet = _reorder(_alphabet, _salt);
  int num_guards = (int) std::ceil((float) _alphabet.length() / RATIO_GUARDS);

  if (_alphabet.length() < 3) {
    _guards = _separators.substr(0, num_guards);
    _separators = _separators.substr(num_guards);
  } else {
    _guards = _alphabet.substr(0, num_guards);
    _alphabet = _alphabet.substr(num_guards);
  };
}

std::string& Hashids::_reorder(std::string &input, const std::string &salt) const
{
  if (salt.empty()) return input;

  int i = input.length() - 1;
  int index = 0;
  int integer_sum = 0;

  while (i > 0) {
    index %= salt.length();
    int integer = salt[index];
    integer_sum += integer;
    unsigned int j = (integer + index + integer_sum) % i;

    char tmp = input[j];
    std::string trailer((j + 1 < input.length()) ? input.substr(j+1) : "");
    input = input.substr(0, j) + input[i] + trailer;
    input = input.substr(0, i) + tmp + input.substr(i+1);

    --i;
    ++index;
  };

  return input;
}

std::string Hashids::hash(uint32_t number, const std::string &alphabet) const
{
  std::string output;
  while (true) {
    output.insert(output.begin(), alphabet[number % alphabet.size()]);
    number /= alphabet.size();
    if (number == 0) return output;
  };
}

std::string Hashids::encrypt(const std::vector<uint32_t>& input) const
{
  // Encrypting nothing makes no sense
  if (input.empty()) return "";

  // Make a copy of our alphabet so we can reorder it on the fly etc
  std::string alphabet(_alphabet);

  int values_hash = 0;
  for (int i = 0; i < input.size(); ++i) values_hash += (input[i] % (i + 100));

  char encoded = _alphabet[values_hash % _alphabet.size()];
  char lottery = encoded;

  std::string output;
  output.push_back(encoded);

  for (int i = 0; i < input.size(); ++i) {
    uint32_t number = input[i];

    std::string alphabet_salt;
    alphabet_salt.push_back(lottery);
    alphabet_salt.append(_salt).append(alphabet);

    alphabet = _reorder(alphabet, alphabet_salt);

    std::string last = hash(number, alphabet);
    output.append(last);

    number %= last[0] + i;
    output.push_back(_separators[number % _separators.size()]);
  };

  output.pop_back();

  return output;
}

std::vector<uint32_t> Hashids::decrypt(const std::string& input) const
{
  std::vector<uint32_t> output;
  return output;
}

};