#include "Form1.h"

void saf::write(SFile& patch)
{
    std::fstream fs("data.saf", std::fstream::binary | std::fstream::in | std::fstream::out);

    if (fs.is_open())
    {
        if (patch.append)
        {
            fs.seekp(0, fs.end);
            patch.offset = fs.tellp();

            // rewind
            fs.clear();
            fs.seekp(0, fs.beg);
        }

        fs.seekp(patch.offset);
        fs.write(patch.buffer.data(), patch.buffer.size());
        fs.close();
    }
}

void saf::erase(uint64_t offset, uint64_t length)
{
    std::fstream fs("data.saf", std::fstream::binary | std::fstream::in | std::fstream::out);

    if (fs.is_open())
    {
        size_t size = static_cast<size_t>(length);
        std::vector<char> buffer(size, 0);

        fs.seekp(offset);
        fs.write(buffer.data(), buffer.size());
        fs.close();
    }
}
