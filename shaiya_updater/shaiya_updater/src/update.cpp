#include "Form1.h"

using namespace System;
using namespace System::IO;
using namespace System::IO::Compression;
using namespace System::Net;
using namespace System::Net::Cache;

using namespace Program;

HttpStatusCode update::getStatusCode(String^ url)
{
    auto statusCode = HttpStatusCode::NotFound;

    try
    {
        auto cachePolicy = gcnew RequestCachePolicy(RequestCacheLevel::NoCacheNoStore);
        auto webRequest = (HttpWebRequest^)WebRequest::Create(url);

        webRequest->CachePolicy = cachePolicy;

        auto webResponse = (HttpWebResponse^)webRequest->GetResponse();

        statusCode = webResponse->StatusCode;
        webResponse->Close();
    }
    catch (Exception^ ex)
    {
        Program::Form1::setTextBox1(ex->Message);
    }

    return statusCode;
}

int update::getArchive(int version, int step, int total)
{
    auto archiveFileName = String::Format("ps{0:D4}{1}", version, ".zip");
    auto url = gcnew String(DOWNLOAD_URL.c_str()) + archiveFileName + "?timestamp=" + DateTime::Now.Ticks.ToString();

    if (update::getStatusCode(url) != HttpStatusCode::OK)
    {
        return -1;
    }

    try
    {
        auto cachePolicy = gcnew RequestCachePolicy(RequestCacheLevel::NoCacheNoStore);
        auto webClient = gcnew WebClient();

        webClient->CachePolicy = cachePolicy;

        auto text = String::Format("Downloading archive ({0}/{1})", step, total);
        Form1::setTextBox1(text);

        webClient->DownloadFile(url, archiveFileName);
        delete webClient;
    }
    catch (Exception^ ex)
    {
        Form1::setTextBox1(ex->Message);
        return -1;
    }

    try
    {
        auto source = ZipFile::OpenRead(archiveFileName);

        Program::Form1::setTextBox1("Extracting archive");

        // Loop through all entries in the zip archive
        for each (auto entry in source->Entries)
        {
            if (entry->FullName->EndsWith("/")) // Check if entry is a directory
            {
                String^ dirName = Path::Combine(Path::GetDirectoryName(entry->FullName), Path::GetFileNameWithoutExtension(entry->FullName));
                Directory::CreateDirectory(dirName); // Create the directory in the destination folder
            }
            else // Entry is a file, extract it
            {
                ZipFileExtensions::ExtractToFile(entry, entry->FullName, true);
            }
        }

        delete source;
        File::Delete(archiveFileName);
    }
    catch (Exception^ ex)
    {
        Form1::setTextBox1(ex->Message);
        return -1;
    }

    auto text = String::Format("Updating client to v{0}", version);
    Form1::setTextBox1(text);

    if (std::filesystem::exists(DELETE_LIST_FILENAME))
    {
        std::ifstream ifs(DELETE_LIST_FILENAME);

        if (!ifs.is_open())
        {
            return -1;
        }

        std::vector<SFile> patches;

        std::string path;
        while (std::getline(ifs, path))
        {
            SFile patch{};
            patch.path = path;
            patches.push_back(patch);
        }

        ifs.close();

        sah::open(patches, true);
        std::remove(DELETE_LIST_FILENAME.c_str());

        if (!update::open())
        {
            std::remove("update.sah");
            std::remove("update.saf");
            Form1::setTextBox1("Update completed");
            return 0;
        }

        return -1;
    }

    if (!update::open())
    {
        std::remove("update.sah");
        std::remove("update.saf");
        Form1::setTextBox1("Update completed");
        return 0;
    }

    Form1::setTextBox1("Update completed");
    return 0;
}

void update::init()
{
    auto url = gcnew String(INI_FILE_URL.c_str()) + "UpdateVersion.ini";

    if (update::getStatusCode(url) != HttpStatusCode::OK)
    {
        return;
    }

    try
    {
        auto cachePolicy = gcnew RequestCachePolicy(RequestCacheLevel::NoCacheNoStore);
        auto webClient = gcnew WebClient();

        webClient->CachePolicy = cachePolicy;
        webClient->DownloadFile(url, "UpdateVersion.ini");
        delete webClient;
    }
    catch (Exception^ ex)
    {
        Form1::setTextBox1(ex->Message);
        return;
    }

    int patchFileVersion = GetPrivateProfileIntA("Version", "PatchFileVersion", 0, ".\\UpdateVersion.ini");
    File::Delete("UpdateVersion.ini");

    if (!patchFileVersion)
    {
        return;
    }

    int currentVersion = GetPrivateProfileIntA("Version", "CurrentVersion", 0, ".\\Version.ini");
    if (!currentVersion)
    {
        return;
    }

    if (patchFileVersion > currentVersion)
    {
        int total = patchFileVersion - currentVersion;
        int step = 1;

        Form1::setProgress(1, 0, total, step);
        Form1::setProgress(2, 0, total, step);

        while (currentVersion < patchFileVersion)
        {
            Form1::performStep(1);
            Form1::performStep(2);

            if (!update::getArchive(currentVersion + 1, step, total))
            {
                ++currentVersion;
                ++step;

                WritePrivateProfileStringA("Version", "CurrentVersion",
                    std::to_string(currentVersion).c_str(), ".\\Version.ini");

                continue;
            }

            break;
        }
    }
    else
    {
        // Set progress bar 1 to max and progress bar 2 to min when client is up-to-date
        Form1::setProgress(1, Form1::getProgressBarMaximum(1), Form1::getProgressBarMaximum(1), 1);
        Form1::setProgress(2, Form1::getProgressBarMaximum(2), Form1::getProgressBarMaximum(2), 2);
        Form1::setTextBox1("Client is up-to-date");
    }

}

int update::open()
{
    std::ifstream ifs("update.sah", std::ifstream::binary);

    if (!ifs.is_open())
    {
        return -1;
    }

    try
    {
        std::vector<SFile> patches{};

        SFolder folder{};
        folder.name = "";

        ifs.ignore(sah::BYTE_LENGTH);

        update::read(folder.name, &folder, &ifs, patches);

        ifs.close();

        update::seek(patches);

        sah::open(patches, false);
        return 0;
    }
    catch (const std::exception& ex)
    {
        Form1::setTextBox1(gcnew String(ex.what()));
        return -1;
    }
}

void update::read(const std::string& path, SFolder* folder, std::ifstream* ifs, std::vector<SFile>& patches)
{
    ifs->read((char*)&folder->fileCount, 4);

    // decrypt the file count
    folder->fileCount ^= sah::FILE_COUNT_KEY;

    for (size_t i = 0; i < folder->fileCount; ++i)
    {
        SFile patch{};
        patch.path = path;
        util::read(ifs, patch.name);

        ifs->read((char*)&patch.offset, 8);
        ifs->read((char*)&patch.length, 8);

        size_t size = static_cast<size_t>(patch.length);
        patch.buffer.resize(size);

        patches.push_back(patch);
    }

    ifs->read((char*)&folder->folderCount, 4);

    for (size_t i = 0; i < folder->folderCount; ++i)
    {
        SFolder subFolder{};
        util::read(ifs, subFolder.name);

        std::filesystem::path fsp(path);
        fsp.append(subFolder.name);

        update::read(fsp.string(), &subFolder, ifs, patches);
    }
}

void update::seek(std::vector<SFile>& patches)
{
    for (auto& patch : patches)
    {
        std::ifstream ifs("update.saf", std::ifstream::binary);

        if (ifs.is_open())
        {
            ifs.seekg(patch.offset);
            ifs.read(patch.buffer.data(), patch.buffer.size());
            ifs.close();
        }
    }
}

void update::assign(const std::string& path, SFolder* folder, std::vector<SFile>& patches, bool deleteList)
{
    if (deleteList)
    {
        for (size_t i = 0; i < folder->files.size(); ++i)
        {
            for (auto& patch : patches)
            {
                if (patch.assigned)
                    continue;

                std::filesystem::path fsp(path);
                fsp.append(folder->files[i].name);

                if (fsp.string() == patch.path)
                {
                    saf::erase(folder->files[i].offset, folder->files[i].length);

                    folder->files.erase(folder->files.begin() + i);
                    --folder->fileCount;

                    patch.assigned = true;
                }
            }
        }

        return;
    }

    // this loop is for old data
    for (const auto& file : folder->files)
    {
        for (auto& patch : patches)
        {
            if (patch.assigned)
                continue;

            if (file.name == patch.name)
            {
                saf::erase(file.offset, file.length);

                if (patch.length <= file.length)
                {
                    patch.offset = file.offset;
                    patch.append = false;
                }

                patch.assigned = true;
            }
        }
    }

    // this loop is for new data
    for (auto& patch : patches)
    {
        if (patch.assigned)
            continue;

        if (patch.path == path)
        {
            SFile file{};

            file.name = patch.name;
            file.length = patch.length;

            folder->files.push_back(file);
            ++folder->fileCount;

            patch.assigned = true;
        }
    }
}