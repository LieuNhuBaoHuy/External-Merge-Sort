#pragma once
#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <algorithm>
#include <queue>
#include <limits>
#include <random>
#include <cstring>

using DataType = double;

constexpr int ElementsPerPage = 512;
constexpr int PageBytes = sizeof(DataType) * ElementsPerPage;
constexpr int MainMemPages = 3;

constexpr const char* chunkedtail = "chunked.bin";
constexpr const char* mergedhead = "merge_";
constexpr const char* sortedhead = "sorted_";

using Page = std::array<DataType, ElementsPerPage>;
using MainMemory = std::array<Page, MainMemPages>;

constexpr DataType MaxRandomRight = 100;
constexpr DataType MinRandomLeft = -100;
constexpr int TestTotalElements = ElementsPerPage * MainMemPages * 6;
constexpr const char* DefaultTestFileName = "sample.bin";

// --- Macro cho module ExtSort ---
#ifdef EXTSORT_LIBRARY_EXPORTS
#define EXTSORT_API extern "C" __declspec(dllexport)
#else
#define EXTSORT_API extern "C" __declspec(dllimport)
#endif

EXTSORT_API void extsort(const char* inputFileName, char* outputBuffer, int bufferSize);

// --- Macro cho module Testing Sample ---
#ifdef GET_TESTING_SAMPLE_EXPORTS
#define SAMPLE_API extern "C" __declspec(dllexport)
#else
#define SAMPLE_API extern "C" __declspec(dllimport)
#endif

SAMPLE_API void get_testing_sample(unsigned int remainNum , const char* outFileName );