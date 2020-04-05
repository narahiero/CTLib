//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#include <filesystem>
#include <iostream>
#include <vector>

#include <CTLib/U8.hpp>
#include <CTLib/Yaz.hpp>

int cmdHelp(std::vector<std::string>& args);
int cmdCreate(std::vector<std::string>& args);
int cmdExtract(std::vector<std::string>& args);
int cmdList(std::vector<std::string>& args);

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Type `SZSTool help` for more info on this tool" << std::endl;
        return EXIT_SUCCESS;
    }
    
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
        args.push_back(argv[i]);
    }

    if (args[0] == "help")
    {
        return cmdHelp(args);
    }
    else if (args[0] == "create")
    {
        return cmdCreate(args);
    }
    else if (args[0] == "extract")
    {
        return cmdExtract(args);
    }
    else if (args[0] == "list")
    {
        return cmdList(args);
    }
    else
    {
        std::cout << "Unknown command! Type `SZSTool help` for help" << std::endl;
        return EXIT_FAILURE;
    }
}

int cmdHelp(std::vector<std::string>& args)
{
    if (args.size() == 1)
    {
        std::cout << "SYNTAX: SZSTool <command> [...]" << std::endl;
        std::cout << std::endl;
        std::cout << "Type `SZSTool help <command>` for help on command" << std::endl;
        std::cout << std::endl;
        std::cout << "COMMANDS: " << std::endl;
        std::cout << "  help       Prints help for a specific command" << std::endl;
        std::cout << "  create     Creates an archive from the contents of a directory" << std::endl;
        std::cout << "  extract    Extracts the contents of an archive" << std::endl;
        std::cout << "  list       Lists the entries in an archive" << std::endl;
        return EXIT_SUCCESS;
    }
    else if (args.size() == 2)
    {
        if (args[1] == "help")
        {
            std::cout << "SYNTAX: SZSTool help <command>" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (args[1] == "create")
        {
            std::cout << "SYNTAX: SZSTool create <directory> [output]" << std::endl;
            std::cout << std::endl;
            std::cout << "Creates an archive from the contents of a directory" << std::endl;
            std::cout << std::endl;
            std::cout << "ARGUMENTS: " << std::endl;
            std::cout << "  directory  The directory containing the input files" << std::endl;
            std::cout << "  output     OPTIONAL The path to the output archive" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (args[1] == "extract")
        {
            std::cout << "SYNTAX: SZSTool extract <archive> [output]" << std::endl;
            std::cout << std::endl;
            std::cout << "Extracts the contents of an archive" << std::endl;
            std::cout << std::endl;
            std::cout << "ARGUMENTS: " << std::endl;
            std::cout << "  archive    The archive to be extracted" << std::endl;
            std::cout << "  output     OPTIONAL The path to the output directory" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (args[1] == "list")
        {
            std::cout << "SYNTAX: SZSTool list <archive>" << std::endl;
            std::cout << std::endl;
            std::cout << "Lists the contents of an archive" << std::endl;
            std::cout << std::endl;
            std::cout << "ARGUMENTS: " << std::endl;
            std::cout << "  archive    The archive to be listed" << std::endl;
            return EXIT_SUCCESS;
        }
        else
        {
            std::cout << "Unknown command! Type `SZSTool help` for help" << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cout << "Too many arguments! Type `SZSTool help` for help" << std::endl;
        return EXIT_FAILURE;
    }
}

CTLib::U8Arc createArchive(std::filesystem::path path)
{
    CTLib::U8Arc arc;
    for (auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
        std::string rel = "./" + std::filesystem::relative(entry.path(), path).generic_string();
        if (entry.is_directory())
        {
            arc.addDirectoryAbsolute(rel);
        }
        else if (entry.is_regular_file())
        {
            CTLib::Buffer data = CTLib::IO::readFile(entry.path().generic_string());
            arc.addFileAbsolute(rel)->setData(data);
        }
    }
    return arc;
}

CTLib::U8Arc readArchive(std::filesystem::path path)
{
    CTLib::Buffer data = CTLib::IO::readFile(path.generic_string());

    CTLib::Buffer decompressed;
    try
    {
        decompressed = CTLib::Yaz::decompress(data, CTLib::YazFormat::Yaz0);
    }
    catch(const CTLib::YazError& e)
    {
        std::cout << std::endl;
        std::cout << "Invalid SZS archive! Data not Yaz compressed!" << std::endl;
        std::cout << std::endl;
        std::cout << e.what() << std::endl;
        return CTLib::U8Arc();
    }

    try
    {
        return CTLib::U8::read(decompressed);
    }
    catch (const CTLib::U8Error& e)
    {
        std::cout << std::endl;
        std::cout << "Invalid SZS archive! Decompressed data is not U8!" << std::endl;
        std::cout << std::endl;
        std::cout << e.what() << std::endl;
        return CTLib::U8Arc();
    }
}

int cmdCreate(std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        std::cout << "Not enough arguments! Type `SZSTool help create` for help" << std::endl;
        return EXIT_FAILURE;
    }
    else if (args.size() > 3)
    {
        std::cout << "Too many arguments! Type `SZSTool help create` for help" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path inPath = args[1];
    if (!std::filesystem::is_directory(inPath))
    {
        std::cout << "Input is not a directory!" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "Creating archive from source directory..." << std::flush;
    CTLib::U8Arc arc = createArchive(inPath);
    CTLib::Buffer data = CTLib::U8::write(arc);
    std::cout << " Done!" << std::endl;

    std::cout << "Compressing U8 archive..." << std::flush;
    CTLib::Buffer compressed = CTLib::Yaz::compress(data, CTLib::YazFormat::Yaz0);
    std::cout << " Done!" << std::endl;

    std::cout << "Writing SZS archive..." << std::flush;
    std::filesystem::path outPath = args.size() == 3 ? args[2] : inPath.replace_extension(".szs");
    std::filesystem::create_directories(outPath.parent_path());
    CTLib::IO::writeFile(outPath.generic_string(), compressed);
    std::cout << " Done!" << std::endl;

    return EXIT_SUCCESS;
}

// note that this function does not handle names containing illegal characters
void cmdExtractWriteEntriesRecursively(CTLib::U8Dir* parent, std::filesystem::path base)
{
    for (CTLib::U8Entry* entry : *parent)
    {
        std::filesystem::path path = base / entry->getName();
        if (entry->getType() == CTLib::U8EntryType::Directory)
        {
            std::filesystem::create_directory(path);
            cmdExtractWriteEntriesRecursively(entry->asDirectory(), path);
        }
        else if (entry->getType() == CTLib::U8EntryType::File)
        {
            CTLib::IO::writeFile(path.generic_string(), entry->asFile()->getData());
        }
    }
}

int cmdExtract(std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        std::cout << "Not enough arguments! Type `SZSTool help extract` for help" << std::endl;
        return EXIT_FAILURE;
    }
    else if (args.size() > 3)
    {
        std::cout << "Too many arguments! Type `SZSTool help extract` for help" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path inPath = args[1];
    if (!std::filesystem::is_regular_file(inPath))
    {
        std::cout << "Input is a directory or non-existent!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Reading SZS archive..." << std::flush;
    CTLib::U8Arc arc = readArchive(inPath);
    if (arc.totalCount() == 0)
    {
        return EXIT_FAILURE;
    }
    std::cout << " Done!" << std::endl;
    
    std::filesystem::path outPath = args.size() == 3 ? args[2] :
        inPath.replace_extension(inPath.has_extension() ? "" : ".d");

    std::filesystem::create_directories(outPath.parent_path());

    std::cout << "Writing extracted files..." << std::flush;
    cmdExtractWriteEntriesRecursively(arc.asDirectory(), outPath);
    std::cout << " Done!" << std::endl;

    return EXIT_SUCCESS;
}

void cmdListPrintEntriesRecursively(CTLib::U8Dir* parent)
{
    constexpr const char* format = "  %-6s %8d %-2s %s";
    
    for (CTLib::U8Entry* entry : *parent)
    {
        if (entry->getType() == CTLib::U8EntryType::Directory)
        {
            CTLib::U8Dir* dir = entry->asDirectory();
            std::cout << CTLib::Strings::format(
                    format, "DIR", dir->count(), "", dir->getAbsolutePath().c_str()
                ) << std::endl;
            
            cmdListPrintEntriesRecursively(dir);
        }
        else if (entry->getType() == CTLib::U8EntryType::File)
        {
            CTLib::U8File* file = entry->asFile();
            std::cout << CTLib::Strings::format(
                    format, "FILE", file->getDataSize(), "B", file->getAbsolutePath().c_str()
                ) << std::endl;
        }
    }
}

int cmdList(std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        std::cout << "Not enough arguments! Type `SZSTool help list` for help" << std::endl;
        return EXIT_FAILURE;
    }
    else if (args.size() > 2)
    {
        std::cout << "Too many arguments! Type `SZSTool help list` for help" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path inPath = args[1];
    if (!std::filesystem::is_regular_file(inPath))
    {
        std::cout << "Input is a directory or non-existent!" << std::endl;
        return EXIT_FAILURE;
    }

    CTLib::U8Arc arc = readArchive(inPath);
    if (arc.totalCount() == 0)
    {
        return EXIT_FAILURE;
    }

    std::cout << "  Type       Size    Name" << std::endl;
    std::cout << "----------------------------------------------------------------" << std::endl;
    cmdListPrintEntriesRecursively(arc.asDirectory());

    return EXIT_SUCCESS;
}
