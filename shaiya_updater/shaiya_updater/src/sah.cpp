#include "Form1.h"

using namespace Program;

void sah::open(std::vector<SFile>& patches, bool deleteList)
{
    std::ifstream ifs("data.sah", std::ifstream::binary);

    if (ifs.is_open())
    {
        SFolder folder{};
        folder.name = "";

        ifs.ignore(BYTE_LENGTH);

        if (deleteList)
        {
            sah::read(folder.name, &folder, &ifs, patches, deleteList);

            ifs.close();

            sah::save(&folder, {});
        }
        else
        {
            sah::read(folder.name, &folder, &ifs, patches, deleteList);

            ifs.close();

            int maximum = static_cast<int>(patches.size());
            Form1::setProgress(2, 0, maximum, 1);

            for (auto& patch : patches)
            {
                saf::write(patch);
                Form1::performStep(2);
            }

            sah::save(&folder, patches);
        }
    }
}

void sah::read(const std::string& path, SFolder* folder, std::ifstream* ifs, std::vector<SFile>& patches, bool deleteList)
{
    ifs->read((char*)&folder->fileCount, 4);

    // decrypt the file count
    folder->fileCount ^= FILE_COUNT_KEY;

    for (size_t i = 0; i < folder->fileCount; ++i)
    {
        SFile file{};

        util::read(ifs, file.name);

        ifs->read((char*)&file.offset, 8);
        ifs->read((char*)&file.length, 8);

        folder->files.push_back(file);
    }

    update::assign(path, folder, patches, deleteList);

    ifs->read((char*)&folder->folderCount, 4);

    for (size_t i = 0; i < folder->folderCount; ++i)
    {
        SFolder subFolder{};
        util::read(ifs, subFolder.name);

        std::filesystem::path fsp(path);
        fsp.append(subFolder.name);

        sah::read(fsp.string(), &subFolder, ifs, patches, deleteList);

        folder->folders.push_back(subFolder);
    }
}

void sah::save(SFolder* folder, const std::vector<SFile>& patches)
{
    std::ofstream ofs("data.sah", std::ofstream::binary);

    if (ofs.is_open())
    {
        uint32_t fileCount = 0;

        ofs.write(FILE_SIGNATURE, 3);
        util::write(&ofs, 48);
        util::write(&ofs, "");

        sah::write(folder, &ofs, patches, fileCount);

        util::write(&ofs, 8);

        // rewind
        ofs.clear();
        ofs.seekp(7, std::ios::beg);

        ofs.write((char*)&fileCount, 4);
        ofs.close();
    }
}

void sah::write(SFolder* folder, std::ofstream* ofs, const std::vector<SFile>& patches, uint32_t& fileCount)
{
    // encrypt the file count
    folder->fileCount ^= FILE_COUNT_KEY;

    ofs->write((char*)&folder->fileCount, 4);

    for (auto& file : folder->files)
    {
        for (const auto& patch : patches)
        {
            if (file.name == patch.name)
            {
                file.offset = patch.offset;
                file.length = patch.length;
            }
        }

        util::write(ofs, file.name);
        ofs->write((char*)&file.offset, 8);
        ofs->write((char*)&file.length, 8);

        ++fileCount;
    }

    ofs->write((char*)&folder->folderCount, 4);

    for (auto& subFolder : folder->folders)
    {
        util::write(ofs, subFolder.name);
        sah::write(&subFolder, ofs, patches, fileCount);
    }
}
