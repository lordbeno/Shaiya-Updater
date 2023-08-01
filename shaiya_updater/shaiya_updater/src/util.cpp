#include "Form1.h"

void util::read(std::ifstream* ifs, std::string& str)
{
    size_t count{};
    ifs->read((char*)&count, 4);

    auto buffer = std::make_unique<char[]>(count);
    ifs->read(buffer.get(), count);

    str = buffer.get();
}

void util::write(std::ofstream* ofs, const std::string& str)
{
    size_t count = str.length() + 1;
    ofs->write((char*)&count, 4);
    ofs->write(str.c_str(), count);
}

void util::write(std::ofstream* ofs, size_t count)
{
    std::vector<char> buffer(count, 0);
    ofs->write(buffer.data(), buffer.size());
}
