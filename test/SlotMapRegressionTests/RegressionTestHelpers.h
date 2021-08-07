#pragma once

#include <gtest/gtest.h>

#include <string>
#include <array>
#include <vector>
#include <thread>
#include <future>


struct TestObj
{
    int         _a;
    char        _b;
    std::string _c;
};

inline bool operator==(const TestObj &lhs, const TestObj &rhs)
{
    return lhs._a == rhs._a &&
           lhs._b == rhs._b &&
           lhs._c == rhs._c;
}

// returns pair<TimeInFunction, numberOfElementsErased>
template <size_t writeCount, typename T, typename U, typename Z>
std::pair<std::chrono::microseconds, size_t> writerEraserFnc(T &keys, U &map, Z genFunc)
{
    long erasedCount {};
    auto timeStart = std::chrono::high_resolution_clock::now();
    for(size_t count{}; count < writeCount; ++count)    
    {
        keys.push_back(map.insert(genFunc()));

        if (auto chanceToDelete = rand() % 10; chanceToDelete == 1)
        {
            auto key_it {keys.begin() + (rand() % keys.size())};
            map.erase(*key_it);
            keys.erase(key_it);
            erasedCount++;
        }
    }
    auto timeEnd = std::chrono::high_resolution_clock::now();
    return {std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart), erasedCount};
}

template <size_t writeCount, typename T, typename U, typename Z>
std::pair<std::chrono::microseconds, size_t> writerFnc (T &keys, U &map, Z genFunc)
{
    auto timeStart = std::chrono::high_resolution_clock::now();
    long count {};
    while(count++ < writeCount)    
        keys.push_back(map.insert(genFunc()));
    auto timeEnd = std::chrono::high_resolution_clock::now();
    
    return {std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart), 0};
}

template<typename T, typename U>
std::tuple<std::chrono::microseconds, size_t, size_t> readerFnc(const T &keys, const U &map, bool &readFlag) 
{
    size_t readCount {};
    size_t errorCount {};

    auto timeStart = std::chrono::high_resolution_clock::now();
    while (readFlag)
    {
        if (!keys.empty())
        {
            try
            {
                *map.find(keys[rand()%keys.size()]);
                __asm("");
                readCount++;
            }
            catch(...) 
            {
                errorCount++;
            }
        }
    }
    auto timeEnd = std::chrono::high_resolution_clock::now();

    return {std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart), readCount, errorCount};
}


template <size_t WriteCount, size_t ReaderCount, typename T, typename U>
void test_SCMP(T& map, U genKeyFunctor, const bool enableErase)
{
    EXPECT_TRUE(map.empty());

    std::vector<typename T::key_type> keys{};
    keys.reserve(WriteCount);

    auto writer_fut = std::async(std::launch::async,
                                 (enableErase) ? writerEraserFnc<WriteCount, decltype(keys), T, U> 
                                               : writerFnc<WriteCount, decltype(keys), T, U>, 
                                 std::ref(keys), std::ref(map), genKeyFunctor);

    bool readFlag{true};
    std::vector<std::future<std::tuple<std::chrono::microseconds, size_t, size_t>>> readers{};
    for (size_t i {}; i < ReaderCount; ++i)
        readers.push_back(std::async(std::launch::async, 
                                     readerFnc<decltype(keys), T>, 
                                     std::ref(keys), std::ref(map), std::ref(readFlag)) );

    auto [writingTimeInMicros, erasedCount] = writer_fut.get();

    readFlag = false;

    std::chrono::microseconds totalReaderTimeInMicros {};
    size_t totalReads {};
    for (auto& reader : readers)
    {
        auto [readerTimeInMicros, readCount, errorCount] = reader.get();

        EXPECT_EQ(0, errorCount);
        totalReaderTimeInMicros += readerTimeInMicros;
        totalReads += readCount;
    }

    std::cout << "Single Produce Multi Consumer statistics:"                      << "\n"
              << "Writer:"                                                        << "\n"
              << "     - total elements written: " << WriteCount                  << "\n"
              << "     - total elements erased: "  << erasedCount                 << "\n"
              << "     - time (in microseconds): " << writingTimeInMicros.count()
              << ((erasedCount == 0) ? (" averaging " + std::to_string(writingTimeInMicros.count()/WriteCount) + " micros per write.\n")
                                    : ("\n"))
              << "Readers:"                                                       << "\n"
              << "     - concurrent readers count: " << ReaderCount               << "\n"
              << "     - Total elements read: "      << totalReads                << "\n"
              << "     - time (in microseconds): "   << totalReaderTimeInMicros.count()
                                    << " averaging " << totalReaderTimeInMicros.count()/totalReads
                                    << " micros per read."                        << "\n" 
            << std::endl;
}


template <typename T, typename U>
void test_MCMP(T& map)
{
    EXPECT_TRUE(map.empty());
 

}
