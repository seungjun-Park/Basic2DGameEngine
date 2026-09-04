#include "AudioClipLoader.h"

#include "AudioClip.h"
#include "Engine/Debug/DebugLog.h"

#include <Windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>


namespace
{
    constexpr std::size_t
        MaxAudioDataBytes =
        256ull * 1024ull * 1024ull;


    bool ReadExact(
        std::ifstream& file,
        void* destination,
        std::size_t byteCount)
    {
        if (byteCount == 0)
        {
            return true;
        }

        file.read(
            static_cast<char*>(
                destination
                ),
            static_cast<std::streamsize>(
                byteCount
                )
        );

        return
            file.good() ||
            file.gcount() ==
            static_cast<std::streamsize>(
                byteCount
                );
    }


    bool ReadUInt16(
        const std::vector<std::uint8_t>& data,
        std::size_t offset,
        std::uint16_t& value)
    {
        if (offset + 2 >
            data.size())
        {
            return false;
        }

        value =
            static_cast<std::uint16_t>(
                data[offset]
                ) |
            static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(
                    data[offset + 1]
                    ) << 8
                );

        return true;
    }


    bool ReadUInt32(
        const std::vector<std::uint8_t>& data,
        std::size_t offset,
        std::uint32_t& value)
    {
        if (offset + 4 >
            data.size())
        {
            return false;
        }

        value =
            static_cast<std::uint32_t>(
                data[offset]
                ) |
            (
                static_cast<std::uint32_t>(
                    data[offset + 1]
                    ) << 8
                ) |
            (
                static_cast<std::uint32_t>(
                    data[offset + 2]
                    ) << 16
                ) |
            (
                static_cast<std::uint32_t>(
                    data[offset + 3]
                    ) << 24
                );

        return true;
    }


    bool FourCCEquals(
        const std::array<char, 4>& value,
        const char expected[5])
    {
        return
            value[0] == expected[0] &&
            value[1] == expected[1] &&
            value[2] == expected[2] &&
            value[3] == expected[3];
    }
}


std::unique_ptr<AudioClip>
AudioClipLoader::Load(
    const std::wstring& path)
{
    std::ifstream file(
        std::filesystem::path(
            path
        ),
        std::ios::binary
    );

    if (!file)
    {
        AUDIOLOADER_DEBUG_LOG(
            "could not open file."
        );

        return nullptr;
    }


    //
    // RIFF header:
    //
    // 4 bytes "RIFF"
    // 4 bytes file/chunk size
    // 4 bytes "WAVE"
    //
    std::array<char, 4>
        riffId{};

    std::uint32_t riffSize =
        0;

    std::array<char, 4>
        waveId{};


    if (!ReadExact(
        file,
        riffId.data(),
        riffId.size()
    ) ||
        !ReadExact(
            file,
            &riffSize,
            sizeof(riffSize)
        ) ||
        !ReadExact(
            file,
            waveId.data(),
            waveId.size()
        ))
    {
        AUDIOLOADER_DEBUG_LOG(
            "truncated RIFF header."
        );

        return nullptr;
    }


    if (!FourCCEquals(
        riffId,
        "RIFF"))
    {
        AUDIOLOADER_DEBUG_LOG(
            "file is not RIFF."
        );

        return nullptr;
    }


    if (!FourCCEquals(
        waveId,
        "WAVE"))
    {
        AUDIOLOADER_DEBUG_LOG(
            "RIFF file is not WAVE."
        );

        return nullptr;
    }


    WAVEFORMATEX format{};

    bool foundFormat =
        false;

    bool foundData =
        false;

    std::vector<std::uint8_t>
        audioData;


    while (file)
    {
        std::array<char, 4>
            chunkId{};

        std::uint32_t chunkSize =
            0;


        if (!ReadExact(
            file,
            chunkId.data(),
            chunkId.size()
        ))
        {
            break;
        }


        if (!ReadExact(
            file,
            &chunkSize,
            sizeof(chunkSize)
        ))
        {
            AUDIOLOADER_DEBUG_LOG(
                "truncated chunk header."
            );

            return nullptr;
        }


        if (FourCCEquals(
            chunkId,
            "fmt "))
        {
            //
            // Standard PCM fmt payload는
            // 최소 16 bytes.
            //
            if (chunkSize < 16)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "fmt chunk is too small."
                );

                return nullptr;
            }


            //
            // fmt chunk가 비정상적으로 거대하면
            // 불필요한 allocation 방지.
            //
            if (chunkSize >
                64 * 1024)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "fmt chunk is too large."
                );

                return nullptr;
            }


            std::vector<std::uint8_t>
                formatData(
                    chunkSize
                );


            if (!ReadExact(
                file,
                formatData.data(),
                formatData.size()
            ))
            {
                AUDIOLOADER_DEBUG_LOG(
                    "truncated fmt chunk."
                );

                return nullptr;
            }


            std::uint16_t formatTag =
                0;

            std::uint16_t channels =
                0;

            std::uint32_t sampleRate =
                0;

            std::uint32_t averageBytes =
                0;

            std::uint16_t blockAlign =
                0;

            std::uint16_t bitsPerSample =
                0;


            if (!ReadUInt16(
                formatData,
                0,
                formatTag
            ) ||
                !ReadUInt16(
                    formatData,
                    2,
                    channels
                ) ||
                !ReadUInt32(
                    formatData,
                    4,
                    sampleRate
                ) ||
                !ReadUInt32(
                    formatData,
                    8,
                    averageBytes
                ) ||
                !ReadUInt16(
                    formatData,
                    12,
                    blockAlign
                ) ||
                !ReadUInt16(
                    formatData,
                    14,
                    bitsPerSample
                ))
            {
                AUDIOLOADER_DEBUG_LOG(
                    "invalid fmt chunk."
                );

                return nullptr;
            }


            //
            // Phase 13-B contract:
            // uncompressed integer PCM only.
            //
            if (formatTag !=
                WAVE_FORMAT_PCM)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "only PCM WAV is supported."
                );

                return nullptr;
            }


            if (channels == 0 ||
                sampleRate == 0 ||
                blockAlign == 0 ||
                averageBytes == 0 ||
                bitsPerSample == 0)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "invalid PCM format values."
                );

                return nullptr;
            }


            //
            // 현재 common PCM sample widths만
            // 명시적으로 허용한다.
            //
            if (bitsPerSample != 8 &&
                bitsPerSample != 16 &&
                bitsPerSample != 24 &&
                bitsPerSample != 32)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "unsupported PCM bit depth."
                );

                return nullptr;
            }


            const std::uint32_t
                expectedBlockAlign =
                static_cast<std::uint32_t>(
                    channels
                    ) *
                static_cast<std::uint32_t>(
                    bitsPerSample / 8
                    );


            if (blockAlign !=
                expectedBlockAlign)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "invalid PCM block alignment."
                );

                return nullptr;
            }


            const std::uint64_t
                expectedAverageBytes =
                static_cast<std::uint64_t>(
                    sampleRate
                    ) *
                static_cast<std::uint64_t>(
                    blockAlign
                    );


            if (expectedAverageBytes >
                std::numeric_limits<
                std::uint32_t
                >::max())
            {
                AUDIOLOADER_DEBUG_LOG(
                    "PCM byte rate overflow."
                );

                return nullptr;
            }


            if (averageBytes !=
                static_cast<std::uint32_t>(
                    expectedAverageBytes
                    ))
            {
                AUDIOLOADER_DEBUG_LOG(
                    "invalid PCM byte rate."
                );

                return nullptr;
            }


            format.wFormatTag =
                WAVE_FORMAT_PCM;

            format.nChannels =
                channels;

            format.nSamplesPerSec =
                sampleRate;

            format.nAvgBytesPerSec =
                averageBytes;

            format.nBlockAlign =
                blockAlign;

            format.wBitsPerSample =
                bitsPerSample;

            format.cbSize =
                0;


            foundFormat =
                true;
        }
        else if (
            FourCCEquals(
                chunkId,
                "data"
            ))
        {
            if (foundData)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "multiple data chunks "
                    "are not supported."
                );

                return nullptr;
            }


            if (chunkSize == 0)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "empty data chunk."
                );

                return nullptr;
            }


            if (
                static_cast<std::uint64_t>(
                    chunkSize
                    ) >
                MaxAudioDataBytes)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "audio data is too large."
                );

                return nullptr;
            }


            audioData.resize(
                chunkSize
            );


            if (!ReadExact(
                file,
                audioData.data(),
                audioData.size()
            ))
            {
                AUDIOLOADER_DEBUG_LOG(
                    "truncated audio data."
                );

                return nullptr;
            }


            foundData =
                true;
        }
        else
        {
            //
            // LIST, JUNK, fact 등
            // 모르는 RIFF chunk를 안전하게 skip.
            //
            file.seekg(
                static_cast<
                std::streamoff
                >(chunkSize),
                std::ios::cur
            );

            if (!file)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "failed to skip RIFF chunk."
                );

                return nullptr;
            }
        }


        //
        // RIFF chunks are word-aligned.
        //
        if ((chunkSize & 1u) != 0)
        {
            file.seekg(
                1,
                std::ios::cur
            );

            if (!file)
            {
                AUDIOLOADER_DEBUG_LOG(
                    "invalid RIFF padding."
                );

                return nullptr;
            }
        }
    }


    if (!foundFormat)
    {
        AUDIOLOADER_DEBUG_LOG(
            "fmt chunk was not found."
        );

        return nullptr;
    }


    if (!foundData)
    {
        AUDIOLOADER_DEBUG_LOG(
            "data chunk was not found."
        );

        return nullptr;
    }


    if (
        audioData.size() %
        format.nBlockAlign != 0)
    {
        AUDIOLOADER_DEBUG_LOG(
            "audio data is not frame aligned."
        );

        return nullptr;
    }


    auto clip =
        std::make_unique<
        AudioClip
        >(
            format,
            std::move(audioData)
        );


    if (!clip->IsValid())
    {
        AUDIOLOADER_DEBUG_LOG(
            "constructed AudioClip is invalid."
        );

        return nullptr;
    }


    ENGINE_DEBUG_LOG(
        "[Audio] PCM WAV loaded.\n"
    );


    return clip;
}