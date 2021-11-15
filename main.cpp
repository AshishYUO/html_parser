#include <chrono>
#include "src/include/html_parser.hpp"

int main (int argc, char **argv) {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double> time;
  start = std::chrono::system_clock::now();
  html_parser d;
  dom_element *document = d.parse_html(argv[1]);
  int loop = 0;
  for (auto i = 0; i < loop; ++i) {
    document = d.parse_html(argv[1]);
  }
  end = std::chrono::system_clock::now();
  time = (end - start);
  std::cout << "Time parsing " << loop+1 << " times: " << time.count() << "s" << std::endl;

  std::cout << document->innerHTML() << std::endl;
}
