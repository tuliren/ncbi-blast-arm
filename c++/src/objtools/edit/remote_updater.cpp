/*  $Id: remote_updater.cpp 664926 2023-03-23 19:51:44Z ivanov $
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
* Authors:  Sergiy Gotvyanskyy, NCBI
*           Colleen Bolin, NCBI
*
* File Description:
*   Front-end class for making remote request to MLA and taxon
*
* ===========================================================================
*/
#include <ncbi_pch.hpp>

#include <objects/taxon3/taxon3.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/seq/Pubdesc.hpp>

#include <objects/seqfeat/Org_ref.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objmgr/seq_descr_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>

// new
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <objtools/edit/remote_updater.hpp>
#include <objtools/edit/mla_updater.hpp>
#include <objtools/edit/eutils_updater.hpp>
#include <objtools/edit/edit_error.hpp>
#include <objtools/logging/listener.hpp>

#include <common/test_assert.h> /* This header must go last */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)


namespace
{
TEntrezId FindPMID(const list<CRef<CPub>>& arr)
{
    for (const auto& pPub : arr) {
        if (pPub->IsPmid()) {
            return pPub->GetPmid().Get();
        }
    }
    return ZERO_ENTREZ_ID;
}

static bool s_IsConnectionFailure(EError_val mlaErrorVal)
{
    switch (mlaErrorVal) {
    case eError_val_cannot_connect_pmdb:
    case eError_val_cannot_connect_searchbackend_pmdb:
        return true;
    default:
        break;
    }
    return false;
}

CRef<CPub> s_GetPubFrompmid(IPubmedUpdater* upd, TEntrezId id, int maxAttempts, IObjtoolsListener* pMessageListener)
{
    CRef<CPub> result;

    int maxCount = max(1, maxAttempts);
    for (int count = 0; count < maxCount; ++count) {
        EPubmedError errorVal;
        result = upd->GetPub(id, &errorVal);
        if (result) {
            return result;
        }

        bool isConnectionError = s_IsConnectionFailure(errorVal);
        if (isConnectionError && count < maxCount - 1) {
            continue;
        }

        std::ostringstream oss;
        oss << "Failed to retrieve publication for PMID "
            << id
            << ". ";
        if (isConnectionError) {
            oss << count + 1 << " attempts made. ";
        }
        oss << "CMLAClient : "
            << errorVal;
        string msg = oss.str();
        if (pMessageListener) {
            pMessageListener->PutMessage(CRemoteUpdaterMessage(msg, errorVal));
            break;
        } else {
            NCBI_THROW(CRemoteUpdaterException, eUnknown, msg);
        }
    }
    return result;
}

} // end anonymous namespace

class CCachedTaxon3_impl
{
public:
    typedef map<string, CRef<CT3Reply>> CCachedReplyMap;

    void Init()
    {
        if (! m_taxon) {
            m_taxon.reset(new CTaxon3(CTaxon3::initialize::yes));
            m_cache.reset(new CCachedReplyMap);
        }
    }

    void InitWithTimeout(unsigned seconds, unsigned retries, bool exponential)
    {
        if (! m_taxon) {
            const STimeout timeout = { seconds, 0 };
            m_taxon.reset(new CTaxon3(timeout, retries, exponential));
            m_cache.reset(new CCachedReplyMap);
        }
    }

    void ClearCache()
    {
        if (m_cache) {
            m_cache->clear();
        }
    }

    void ReportStats(std::ostream& str)
    {
        str << "CRemoteUpdater: cache_hits " << m_cache_hits << " out of " << m_num_requests << " requests\n";
    }

    CRef<COrg_ref> GetOrg(const COrg_ref& org, CRemoteUpdater::FLogger f_logger)
    {
        CRef<COrg_ref> result;
        CRef<CT3Reply> reply = GetOrgReply(org, f_logger);
        if (reply->IsData() && reply->SetData().IsSetOrg()) {
            result.Reset(&reply->SetData().SetOrg());
        }
        return result;
    }

    CRef<CT3Reply> GetOrgReply(const COrg_ref& in_org, CRemoteUpdater::FLogger f_logger)
    {
        m_num_requests++;
        std::ostringstream os;
        os << MSerial_AsnText << in_org;
        CRef<CT3Reply>& reply = (*m_cache)[os.str()];
        if (reply.Empty()) {
            CTaxon3_request request;

            CRef<CT3Request> rq(new CT3Request);
            CRef<COrg_ref>   org(new COrg_ref);
            org->Assign(in_org);
            rq->SetOrg(*org);

            request.SetRequest().push_back(rq);
            CRef<CTaxon3_reply> result = m_taxon->SendRequest(request);
            reply                      = result->SetReply().front();
            if (reply->IsError() && f_logger) {
                const string& error_message =
                    "Taxon update: " +
                    (in_org.IsSetTaxname() ? in_org.GetTaxname() : NStr::NumericToString(in_org.GetTaxId())) + ": " +
                    reply->GetError().GetMessage();

                f_logger(error_message);
            } else if (reply->IsData() && reply->SetData().IsSetOrg()) {
                reply->SetData().SetOrg().ResetSyn();
                // next will reset 'attrib = specified'
                // RW-1380 why do we need to reset attrib 'specified' ?
                //reply->SetData().SetOrg().SetOrgname().SetFormalNameFlag(false);
            }
        } else {
            m_cache_hits++;
#ifdef _DEBUG
            //cerr << "Using cache for:" << os.str() << endl;
#endif
        }
        return reply;
    }

    CRef<CTaxon3_reply> SendOrgRefList(const vector<CRef<COrg_ref>>& query, CRemoteUpdater::FLogger logger)
    {
        CRef<CTaxon3_reply> result(new CTaxon3_reply);

        for (const auto& it : query) {
            result->SetReply().push_back(GetOrgReply(*it, logger));
        }

        return result;
    }

protected:
    unique_ptr<CTaxon3>         m_taxon;
    unique_ptr<CCachedReplyMap> m_cache;
    size_t                      m_num_requests = 0;
    size_t                      m_cache_hits   = 0;
};

bool CRemoteUpdater::xUpdatePubPMID(list<CRef<CPub>>& arr, TEntrezId id)
{
    auto new_pub = s_GetPubFrompmid(m_pubmed.get(), id, m_MaxMlaAttempts, m_pMessageListener);
    if (! new_pub) {
        return false;
    }

    // authors come back in a weird format that we need
    // to convert to ISO
    if (new_pub->IsSetAuthors())
        CRemoteUpdater::ConvertToStandardAuthors(new_pub->SetAuthors());

    arr.clear();
    CRef<CPub> new_pmid(new CPub);
    new_pmid->SetPmid().Set(id);
    arr.push_back(new_pmid);
    arr.push_back(new_pub);
    return true;
}

void CRemoteUpdater::SetMaxMlaAttempts(int maxAttempts)
{
    m_MaxMlaAttempts = maxAttempts;
}

void CRemoteUpdater::SetTaxonTimeout(unsigned seconds, unsigned retries, bool exponential)
{
    m_TaxonTimeoutSet  = true;
    m_TaxonTimeout     = seconds;
    m_TaxonAttempts    = retries;
    m_TaxonExponential = exponential;
}

bool CRemoteUpdater::xSetFromConfig()
{
    // default update lambda function
    m_taxon_update = [this](const vector<CRef<COrg_ref>>& query) -> CRef<CTaxon3_reply>
    { // we need to make a copy of record to prevent changes put back to cache
        CConstRef<CTaxon3_reply> res = SendOrgRefList(query);
        CRef<CTaxon3_reply>      copied(new CTaxon3_reply);
        copied->Assign(*res);
        return copied;
    };

    CNcbiApplicationAPI* app = CNcbiApplicationAPI::Instance();
    if (app) {
        const CNcbiRegistry& cfg = app->GetConfig();

        if (cfg.HasEntry("RemotePubmedUpdate")) {
            const string sect = "RemotePubmedUpdate";
            string s = cfg.Get(sect, "Source");
            NStr::ToLower(s);
            if (s == "medarch") {
                m_pm_source = EPubmedSource::eMLA;
            } else if (s == "eutils") {
                m_pm_source = EPubmedSource::eEUtils;
            } else if (s == "none") {
                m_pm_source = EPubmedSource::eNone;
            }

            if (cfg.HasEntry(sect, "URL")) {
                m_pm_url = cfg.GetString(sect, "URL", {});
            }

            if (cfg.HasEntry(sect, "UseCache")) {
                m_pm_use_cache = cfg.GetBool(sect, "UseCache", true);
            }
        }

        if (cfg.HasEntry("RemoteTaxonomyUpdate")) {
            const string sect = "RemoteTaxonomyUpdate";
            int delay = cfg.GetInt(sect, "RetryDelay", 20);
            if (delay < 0)
                delay = 20;
            int count = cfg.GetInt(sect, "RetryCount", 5);
            if (count < 0)
                count = 5;
            bool exponential = cfg.GetBool(sect, "RetryExponentially", false);

            SetTaxonTimeout(static_cast<unsigned>(delay), static_cast<unsigned>(count), exponential);
            return true;
        }
    }

    return false;
}

void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, CSeqdesc& desc)
{
    if (desc.IsOrg()) {
        xUpdateOrgTaxname(desc.SetOrg(), logger);
    } else if (desc.IsSource() && desc.GetSource().IsSetOrg()) {
        xUpdateOrgTaxname(desc.SetSource().SetOrg(), logger);
    }
}

void CRemoteUpdater::xInitTaxCache()
{
    if (! m_taxClient) {
        m_taxClient.reset(new CCachedTaxon3_impl);
        if (m_TaxonTimeoutSet)
            m_taxClient->InitWithTimeout(m_TaxonTimeout, m_TaxonAttempts, m_TaxonExponential);
        else
            m_taxClient->Init();
    }
}

void CRemoteUpdater::xUpdateOrgTaxname(COrg_ref& org, FLogger logger)
{ // logger parameter is deprecated and should be removed soon
    std::lock_guard<std::mutex> guard(m_Mutex);

    TTaxId taxid = org.GetTaxId();
    if (taxid == ZERO_TAX_ID && ! org.IsSetTaxname())
        return;

    xInitTaxCache();

    CRef<COrg_ref> new_org = m_taxClient->GetOrg(org, logger);
    if (new_org.NotEmpty()) {
        org.Assign(*new_org);
    }
}

void CRemoteUpdater::UpdateOrgFromTaxon(CSeqdesc& desc)
{
    UpdateOrgFromTaxon(m_logger, desc);
}

CRemoteUpdater& CRemoteUpdater::GetInstance()
{
    static CRemoteUpdater instance{ (IObjtoolsListener*)nullptr };
    return instance;
}

CRemoteUpdater::CRemoteUpdater(FLogger logger, EPubmedSource pms, bool bNormalize) :
    m_logger{ logger }, m_pm_source(pms), m_pm_normalize(bNormalize)
{
    xSetFromConfig();
}

CRemoteUpdater::CRemoteUpdater(IObjtoolsListener* pMessageListener, EPubmedSource pms, bool bNormalize) :
    m_pMessageListener(pMessageListener), m_pm_source(pms), m_pm_normalize(bNormalize)
{
    if (m_pMessageListener) {
        m_logger = [this](const string& error_message) {
            m_pMessageListener->PutMessage(CObjEditMessage(error_message, eDiag_Warning));
        };
    }
    xSetFromConfig();
}

CRemoteUpdater::~CRemoteUpdater()
{
}

void CRemoteUpdater::ClearCache()
{
    std::lock_guard<std::mutex> guard(m_Mutex);

    if (m_taxClient) {
        m_taxClient->ClearCache();
    }

    if (m_pm_use_cache && m_pubmed) {
        switch (m_pm_source) {
        case EPubmedSource::eMLA: {
            auto* upd = dynamic_cast<CMLAUpdaterWithCache*>(m_pubmed.get());
            if (upd) {
                upd->ClearCache();
            }
            break;
        }
        case EPubmedSource::eEUtils: {
            auto* upd = dynamic_cast<CEUtilsUpdaterWithCache*>(m_pubmed.get());
            if (upd) {
                upd->ClearCache();
            }
            break;
        }
        default:
            break;
        }
    }
}

void CRemoteUpdater::UpdatePubReferences(CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI it(obj); it; ++it) {
        xUpdatePubReferences(it->GetEditHandle().SetDescr());
    }
}

void CRemoteUpdater::UpdatePubReferences(CSerialObject& obj)
{
    if (obj.GetThisTypeInfo()->IsType(CSeq_entry::GetTypeInfo())) {
        CSeq_entry* entry = static_cast<CSeq_entry*>(&obj);
        xUpdatePubReferences(*entry);
    } else if (obj.GetThisTypeInfo()->IsType(CSeq_submit::GetTypeInfo())) {
        CSeq_submit* submit = static_cast<CSeq_submit*>(&obj);
        for (auto& it : submit->SetData().SetEntrys()) {
            xUpdatePubReferences(*it);
        }
    } else if (obj.GetThisTypeInfo()->IsType(CSeq_descr::GetTypeInfo())) {
        CSeq_descr* desc = static_cast<CSeq_descr*>(&obj);
        xUpdatePubReferences(*desc);
    } else if (obj.GetThisTypeInfo()->IsType(CSeqdesc::GetTypeInfo())) {
        CSeqdesc*  desc = static_cast<CSeqdesc*>(&obj);
        CSeq_descr tmp;
        tmp.Set().push_back(CRef<CSeqdesc>(desc));
        xUpdatePubReferences(tmp);
    }
}

void CRemoteUpdater::xUpdatePubReferences(CSeq_entry& entry)
{
    if (entry.IsSet()) {
        for (auto& it : entry.SetSet().SetSeq_set()) {
            xUpdatePubReferences(*it);
        }
    }

    if (! entry.IsSetDescr())
        return;

    xUpdatePubReferences(entry.SetDescr());
}

void CRemoteUpdater::xUpdatePubReferences(CSeq_descr& seq_descr)
{
    std::lock_guard<std::mutex> guard(m_Mutex);

    for (auto& pDesc : seq_descr.Set()) {
        if (! pDesc->IsPub() || ! pDesc->GetPub().IsSetPub()) {
            continue;
        }

        auto& arr = pDesc->SetPub().SetPub().Set();
        if (! m_pubmed) {
            switch (m_pm_source) {
            case EPubmedSource::eNone:
                break;
            case EPubmedSource::eEUtils:
                if (m_pm_use_cache) {
                    m_pubmed.reset(new CEUtilsUpdaterWithCache(m_pm_normalize));
                } else {
                    m_pubmed.reset(new CEUtilsUpdater(m_pm_normalize));
                }
                if (! m_pm_url.empty()) {
                    CEUtils_Request::SetBaseURL(m_pm_url);
                }
                if (m_pm_interceptor)
                    m_pubmed->SetPubInterceptor(m_pm_interceptor);
                break;
            default:
            case EPubmedSource::eMLA:
                if (m_pm_use_cache) {
                    m_pubmed.reset(new CMLAUpdaterWithCache(m_pm_normalize));
                } else {
                    m_pubmed.reset(new CMLAUpdater(m_pm_normalize));
                }
                if (m_pm_interceptor)
                    m_pubmed->SetPubInterceptor(m_pm_interceptor);
                break;
            }
        }

        TEntrezId id = FindPMID(arr);
        if (id > ZERO_ENTREZ_ID) {
            xUpdatePubPMID(arr, id);
            continue;
        }

        for (const auto& pPubEquiv : arr) {
            if (pPubEquiv->IsArticle()) {
                id = m_pubmed->CitMatch(*pPubEquiv);
                if (id > ZERO_ENTREZ_ID && xUpdatePubPMID(arr, id)) {
                    break;
                }
            }
        }
    }
}


namespace
{
    typedef set<CRef<CSeqdesc>*> TOwnerSet;
    typedef struct {
        TOwnerSet      owner;
        CRef<COrg_ref> org_ref;
    } TOwner;
    typedef map<string, TOwner> TOrgMap;
    void                        _UpdateOrgFromTaxon(CSeq_entry& entry, TOrgMap& m)
    {
        if (entry.IsSet()) {
            for (auto& it : entry.SetSet().SetSeq_set()) {
                _UpdateOrgFromTaxon(*it, m);
            }
        }

        if (! entry.IsSetDescr())
            return;

        for (auto& it : entry.SetDescr().Set()) {
            CRef<CSeqdesc>& owner = it;
            CSeqdesc&       desc  = *owner;
            CRef<COrg_ref>  org_ref;
            if (desc.IsOrg()) {
                org_ref.Reset(&desc.SetOrg());
            } else if (desc.IsSource() && desc.GetSource().IsSetOrg()) {
                org_ref.Reset(&desc.SetSource().SetOrg());
            }
            if (org_ref) {
                string             id;
                std::ostringstream os;
                os << MSerial_AsnText << *org_ref;
                id        = os.str();
                TOwner& v = m[id];
                v.owner.insert(&owner);
                v.org_ref = org_ref;
            }
        }
    }

    void xUpdate(TOwnerSet& owners, COrg_ref& org_ref)
    {
        for (auto& owner_it : owners) {
            if ((*owner_it)->IsOrg()) {
                (*owner_it)->SetOrg(org_ref);
            } else if ((*owner_it)->IsSource()) {
                (*owner_it)->SetSource().SetOrg(org_ref);
            }
        }
    }
}

void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, CSeq_entry& entry)
{
    TOrgMap org_to_update;

    _UpdateOrgFromTaxon(entry, org_to_update);
    if (org_to_update.empty())
        return;

    std::lock_guard<std::mutex> guard(m_Mutex);

    if (! m_taxClient) {
        m_taxClient.reset(new CCachedTaxon3_impl);
        if (m_TaxonTimeoutSet)
            m_taxClient->InitWithTimeout(m_TaxonTimeout, m_TaxonAttempts, m_TaxonExponential);
        else
            m_taxClient->Init();
    }

    for (auto& it : org_to_update) {
        vector<CRef<COrg_ref>> reflist;
        reflist.push_back(it.second.org_ref);
        CRef<CTaxon3_reply> reply = m_taxClient->SendOrgRefList(reflist, logger);

        if (reply.NotNull()) {
            auto& reply_it = reply->SetReply().front();
            if (reply_it->IsData() && reply_it->SetData().IsSetOrg()) {
                xUpdate(it.second.owner, reply_it->SetData().SetOrg());
            }
        }
    }
}

void CRemoteUpdater::UpdateOrgFromTaxon(CSeq_entry& entry)
{
    UpdateOrgFromTaxon(m_logger, entry);
}

void CRemoteUpdater::UpdateOrgFromTaxon(FLogger logger, CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI bioseq_it(obj); bioseq_it; ++bioseq_it) {
        for (CSeqdesc_CI desc_it(bioseq_it->GetEditHandle()); desc_it; ++desc_it) {
            UpdateOrgFromTaxon(logger, (CSeqdesc&)*desc_it);
        }
    }
}


void CRemoteUpdater::ConvertToStandardAuthors(CAuth_list& auth_list)
{
    if (! auth_list.IsSetNames()) {
        return;
    }

    auth_list.ConvertMlToStd(false);

    if (auth_list.GetNames().IsStd()) {
        list<CRef<CAuthor>> authors_with_affil;
        for (CRef<CAuthor>& it : auth_list.SetNames().SetStd()) {
            if (it->IsSetAffil()) {
                authors_with_affil.push_back(it);
            }
        }

        if (authors_with_affil.size() == 1) {
            // we may need to hoist an affiliation
            if (auth_list.IsSetAffil()) {
                ERR_POST(Error << "publication contains multiple affiliations");
            } else {
                auth_list.SetAffil(authors_with_affil.front()->SetAffil());
                authors_with_affil.front()->ResetAffil();
            }
        }
    }
}


void CRemoteUpdater::PostProcessPubs(CSeq_entry& obj)
{
    if (obj.IsSet()) {
        for (CRef<CSeq_entry>& it : obj.SetSet().SetSeq_set()) {
            PostProcessPubs(*it);
        }
    } else if (obj.IsSeq() && obj.IsSetDescr()) {
        for (CRef<CSeqdesc>& desc_it : obj.SetSeq().SetDescr().Set()) {
            if (desc_it->IsPub()) {
                PostProcessPubs(desc_it->SetPub());
            }
        }
    }
}

void CRemoteUpdater::PostProcessPubs(CPubdesc& pubdesc)
{
    if (! pubdesc.IsSetPub())
        return;

    for (CRef<CPub>& it : pubdesc.SetPub().Set()) {
        if (it->IsSetAuthors()) {
            ConvertToStandardAuthors(it->SetAuthors());
        }
    }
}

void CRemoteUpdater::PostProcessPubs(CSeq_entry_EditHandle& obj)
{
    for (CBioseq_CI bioseq_it(obj); bioseq_it; ++bioseq_it) {
        for (CSeqdesc_CI desc_it(bioseq_it->GetEditHandle(), CSeqdesc::e_Pub); desc_it; ++desc_it) {
            PostProcessPubs(const_cast<CPubdesc&>(desc_it->GetPub()));
        }
    }
}

void CRemoteUpdater::SetPubmedClient(IPubmedUpdater* pubmedUpdater)
{
    m_pubmed.reset(pubmedUpdater);
}

void CRemoteUpdater::SetMLAClient(CMLAClient& mlaClient)
{
    CMLAUpdater* mlau = new CMLAUpdater(m_pm_normalize);
    mlau->SetClient(&mlaClient);
    m_pubmed.reset(mlau);
}

CConstRef<CTaxon3_reply> CRemoteUpdater::SendOrgRefList(const vector<CRef<COrg_ref>>& list)
{
    std::lock_guard<std::mutex> guard(m_Mutex);

    xInitTaxCache();

    CRef<CTaxon3_reply> reply = m_taxClient->SendOrgRefList(list, nullptr);
    return reply;
}

void CRemoteUpdater::ReportStats(std::ostream& os)
{
    std::lock_guard<std::mutex> guard(m_Mutex);

    if (m_taxClient) {
        m_taxClient->ReportStats(os);
    }

    if (m_pm_use_cache && m_pubmed) {
        switch (m_pm_source) {
        case EPubmedSource::eMLA: {
            auto* upd = dynamic_cast<CMLAUpdaterWithCache*>(m_pubmed.get());
            if (upd && ! dynamic_cast<CMLAUpdater*>(m_pubmed.get())) {
                upd->ReportStats(os);
            }
            break;
        }
        case EPubmedSource::eEUtils: {
            auto* upd = dynamic_cast<CEUtilsUpdaterWithCache*>(m_pubmed.get());
            if (upd && ! dynamic_cast<CEUtilsUpdater*>(m_pubmed.get())) {
                upd->ReportStats(os);
            }
            break;
        }
        default:
            break;
        }
    }
}

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE
