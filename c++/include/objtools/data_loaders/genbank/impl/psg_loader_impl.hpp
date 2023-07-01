#ifndef OBJTOOLS_DATA_LOADERS_PSG___PSG_LOADER_IMPL__HPP
#define OBJTOOLS_DATA_LOADERS_PSG___PSG_LOADER_IMPL__HPP

/*  $Id: psg_loader_impl.hpp 664254 2023-03-10 13:44:58Z ivanov $
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
 * Author: Eugene Vasilchenko, Aleksey Grichenko
 *
 * File Description: PSG data loader
 *
 * ===========================================================================
 */

#include <corelib/ncbistd.hpp>
#include <objtools/data_loaders/genbank/psg_loader.hpp>
#include <objtools/pubseq_gateway/client/psg_client.hpp>
#include <objtools/data_loaders/genbank/blob_id.hpp>
#include <objtools/data_loaders/genbank/impl/incr_time.hpp>
#include <memory>
#include <vector>

#if defined(HAVE_PSG_LOADER)

BEGIN_NCBI_SCOPE

class CThreadPool;
class CThreadPool_Task;

BEGIN_SCOPE(objects)

struct SPsgBioseqInfo
{
    SPsgBioseqInfo(const CPSG_BioseqInfo& bioseq_info, int lifespan);

    typedef CPSG_Request_Resolve::TIncludeInfo TIncludedInfo;
    typedef CDataLoader::TIds TIds;

    TIncludedInfo included_info;
    CSeq_inst::TMol molecule_type;
    Uint8 length;
    CPSG_BioseqInfo::TState state;
    TTaxId tax_id;
    int hash;
    TGi gi;
    CSeq_id_Handle canonical;
    TIds ids;
    string blob_id;

    CDeadline deadline;

    TIncludedInfo Update(const CPSG_BioseqInfo& bioseq_info);
    CBioseq_Handle::TBioseqStateFlags GetBioseqStateFlags() const;

private:
    SPsgBioseqInfo(const SPsgBioseqInfo&);
    SPsgBioseqInfo& operator=(const SPsgBioseqInfo&);
};


struct SPsgBlobInfo
{
    explicit SPsgBlobInfo(const CPSG_BlobInfo& blob_info);
    explicit SPsgBlobInfo(const CTSE_Info& tse);

    string blob_id_main;
    string id2_info;
    CBioseq_Handle::TBioseqStateFlags blob_state_flags;
    Int8 last_modified;

    int GetBlobVersion() const { return int(last_modified/60000); /* ms to minutes */ }

    bool IsSplit() const { return !id2_info.empty(); }

private:
    SPsgBlobInfo(const SPsgBlobInfo&);
    SPsgBlobInfo& operator=(const SPsgBlobInfo&);
};


class CPSGBioseqCache;
class CPSGAnnotCache;
class CPSGCDDInfoCache;
class CPSGBlobMap;
class CPSG_Blob_Task;
class CPSG_PrefetchCDD_Task;


class CPSGDataLoader_Impl : public CObject
{
public:
    explicit CPSGDataLoader_Impl(const CGBLoaderParams& params);
    ~CPSGDataLoader_Impl(void) override;

    typedef CDataLoader::TIds TIds;
    typedef CDataLoader::TGis TGis;
    typedef CDataLoader::TSequenceLengths TSequenceLengths;
    typedef CDataLoader::TSequenceTypes TSequenceTypes;
    typedef CDataLoader::TLoaded TLoaded;
    typedef CDataLoader::TTSE_LockSets TTSE_LockSets;

    void GetIds(const CSeq_id_Handle& idh, TIds& ids);
    void GetIdsOnce(const CSeq_id_Handle& idh, TIds& ids);
    CDataLoader::SGiFound GetGi(const CSeq_id_Handle& idh);
    CDataLoader::SGiFound GetGiOnce(const CSeq_id_Handle& idh);
    CDataLoader::SAccVerFound GetAccVer(const CSeq_id_Handle& idh);
    CDataLoader::SAccVerFound GetAccVerOnce(const CSeq_id_Handle& idh);
    TTaxId GetTaxId(const CSeq_id_Handle& idh);
    TTaxId GetTaxIdOnce(const CSeq_id_Handle& idh);
    TSeqPos GetSequenceLength(const CSeq_id_Handle& idh);
    TSeqPos GetSequenceLengthOnce(const CSeq_id_Handle& idh);
    CDataLoader::SHashFound GetSequenceHash(const CSeq_id_Handle& idh);
    CDataLoader::SHashFound GetSequenceHashOnce(const CSeq_id_Handle& idh);
    CDataLoader::STypeFound GetSequenceType(const CSeq_id_Handle& idh);
    CDataLoader::STypeFound GetSequenceTypeOnce(const CSeq_id_Handle& idh);
    int GetSequenceState(CDataSource* data_source, const CSeq_id_Handle& idh);
    int GetSequenceStateOnce(CDataSource* data_source, const CSeq_id_Handle& idh);

    CDataLoader::TTSE_LockSet GetRecords(CDataSource* data_source,
                                         const CSeq_id_Handle& idh,
                                         CDataLoader::EChoice choice);
    CDataLoader::TTSE_LockSet GetRecordsOnce(CDataSource* data_source,
                                         const CSeq_id_Handle& idh,
                                         CDataLoader::EChoice choice);
    CRef<CPsgBlobId> GetBlobId(const CSeq_id_Handle& idh);
    CRef<CPsgBlobId> GetBlobIdOnce(const CSeq_id_Handle& idh);
    CTSE_Lock GetBlobById(CDataSource* data_source,
                              const CPsgBlobId& blob_id);
    CTSE_Lock GetBlobByIdOnce(CDataSource* data_source,
                              const CPsgBlobId& blob_id);
    void LoadChunk(CDataSource* data_source,
                   CTSE_Chunk_Info& chunk_info);
    void LoadChunks(CDataSource* data_source,
                    const CDataLoader::TChunkSet& chunks);
    void LoadChunksOnce(CDataSource* data_source,
                    const CDataLoader::TChunkSet& chunks);

    void GetBlobs(CDataSource* data_source, TTSE_LockSets& tse_sets);
    typedef set<CSeq_id_Handle> TLoadedSeqIds;
    void GetBlobsOnce(CDataSource* data_source, TLoadedSeqIds& loaded, TTSE_LockSets& tse_sets);

    typedef CDataLoader::TSeqIdSets TSeqIdSets;
    typedef CDataLoader::TCDD_Locks TCDD_Locks;
    void GetCDDAnnots(CDataSource* data_source,
        const TSeqIdSets& id_sets, TLoaded& loaded, TCDD_Locks& ret);
    void GetCDDAnnotsOnce(CDataSource* data_source,
        const TSeqIdSets& id_sets, TLoaded& loaded, TCDD_Locks& ret);

    CDataLoader::TTSE_LockSet GetAnnotRecordsNA(CDataSource* data_source,
                                                const TIds& ids,
                                                const SAnnotSelector* sel,
                                                CDataLoader::TProcessedNAs* processed_nas);
    CDataLoader::TTSE_LockSet GetAnnotRecordsNAOnce(CDataSource* data_source,
                                                    const TIds& ids,
                                                    const SAnnotSelector* sel,
                                                    CDataLoader::TProcessedNAs* processed_nas);

    void DropTSE(const CPsgBlobId& blob_id);

    void GetAccVers(const TIds& ids, TLoaded& loaded, TIds& ret);
    void GetAccVersOnce(const TIds& ids, TLoaded& loaded, TIds& ret);
    void GetGis(const TIds& ids, TLoaded& loaded, TGis& ret);
    void GetGisOnce(const TIds& ids, TLoaded& loaded, TGis& ret);
    void GetSequenceLengths(const TIds& ids, TLoaded& loaded, TSequenceLengths& ret);
    void GetSequenceLengthsOnce(const TIds& ids, TLoaded& loaded, TSequenceLengths& ret);
    void GetSequenceTypes(const TIds& ids, TLoaded& loaded, TSequenceTypes& ret);
    void GetSequenceTypesOnce(const TIds& ids, TLoaded& loaded, TSequenceTypes& ret);

    static CObjectIStream* GetBlobDataStream(const CPSG_BlobInfo& blob_info, const CPSG_BlobData& blob_data);

    typedef vector<shared_ptr<SPsgBioseqInfo>> TBioseqInfos;

    struct SReplyResult {
        CTSE_Lock lock;
        string blob_id;
        shared_ptr<SPsgBlobInfo> blob_info;
    };

    static void NCBI_XLOADER_GENBANK_EXPORT SetGetBlobByIdShouldFail(bool value);
    static bool NCBI_XLOADER_GENBANK_EXPORT GetGetBlobByIdShouldFail();

    template<class Call>
    typename std::result_of<Call()>::type
    CallWithRetry(Call&& call,
                  const char* name,
                  int retry_count = 0);
    
    void PrefetchCDD(const TIds& ids);

private:
    friend class CPSG_Blob_Task;

    shared_ptr<CPSG_Reply> x_SendRequest(shared_ptr<CPSG_Request> request);
    SReplyResult x_ProcessBlobReply(shared_ptr<CPSG_Reply> reply, CDataSource* data_source, CSeq_id_Handle req_idh, bool retry, bool lock_asap = false, CTSE_LoadLock* load_lock = nullptr);
    SReplyResult x_RetryBlobRequest(const string& blob_id, CDataSource* data_source, CSeq_id_Handle req_idh);
    string x_GetCachedBlobId(const CSeq_id_Handle& idh);
    shared_ptr<SPsgBioseqInfo> x_GetBioseqInfo(const CSeq_id_Handle& idh);
    shared_ptr<SPsgBlobInfo> x_GetBlobInfo(CDataSource* data_source, const string& blob_id);
    typedef pair<shared_ptr<SPsgBioseqInfo>, shared_ptr<SPsgBlobInfo>> TBioseqAndBlobInfo;
    TBioseqAndBlobInfo x_GetBioseqAndBlobInfo(CDataSource* data_source, const CSeq_id_Handle& idh);

    enum EMainChunkType {
        eNoDelayedMainChunk,
        eDelayedMainChunk
    };
    enum ESplitInfoType {
        eNoSplitInfo,
        eIsSplitInfo
    };
    // caller of x_ReadBlobData() should call SetLoaded();
    void x_ReadBlobData(
        const SPsgBlobInfo& psg_blob_info,
        const CPSG_BlobInfo& blob_info,
        const CPSG_BlobData& blob_data,
        CTSE_LoadLock& load_lock,
        ESplitInfoType split_info_type);
    void x_SetLoaded(CTSE_LoadLock& load_lock, EMainChunkType main_chunk_type);

    // returns pair(number of loaded infos, number of failed infos)
    pair<size_t, size_t> x_GetBulkBioseqInfo(CPSG_Request_Resolve::EIncludeInfo info,
        const TIds& ids,
        const TLoaded& loaded,
        TBioseqInfos& ret);

    shared_ptr<CPSG_Request_Blob>
    x_MakeLoadLocalCDDEntryRequest(CDataSource* data_source,
                                   CDataLoader::TChunk chunk);
    bool x_ReadCDDChunk(CDataSource* data_source,
                        CDataLoader::TChunk chunk,
                        const CPSG_BlobInfo& blob_info,
                        const CPSG_BlobData& blob_data);
    bool x_CheckAnnotCache(const string& name,
                           const TIds& ids,
                           CDataSource* data_source,
                           CDataLoader::TProcessedNAs* processed_nas,
                           CDataLoader::TTSE_LockSet& locks);

    CPSG_Request_Biodata::EIncludeData m_TSERequestMode = CPSG_Request_Biodata::eSmartTSE;
    CPSG_Request_Biodata::EIncludeData m_TSERequestModeBulk = CPSG_Request_Biodata::eWholeTSE;
    bool m_AddWGSMasterDescr = true;
    shared_ptr<CPSG_Queue> m_Queue;
    unique_ptr<CPSGBlobMap> m_BlobMap;
    unique_ptr<CPSGBioseqCache> m_BioseqCache;
    unique_ptr<CPSGAnnotCache> m_AnnotCache;
    unique_ptr<CPSGCDDInfoCache> m_CDDInfoCache;
    unique_ptr<CThreadPool> m_ThreadPool;
    CRef<CPSG_PrefetchCDD_Task> m_CDDPrefetchTask;
    int m_CacheLifespan;
    unsigned int m_RetryCount;
    unsigned int m_BulkRetryCount;
    CIncreasingTime m_WaitTime;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif // HAVE_PSG_LOADER

#endif  // OBJTOOLS_DATA_LOADERS_PSG___PSG_LOADER_IMPL__HPP
