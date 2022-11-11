#ifndef UTILITIES_H
#define UTILITIES_H

#include <regex>

namespace lexemn::utilities
{

  namespace regex
  {
    std::regex digit("^(([0-9]*)|(([0-9]*)\\.([0-9]*)))$");
    std::regex arithmetic_operator("[\\^+\\-*\\/]");
    std::regex identifier("[a-zA-Z]");
  }

}

#endif /* UTILITIES_H */
