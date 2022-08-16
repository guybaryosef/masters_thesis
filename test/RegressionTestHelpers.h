
#pragma once

#include <gtest/gtest.h>

#include <string>
#include <array>
#include <vector>
#include <thread>
#include <future>


constexpr size_t maxStrLen {50};

extern char alphaNum2[63];

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

template<size_t T>
auto genTestObj()
{
    std::array<TestObj, T> testObjInput {};

    auto genTestObj = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphaNum2[rand() % sizeof(alphaNum2)];
            
            return TestObj{rand(), alphaNum2[rand() % sizeof(alphaNum2)], s};
        };
    std::generate_n(testObjInput.begin(), T, genTestObj);
    return testObjInput;
}

template <size_t T>
auto genStrInput()
{
    std::array<std::string, T> strInput {};

    auto genStr = []
        {
            auto len {rand()%maxStrLen};

            std::string s{};
            s.reserve(len);
            for (int i = 0; i < len; ++i)
                s += alphaNum2[rand() % sizeof(alphaNum2)];
            return s;
        };
    std::generate_n(strInput.begin(), T, genStr);
    return strInput;
}

// returns pair<TimeInFunction, numberOfElementsErased>
template <size_t writeCount, typename T, typename U, typename Z>
std::pair<std::chrono::microseconds, size_t> writerEraserFnc(T &keys, U &map, Z genFunc)
{
    long erasedCount {};
    auto timeStart = std::chrono::high_resolution_clock::now();
    for(size_t count{}; count < writeCount; ++count)    
    {
        keys.emplace_back(map.insert(genFunc()));

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

template <typename T, typename U, typename Z>
std::pair<std::chrono::microseconds, size_t> eraserFnc (T &keys, U &map, bool &readFlag)
{
    long count {};

    auto timeStart = std::chrono::high_resolution_clock::now();
    while(readFlag)
    {
        sleep(0.01);
        size_t key_idx {rand()%keys.size()};
        map.erase(keys[key_idx]);
    }
    auto timeEnd = std::chrono::high_resolution_clock::now();
    
    std::chrono::microseconds timeSlept {10000*count};
    return {std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart) - timeSlept, 0};
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

    std::cout << "Single Producer Multi Consumer statistics:"                     << "\n"
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


template <size_t WriterCount, size_t WriteCountPerWriter, size_t EraserCount, size_t ReaderCount, typename T, typename U>
void test_MCMP(T& map, U genKeyFunctor)
{
    EXPECT_TRUE(map.empty());

    std::vector<std::vector<typename T::key_type>> vecOfKeys{};
    vecOfKeys.reserve(WriterCount);

    std::vector<std::future<std::pair<std::chrono::microseconds, size_t>>> writers{};
    writers.reserve(WriterCount);
    for (size_t i {}; i < WriterCount; ++i)
    {
        vecOfKeys.emplace_back();
        vecOfKeys.back().reserve(WriteCountPerWriter);

        writers.emplace_back(std::async(std::launch::async,
                                writerFnc<WriteCountPerWriter, std::vector<typename T::key_type>, T, U>, 
                                std::ref(vecOfKeys.back()), std::ref(map), genKeyFunctor));
    }

    bool readFlag{true};

    std::vector<std::future<std::pair<std::chrono::microseconds, size_t>>> erasers{};
    auto eraseCount = std::min(EraserCount, WriterCount);
    if (eraseCount > 0)
    {
        erasers.reserve(eraseCount);
        for (size_t i {}; i < eraseCount; ++i)
            erasers.emplace_back(std::async(std::launch::async,
                                            eraserFnc<std::vector<typename T::key_type>, T, U>,
                                            std::ref(vecOfKeys[i]), std::ref(map), std::ref(readFlag)) );
    }

    std::vector<std::future<std::tuple<std::chrono::microseconds, size_t, size_t>>> readers{};
    readers.reserve(ReaderCount);
    for (size_t i {}; i < ReaderCount; ++i)
        readers.emplace_back(std::async(std::launch::async, 
                                        readerFnc<std::vector<typename T::key_type>, T>, 
                                        std::ref(vecOfKeys[rand()%vecOfKeys.size()]), std::ref(map), std::ref(readFlag)) );

    std::chrono::microseconds totalWriteInMicros {};
    size_t totalWrites {WriterCount*WriteCountPerWriter};
    // for (auto& writer : writers)
    // {
    //     auto [writerInMicros, a] = writer.get();
    //     totalWriteInMicros += writerInMicros;
    // }

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

    std::chrono::microseconds totalEraserTimeInMicros {};
    size_t totalErases {};
    for (auto& eraser : erasers)
    {
        auto [eraserTimeInMicros, eraseCount] = eraser.get();
        totalEraserTimeInMicros += eraserTimeInMicros;
        totalErases += eraseCount;
    }

    std::cout << "Multi Producer Multi Consumer statistics:"                      << "\n"
              << "Writers:"                                                       << "\n"
              << "     - concurrent writers count: " << WriterCount               << "\n"
              << "     - total elements written: "   << totalWrites               << "\n"
              << "     - time (in microseconds): "   << totalWriteInMicros.count()
                                     << " averaging "<< (totalWrites == 0 ? 0 : totalWriteInMicros.count()/totalWrites)
                                     << " micros per write."      << "\n"
              << "Erasers:"                                                       << "\n"
              << "     - concurrent erasers count: " << WriterCount               << "\n"
              << "     - total elements erased: "    << totalErases               << "\n"
              << "     - time (in microseconds): "   << totalEraserTimeInMicros.count()
                                     << " averaging "<< (totalErases == 0 ? 0 : totalEraserTimeInMicros.count()/totalErases)
                                     << " micros per write."      << "\n"
              << "Readers:"                                                       << "\n"
              << "     - concurrent readers count: " << ReaderCount               << "\n"
              << "     - Total elements read: "      << totalReads                << "\n"
              << "     - time (in microseconds): "   << totalReaderTimeInMicros.count()
                                     << " averaging "<< (totalReads == 0 ? 0 : totalReaderTimeInMicros.count()/totalReads)
                                     << " micros per read."       << "\n"
              << std::endl;
}
