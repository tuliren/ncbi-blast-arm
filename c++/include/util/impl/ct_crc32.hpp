#ifndef __CT_CRC32_HPP_INCLUDED__
#define __CT_CRC32_HPP_INCLUDED__

/*  $Id: ct_crc32.hpp 661151 2023-01-05 15:03:50Z ivanov $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Sergiy Gotvyanskyy
 *
 * File Description:
 *
 *  crc32   -- constexpr capable crc32 functions
 *
 *
 */
#include <cstddef>
#include <utility>
#include <cstdint>

#include <corelib/ncbistr.hpp>
#include <common/ncbi_export.h>

namespace compile_time_bits
{
    constexpr uint32_t sse42_poly = 0x82f63b78;
    constexpr uint32_t armv8_poly = 0x04c11db7;
    constexpr uint32_t platform_poly = sse42_poly;

    template<uint32_t poly>
    struct ct_crc32
    { // compile time CRC32, C++14 compatible
        using type = uint32_t;

        static constexpr type update(type crc, uint8_t b)
        {
            crc ^= b;
            for (int k{ 0 }; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;

            return crc;
        }

        static constexpr type update4(type crc, type d32)
        {
            size_t len = 4;
            while (len--) {
                uint8_t b = static_cast<uint8_t>(d32);
                d32 = d32 >> 8;
                crc = update(crc, b);
            }
            return crc;
        }

        template<bool _lowercase, size_t N>
        static constexpr type SaltedHash(const char(&s)[N]) noexcept
        {
            type hash{ update4(type(0), type(N - 1))};
            for (size_t i=0; i<N-1; ++i)
            {
                char c = s[i];
                uint8_t b = static_cast<uint8_t>((_lowercase && 'A' <= c && c <= 'Z') ? c + 'a' - 'A' : c);
                hash = update(hash, b);
            }
            return hash;
        }

        class MakeCRC32Table
        {
        public:
            struct cont_t
            {
                uint32_t m_data[256];
            };

            constexpr cont_t operator()() const noexcept
            {
                cont_t ret{};
                for (size_t i=0; i<256; ++i)
                    ret.m_data[i] = update(0, (uint8_t)i);
                return ret;
            }
        };
    }; // ct_crc32
} // namespace compile_time_bits

namespace ct
{
    template<typename _CaseTag>
    struct NCBI_XUTIL_EXPORT SaltedCRC32
    {
        using type = uint32_t;
        static constexpr bool need_lower_case = std::is_same<_CaseTag, compile_time_bits::tagStrNocase>::value;

        template<size_t N>
        static type constexpr ct(const char(&s)[N]) noexcept
        {
            auto hash = compile_time_bits::ct_crc32<compile_time_bits::platform_poly>::SaltedHash<need_lower_case, N>(s);
#if 0
//def NCBI_COMPILER_ICC
// Intel compiler ignores uint32_to int32_t conversion at compile time
            if (hash >= 0x8000'0000)
            {
                int64_t h64 = hash-1;
                h64 ^= 0xFFFF'FFFF;
                return -h64;
            }
#endif
            return hash;
        }
        static type rt(const char* s, size_t realsize) noexcept
        {
#if defined(NCBI_SSE)  &&  NCBI_SSE >= 42
            auto hash = sse42(s, realsize);
#else
            auto hash = general(s, realsize);
#endif
            return hash;
        }
        static uint32_t general(const char* s, size_t realsize) noexcept;
#if defined(NCBI_SSE)  &&  NCBI_SSE >= 42
        static uint32_t sse42(const char* s, size_t realsize) noexcept;
#endif
    };
}

namespace compile_time_bits
{
    template<typename _CaseTag, class _Hash = ct::SaltedCRC32<_CaseTag>>
    class CHashString
    {
    public:
        using hash_func = _Hash;
        using hash_type = typename _Hash::type;
        using sv = ct_string;

        constexpr CHashString() noexcept = default;

        template<size_t N>
        constexpr CHashString(char const (&s)[N]) noexcept
            : m_view{s, N-1},  m_hash{hash_func::ct(s)}
        {}

        CHashString(const sv& s) noexcept
            : m_view{s}, m_hash(hash_func::rt(s.data(), s.size()))
        {}

        constexpr operator hash_type() const noexcept { return m_hash; }
        constexpr operator const sv&() const noexcept { return m_view; }
        constexpr hash_type get_hash() const noexcept { return m_hash; }
        constexpr const sv& get_view() const noexcept { return m_view; }

    private:
        sv        m_view;
        hash_type m_hash{ 0 };
    };

    // this crc32 hashed string, probably nobody will use it
    template<typename _T, _T value>
    struct DeduceHashedType<std::integral_constant<_T, value>>
    {
        using case_tag   = std::integral_constant<_T, value>;
        using init_type  = CHashString<case_tag>;
        using value_type = typename init_type::sv;
        using hash_type  = typename init_type::hash_type;
        using hash_compare   = std::less<hash_type>;
        using value_compare  = std::less<case_tag>;
    };
}

namespace std
{
    template<class _Traits, typename _T, _T value> inline
        basic_ostream<char, _Traits>& operator<<(basic_ostream<char, _Traits>& _Ostr, const compile_time_bits::CHashString<std::integral_constant<_T, value>>& v)
    {
        return operator<<(_Ostr, v.get_view());
    }
}

#endif
