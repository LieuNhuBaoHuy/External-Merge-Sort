#include "pch.h"
#include "All_Purpose.h" 

size_t load_bin_to_page(std::ifstream& file, Page& page)
{
    file.read(reinterpret_cast<char*>(page.data()), sizeof(page));
    return file.gcount() / sizeof(DataType);
}

void push_page_to_bin(std::ofstream& file, Page& page, size_t elements)
{
    if (elements > 0)
    {
        file.write(reinterpret_cast<char*>(page.data()),
            elements * sizeof(DataType));
    }
}

void chunking(const std::string& inFileName, std::queue<std::string>& chunked_list)
{
    std::ifstream inFile(inFileName, std::ios::binary);
    MainMemory buffer;

    for (int loop_count = 0; inFile; ++loop_count)
    {
        inFile.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));

        size_t total_elements = inFile.gcount() / sizeof(DataType);
        if (total_elements == 0)
            break;

        DataType* startPtr = &buffer[0][0];
        std::sort(startPtr, startPtr + total_elements);

        std::string outFileName = std::to_string(loop_count) + chunkedtail;
        std::ofstream outFile(outFileName, std::ios::binary);

        outFile.write(reinterpret_cast<char*>(&buffer),
            total_elements * sizeof(DataType));

        chunked_list.push(outFileName);
        outFile.close();
    }

    inFile.close();
    remove(inFileName.c_str());
}

void merge_2sorted_to_1sorted(const std::string& inFileName1, const std::string& inFileName2, const std::string& outFileName)
{
    std::ifstream inFile1(inFileName1, std::ios::binary);
    std::ifstream inFile2(inFileName2, std::ios::binary);
    std::ofstream outFile(outFileName, std::ios::binary);

    Page buffer[3];

    size_t e1 = 0, lim_e1 = 0;
    size_t e2 = 0, lim_e2 = 0;
    size_t e3 = 0;

    while (true)
    {
        if (e1 >= lim_e1)
        {
            lim_e1 = load_bin_to_page(inFile1, buffer[0]);
            e1 = 0;
        }

        if (e2 >= lim_e2)
        {
            lim_e2 = load_bin_to_page(inFile2, buffer[1]);
            e2 = 0;
        }

        if (lim_e1 == 0 && lim_e2 == 0)
            break;

        DataType value;

        if (e1 < lim_e1 && e2 < lim_e2)
        {
            if (buffer[0][e1] <= buffer[1][e2])
                value = buffer[0][e1++];
            else
                value = buffer[1][e2++];
        }
        else if (e1 < lim_e1)
        {
            value = buffer[0][e1++];
        }
        else
        {
            value = buffer[1][e2++];
        }

        buffer[2][e3++] = value;

        if (e3 == ElementsPerPage)
        {
            push_page_to_bin(outFile, buffer[2], ElementsPerPage);
            e3 = 0;
        }
    }

    if (e3 > 0)
        push_page_to_bin(outFile, buffer[2], e3);

    inFile1.close();
    remove(inFileName1.c_str());

    inFile2.close();
    remove(inFileName2.c_str());

    outFile.close();
}

// ================= CORE FUNCTION =================

std::string extsort_core(const std::string& inputFileName)
{
    std::queue<std::string> chunked_list;
    chunking(inputFileName, chunked_list);

    int chunk_count = 0;
    while (chunked_list.size() > 1)
    {
        std::string name1 = chunked_list.front(); chunked_list.pop();
        std::string name2 = chunked_list.front(); chunked_list.pop();

        // ??m b?o file t?m merge_ c?ng n?m trong vùng an toàn
        std::string outputName = std::string(mergedhead) + std::to_string(chunk_count++) + ".bin";
        merge_2sorted_to_1sorted(name1, name2, outputName);
        chunked_list.push(outputName);
    }

    // TÌM TÊN FILE AN TOÀN: Thay vì c?ng chu?i tr?c ti?p, hãy l?y tên file
    // Ví d?: input là "C:/Temp/data.bin" -> output nên là "C:/Temp/sorted_data.bin"
    size_t last_slash = inputFileName.find_last_of("\\/");
    std::string path = (last_slash == std::string::npos) ? "" : inputFileName.substr(0, last_slash + 1);
    std::string filename = (last_slash == std::string::npos) ? inputFileName : inputFileName.substr(last_slash + 1);

    std::string ResultFileName = path + std::string(sortedhead) + filename;

    if (!chunked_list.empty())
    {
        std::string FinalFileName = chunked_list.front();
        chunked_list.pop();
        rename(FinalFileName.c_str(), ResultFileName.c_str());
    }

    return ResultFileName;
}

// ================= EXPORT WRAPPER =================

void extsort(const char* inputFileName, char* outputBuffer, int bufferSize)
{
    if (!inputFileName || !outputBuffer || bufferSize <= 0)
        return;

    std::string result = extsort_core(std::string(inputFileName));

    if (result.size() + 1 <= static_cast<size_t>(bufferSize))
    {
        strcpy_s(outputBuffer, bufferSize, result.c_str());
    }
}