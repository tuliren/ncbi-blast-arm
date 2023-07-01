/*  $Id: test_psg_client_impl.cpp 660267 2022-12-13 13:05:07Z ivanov $
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
 * Authors: Rafael Sadyrov
 *
 */

#include <ncbi_pch.hpp>

#include "../psg_client_impl.hpp"

#ifdef HAVE_PSG_CLIENT

#include <corelib/test_boost.hpp>

#include <deque>
#include <thread>
#include <random>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;

const char kAllowedChars[] = "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

struct SRandom
{
    SRandom() :
        m_Engine(random_device()()),
        m_CharsDistribution(0, sizeof(kAllowedChars) - 2)
    {
    }

    size_t Get(size_t min = 0, size_t max = numeric_limits<size_t>::max())
    {
        assert(min <= max);
        return uniform_int_distribution<size_t>(min, max)(m_Engine);
    }

    string GetString(size_t length)
    {
        auto get_char = [&]() { return kAllowedChars[m_CharsDistribution(m_Engine)]; };

        string result;
        generate_n(back_inserter(result), length, get_char);
        return result;
    }

    void Fill(char* buf, size_t size)
    {
        while (size > 0) {
            auto random = m_Distribution(m_Engine);
            auto to_copy = min(size, sizeof(random));
            memcpy(buf, reinterpret_cast<char*>(&random), to_copy);
            buf += to_copy;
            size -= to_copy;
        }
    }

    template <class TIterator>
    void Shuffle(TIterator first, TIterator last)
    {
        shuffle(first, last, m_Engine);
    }

private:
    mt19937 m_Engine;
    uniform_int_distribution<size_t> m_Distribution;
    uniform_int_distribution<size_t> m_CharsDistribution;
};

struct SFixture
{
    using TData = vector<char>;

    static thread_local SRandom r;
    unordered_map<string, TData> src_blobs;
    deque<stringstream> src_chunks;

    SFixture();
    void Reset();

    template <class TReadImpl>
    void MtReading();
};

thread_local SRandom SFixture::r;

void s_OutputArgs(ostream& os, SRandom& r, vector<string> args)
{
    r.Shuffle(args.begin(), args.end());

    const char* delim = "\n\nPSG-Reply-Chunk: ";

    for (auto& arg : args) {
        os << delim << arg;
        delim = "&";
    }

    os << '\n';
}

vector<string> s_GetReplyMetaArgs(size_t n_chunks)
{
    return {
        "item_type=reply",
        "chunk_type=meta",
        "item_id=0",
        "n_chunks=" + to_string(n_chunks)
    };
}

vector<string> s_GetBlobMetaArgs(size_t item_id, const string& blob_id, size_t n_chunks)
{
    return {
        "item_type=blob",
        "chunk_type=meta",
        "item_id=" + to_string(item_id),
        "blob_id=" + blob_id,
        "n_chunks=" + to_string(n_chunks)
    };
}

vector<string> s_GetBlobDataArgs(size_t item_id, const string& blob_id, size_t chunk, size_t size)
{
    return {
        "item_type=blob",
        "chunk_type=data",
        "item_id=" + to_string(item_id),
        "blob_id=" + blob_id,
        "blob_chunk=" + to_string(chunk),
        "size=" + to_string(size)
    };
}

vector<string> s_GetBlobMessageArgs(size_t item_id, const string& blob_id, const string& severity, size_t size)
{
    return {
        "item_type=blob",
        "chunk_type=message",
        "item_id=" + to_string(item_id),
        "blob_id=" + blob_id,
        "severity=" + severity,
        "size=" + to_string(size)
    };
}

SFixture::SFixture()
{
    const size_t kBlobsMin = 3;
    const size_t kBlobsMax = 11;
    const size_t kChunksMin = 3;
    const size_t kChunksMax = 17;
    const size_t kSizeMin = 100 * 1024;
    const size_t kSizeMax = 1024 * 1024;
    const size_t kMessagesMin = 0;
    const size_t kMessagesMax = 3;
    const size_t kMessageSizeMin = 20;
    const size_t kMessageSizeMax = 100;

    SetDiagPostLevel(eDiag_Info);

    // Generating source

    auto blobs_number = r.Get(kBlobsMin, kBlobsMax);
    src_blobs.reserve(blobs_number);
    vector<char> buf(kSizeMax);

    while (blobs_number > 0) {
        auto blob_id = "id_" + to_string(r.Get());
        auto rv = src_blobs.emplace(blob_id, TData());

        // Blob ID already taken
        if (!rv.second) continue;

        auto& blob_data = rv.first->second;
        auto chunks_number = r.Get(kChunksMin, kChunksMax);
        auto messages_number = r.Get(kMessagesMin, kMessagesMax);
        auto n_chunks = chunks_number + messages_number + 1;

        src_chunks.emplace_back();
        s_OutputArgs(src_chunks.back(), r, s_GetBlobMetaArgs(blobs_number, blob_id, n_chunks));

        for (size_t i = 0; i < chunks_number; ++i) {
            src_chunks.emplace_back();
            auto& chunk_stream = src_chunks.back();
            auto chunk_size = r.Get(kSizeMin, kSizeMax);

            s_OutputArgs(src_chunks.back(), r, s_GetBlobDataArgs(blobs_number, blob_id, i, chunk_size));
            r.Fill(buf.data(), chunk_size);
            chunk_stream.write(buf.data(), chunk_size);
            blob_data.insert(blob_data.end(), &buf[0], &buf[chunk_size]);
        }

        for (size_t i = 0; i < messages_number; ++i) {
            src_chunks.emplace_back();
            auto& message_stream = src_chunks.back();
            auto message_size = r.Get(kMessageSizeMin, kMessageSizeMax);
            string severity;

            switch (r.Get(0, 2)) {
                case 0:  severity = "info";    break;
                case 1:  severity = "warning"; break;
                default: severity = "trace";
            }

            s_OutputArgs(src_chunks.back(), r, s_GetBlobMessageArgs(blobs_number, blob_id, severity, message_size));
            message_stream << r.GetString(message_size);
        }

        --blobs_number;
    }

    src_chunks.emplace_back();
    s_OutputArgs(src_chunks.back(), r, s_GetReplyMetaArgs(src_chunks.size()));

    r.Shuffle(src_chunks.begin(), src_chunks.end());
}

void SFixture::Reset()
{
    for (auto& ss : src_chunks) {
        ss.clear();
        ss.seekg(0);
    }
}

template <class TReadImpl>
void SFixture::MtReading()
{
    Reset();

    const size_t kSizeMin = 100 * 1024;
    constexpr static size_t kSizeMax = 1024 * 1024;
    const size_t kSleepMin = 5;
    const size_t kSleepMax = 13;
    const unsigned kReadingDeadline = 300;

    const SPSG_Params params;
    auto queue = make_shared<TPSG_Queue>();
    auto reply = make_shared<SPSG_Reply>("", params, queue);
    map<SPSG_Reply::SItem::TTS*, thread> readers;


    // Reading

    auto reader_impl = [&](const vector<char>& src, SPSG_Reply::SItem::TTS& dst) {
        TReadImpl read_impl(dst);
        vector<char> received(kSizeMax);
        auto expected = src.data();
        size_t expected_to_read = src.size();
        CDeadline deadline(kReadingDeadline, 0);

        while (!deadline.IsExpired()) {
            size_t read = 0;
            auto reading_result = read_impl(r, received.data(), received.size(), expected_to_read, &read);

            if (reading_result < 0) return;

            BOOST_REQUIRE_MESSAGE_MT_SAFE(read <= expected_to_read, "Received more data than expected");
            BOOST_REQUIRE_MESSAGE_MT_SAFE(equal(&received[0], &received[read], expected), "Received data does not match expected");

            expected += read;
            expected_to_read -= read;

            if (reading_result == 0) break;

            auto ms = chrono::milliseconds(r.Get(kSleepMin, kSleepMax));
            this_thread::sleep_for(ms);
        }

        BOOST_REQUIRE_MESSAGE_MT_SAFE(!expected_to_read, "Got less data that expected");
    };

    auto dispatcher_impl = [&]() {
        CDeadline deadline(kReadingDeadline, 0);

        for (;;) {
            bool empty_items = false;

            if (auto items_locked = reply->items.GetLock()) {
                auto& items = *items_locked;

                for (auto& item_ts : items) {
                    auto reader = readers.find(&item_ts);

                    if (reader == readers.end()) {
                        auto item_locked = item_ts.GetLock();
                        auto& chunks = item_locked->chunks;

                        if (chunks.empty()) {
                            empty_items = true;
                        } else {
                            auto blob_id = item_locked->args.GetValue("blob_id");
                            auto src_blob = src_blobs.find(blob_id);

                            BOOST_REQUIRE_MESSAGE_MT_SAFE(src_blob != src_blobs.end(), "Unknown blob received");

                            thread t = thread(reader_impl, src_blob->second, ref(item_ts));
                            readers.emplace(&item_ts, move(t));
                        }
                    }
                }
            }

            if (readers.size() == src_blobs.size()) break;

            if (empty_items) {
                reply->reply_item.WaitUntil(CTimeout(0, 1));

            } else if (!reply->reply_item.WaitUntil(deadline)) {
                break;
            }
        }

        BOOST_REQUIRE_MESSAGE_MT_SAFE(readers.size() >= src_blobs.size(), "Got less blobs that expected");

        for (auto& reader : readers) {
            if (reader.second.joinable()) reader.second.join();
        }
    };

    thread dispatcher(dispatcher_impl);


    // Sending

    vector<char> buf(kSizeMax);
    SPSG_Request request(string(), reply, CDiagContext::GetRequestContext().Clone(), params);

    for (auto& chunk_stream : src_chunks) {
        do {
            chunk_stream.read(buf.data(), r.Get(kSizeMin, kSizeMax));

            if (auto read = chunk_stream.gcount()) {
                request.OnReplyData(TPSG_ProcessorId{}, buf.data(), read);
            }

            auto ms = chrono::milliseconds(r.Get(kSleepMin, kSleepMax));
            this_thread::sleep_for(ms);
        } while (chunk_stream);
    }

    reply->SetComplete();


    // Waiting

    dispatcher.join();
}

BOOST_FIXTURE_TEST_SUITE(PSG, SFixture)

BOOST_AUTO_TEST_CASE(Request)
{
    Reset();

    const size_t kSizeMin = 100 * 1024;
    const size_t kSizeMax = 1024 * 1024;

    const SPSG_Params params;
    auto queue = make_shared<TPSG_Queue>();
    auto reply = make_shared<SPSG_Reply>("", params, queue);


    // Reading

    vector<char> buf(kSizeMax);
    SPSG_Request request(string(), reply, CDiagContext::GetRequestContext().Clone(), params);

    for (auto& chunk_stream : src_chunks) {
        do {
            chunk_stream.read(buf.data(), r.Get(kSizeMin, kSizeMax));

            if (auto read = chunk_stream.gcount()) {
                request.OnReplyData(TPSG_ProcessorId{}, buf.data(), read);
            }
        } while (chunk_stream);
    }

    reply->SetComplete();


    // Checking

    auto items_locked = reply->items.GetLock();
    auto& items = *items_locked;

    for (auto& item_ts : items) {
        auto item_locked = item_ts.GetLock();
        auto& item = *item_locked;
        auto& expected = item.expected;
        auto& received = item.received;

        BOOST_REQUIRE_MESSAGE(!expected.Cmp<greater>(received), "Expected is greater than received");
        BOOST_REQUIRE_MESSAGE(!expected.Cmp<less>(received), "Expected is less than received");

        auto& chunks = item.chunks;
        auto blob_id = item.args.GetValue("blob_id");

        auto src_blob = src_blobs.find(blob_id);

        BOOST_REQUIRE_MESSAGE(src_blob != src_blobs.end(), "Unknown blob received");

        {
            auto src_current = src_blob->second.begin();
            auto src_end = src_blob->second.end();

            for (auto& chunk : chunks) {
                auto dst_current = chunk.begin();
                auto dst_end = chunk.end();

                auto src_to_compare = distance(src_current, src_end);
                auto dst_to_compare = distance(dst_current, dst_end);

                BOOST_REQUIRE_MESSAGE(dst_to_compare <= src_to_compare, "Received more data than sent");
                BOOST_REQUIRE_MESSAGE(equal(dst_current, dst_end, src_current), "Received data does not match expected");

                advance(src_current, dst_to_compare);
            }

            BOOST_REQUIRE_MESSAGE(src_current == src_end, "Received less data than sent");
        }
    }
}

struct SBlobReader
{
    SBlobReader(SPSG_Reply::SItem::TTS& dst) : reader(dst) {}

    int operator()(SRandom& r, char* buf, size_t buf_size, size_t expected, size_t* read)
    {
        assert(buf);
        assert(read);

        auto pending_result = reader.PendingCount(read);

        BOOST_REQUIRE_MESSAGE_MT_SAFE((pending_result == eRW_Success) || (pending_result == eRW_Eof), "PendingCount() failed");
        BOOST_REQUIRE_MESSAGE_MT_SAFE(*read <= expected, "Pending data is more than expected");

        auto to_read = r.Get(1, buf_size);
        auto reading_result = eRW_Success;

        try {
            reading_result = reader.Read(buf, to_read, read);
        }
        catch (CPSG_Exception& ex) {
            BOOST_ERROR_MT_SAFE("Read() exception: " << ex.GetErrCodeString());
            return -1;
        }
        catch (...) {
            BOOST_ERROR_MT_SAFE("Read() exception: Unknown");
            return -1;
        }

        if (reading_result == eRW_Eof)     return 0;
        if (reading_result == eRW_Success) return 1;

        BOOST_ERROR_MT_SAFE("Read() failed: " << g_RW_ResultToString(reading_result));
        return -1;
    }

private:
    SPSG_BlobReader reader;
};

BOOST_AUTO_TEST_CASE(BlobReader)
{
    auto valgrind = getenv("NCBI_RUN_UNDER_VALGRIND");

    if (valgrind && !NStr::strcasecmp(valgrind, "yes")) {
        TPSG_ReaderTimeout::SetDefault(60);
    }

    MtReading<SBlobReader>();
}

struct SStreamReadsome
{
    SStreamReadsome(SPSG_Reply::SItem::TTS& dst) : is(dst) {}

    int operator()(SRandom& r, char* buf, size_t buf_size, size_t, size_t* read)
    {
        auto to_read = r.Get(1, buf_size);
        *read = is.readsome(buf, to_read);

        if (*read) {
            return 1;
        } else if (is.eof()) {
            return 0;
        } else {
            return -1;
        }
    }

private:
    SPSG_RStream is;
};

BOOST_AUTO_TEST_CASE(StreamReadsome)
{
    MtReading<SStreamReadsome>();
}

struct SStreamRead
{
    SStreamRead(SPSG_Reply::SItem::TTS& dst) : is(dst) {}

    int operator()(SRandom& r, char* buf, size_t buf_size, size_t, size_t* read)
    {
        auto to_read = r.Get(1, buf_size);

        if (is.read(buf, to_read)) {
            *read = is.gcount();
            return 1;
        } else if (is.eof()) {
            *read = is.gcount();
            return 0;
        } else {
            return -1;
        }
    }

private:
    SPSG_RStream is;
};

BOOST_AUTO_TEST_CASE(StreamRead)
{
    MtReading<SStreamRead>();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(PSG)

void s_CompilationTest()
{
    string seq_id;
    CPSG_BioId bio_id(seq_id);
    CPSG_BioIds bio_ids{bio_id};
    CPSG_Request_NamedAnnotInfo::TAnnotNames annot_names;

    auto user_context = make_shared<string>();
    auto request_context = CRef<CRequestContext>();

    CPSG_Request_Biodata biodata[] =
    {
        { seq_id },
        { seq_id, user_context },
        { seq_id, user_context, request_context },
        { bio_id },
        { bio_id, user_context },
        { bio_id, user_context, request_context },
        { seq_id, EPSG_BioIdResolution::NoResolve },
        { seq_id, EPSG_BioIdResolution::NoResolve, user_context },
        { seq_id, EPSG_BioIdResolution::NoResolve, user_context, request_context },
        { bio_id, EPSG_BioIdResolution::NoResolve },
        { bio_id, EPSG_BioIdResolution::NoResolve, user_context },
        { bio_id, EPSG_BioIdResolution::NoResolve, user_context, request_context },
    };

    CPSG_Request_Resolve resolve[] =
    {
        { seq_id },
        { seq_id, user_context },
        { seq_id, user_context, request_context },
        { bio_id },
        { bio_id, user_context },
        { bio_id, user_context, request_context },
        { seq_id, EPSG_BioIdResolution::NoResolve },
        { seq_id, EPSG_BioIdResolution::NoResolve, user_context },
        { seq_id, EPSG_BioIdResolution::NoResolve, user_context, request_context },
        { bio_id, EPSG_BioIdResolution::NoResolve },
        { bio_id, EPSG_BioIdResolution::NoResolve, user_context },
        { bio_id, EPSG_BioIdResolution::NoResolve, user_context, request_context },
    };

    CPSG_Request_NamedAnnotInfo named_annot[] =
    {
        { seq_id,  annot_names },
        { seq_id,  annot_names, user_context },
        { seq_id,  annot_names, user_context, request_context },
        { bio_id,  annot_names },
        { bio_id,  annot_names, user_context },
        { bio_id,  annot_names, user_context, request_context },
        { bio_ids, annot_names },
        { bio_ids, annot_names, user_context },
        { bio_ids, annot_names, user_context, request_context },
        { seq_id,  annot_names, EPSG_BioIdResolution::NoResolve },
        { seq_id,  annot_names, EPSG_BioIdResolution::NoResolve, user_context },
        { seq_id,  annot_names, EPSG_BioIdResolution::NoResolve, user_context, request_context },
        { bio_id,  annot_names, EPSG_BioIdResolution::NoResolve },
        { bio_id,  annot_names, EPSG_BioIdResolution::NoResolve, user_context },
        { bio_id,  annot_names, EPSG_BioIdResolution::NoResolve, user_context, request_context },
        { bio_ids, annot_names, EPSG_BioIdResolution::NoResolve },
        { bio_ids, annot_names, EPSG_BioIdResolution::NoResolve, user_context },
        { bio_ids, annot_names, EPSG_BioIdResolution::NoResolve, user_context, request_context },
    };
}

SPSG_UserArgs s_Build(SPSG_UserArgsBuilder& builder, const SPSG_UserArgs& request_args)
{
    ostringstream os;
    builder.Build(os, request_args);
    return os.str();
}

BOOST_AUTO_TEST_CASE(UserArgsBuilder)
{
    s_CompilationTest();

    TPSG_RequestUserArgs::SetDefault("enable_processor=cdd&enable_processor=osg&hops=3");
    SPSG_UserArgsBuilder builder;
    SPSG_UserArgs request_user_args("enable_processor=snp&disable_processor=cdd&hops=2&use_cache=no");

    BOOST_CHECK_EQUAL(s_Build(builder, {}), SPSG_UserArgs("&enable_processor=cdd&enable_processor=osg&hops=3"));
    BOOST_CHECK_EQUAL(s_Build(builder, request_user_args), SPSG_UserArgs("&enable_processor=cdd&enable_processor=osg&enable_processor=snp&hops=3&use_cache=no"));

    builder.SetQueueArgs({{"enable_processor", {"wgs"}}, {"disable_processor", {"snp", "cdd"}}, {"hops", {"1"}}});

    BOOST_CHECK_EQUAL(s_Build(builder, {}), SPSG_UserArgs("&disable_processor=snp&enable_processor=cdd&enable_processor=osg&enable_processor=wgs&hops=3"));
    BOOST_CHECK_EQUAL(s_Build(builder, request_user_args), SPSG_UserArgs("&enable_processor=cdd&enable_processor=osg&enable_processor=snp&enable_processor=wgs&hops=3&use_cache=no"));
}

BOOST_AUTO_TEST_SUITE_END()

#endif
