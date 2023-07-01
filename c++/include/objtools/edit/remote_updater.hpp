/*  $Id: remote_updater.hpp 664926 2023-03-23 19:51:44Z ivanov $
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

#ifndef __REMOTE_UPDATER_HPP_INCLUDED__
#define __REMOTE_UPDATER_HPP_INCLUDED__

#include <corelib/ncbimisc.hpp>
#include <functional>
#include <objtools/edit/edit_error.hpp>
#include <objtools/edit/pubmed_updater.hpp>
#include <objects/taxon3/itaxon3.hpp>
#include <mutex>

BEGIN_NCBI_SCOPE

class CSerialObject;

BEGIN_SCOPE(objects)

class CSeq_entry_EditHandle;
class CSeq_entry;
class CSeqdesc;
class CSeq_descr;
class COrg_ref;
class CMLAClient;
class CAuth_list;
class IObjtoolsListener;
class CPub;
class CPubdesc;
class CTaxon3_reply;

BEGIN_SCOPE(edit)

enum class EPubmedSource
{
    eNone,
    eEUtils,
    eMLA,
};

class CCachedTaxon3_impl;

class NCBI_XOBJEDIT_EXPORT CRemoteUpdaterMessage: public CObjEditMessage
{
public:
    CRemoteUpdaterMessage(const std::string& msg, EError_val error)
    : CObjEditMessage(msg, eDiag_Warning), m_error(error)
    {
    }
    virtual CRemoteUpdaterMessage *Clone(void) const {
        return new CRemoteUpdaterMessage(GetText(), m_error);
    }

    EError_val m_error;
};

class NCBI_XOBJEDIT_EXPORT CRemoteUpdaterException: public CException
{
public:
    using CException::CException;
};

class NCBI_XOBJEDIT_EXPORT CRemoteUpdater
{
public:

    using FLogger = function<void(const string&)>;

    // With this constructor, failure to retrieve
    // a publication for a PMID is logged with the supplied message listener.
    // If no message listener is supplied, an exception is thrown.
    CRemoteUpdater(IObjtoolsListener* pMessageListener, EPubmedSource = EPubmedSource::eMLA, bool bNormalize = false);
    CRemoteUpdater(FLogger logger, EPubmedSource = EPubmedSource::eMLA, bool bNormalize = false);
    ~CRemoteUpdater();

    void UpdatePubReferences(CSerialObject& obj);
    void UpdatePubReferences(CSeq_entry_EditHandle& obj);
    void SetMaxMlaAttempts(int max);
    CConstRef<CTaxon3_reply> SendOrgRefList(const vector<CRef<COrg_ref>>& list);

    // These methods are deprecated, please use CRemoteUpdater constructor to specify logger
    NCBI_DEPRECATED void UpdateOrgFromTaxon(FLogger f_logger, CSeq_entry& entry);
    NCBI_DEPRECATED void UpdateOrgFromTaxon(FLogger f_logger, CSeq_entry_EditHandle& obj);
    NCBI_DEPRECATED void UpdateOrgFromTaxon(FLogger f_logger, CSeqdesc& obj);

    void UpdateOrgFromTaxon(CSeq_entry& entry);
    void UpdateOrgFromTaxon(CSeqdesc& desc);
    void SetTaxonTimeout(unsigned seconds, unsigned retries, bool exponential);

    void ClearCache();
    static void ConvertToStandardAuthors(CAuth_list& auth_list);
    static void PostProcessPubs(CSeq_entry_EditHandle& obj);
    static void PostProcessPubs(CSeq_entry& obj);
    static void PostProcessPubs(CPubdesc& pubdesc);

    void SetPubmedClient(IPubmedUpdater*);
    void SetMLAClient(CMLAClient& mlaClient);
    // Use either shared singleton or individual instances
    NCBI_DEPRECATED static CRemoteUpdater& GetInstance();
    void ReportStats(std::ostream& str);
    taxupdate_func_t GetUpdateFunc() const { return m_taxon_update; }
    TPubInterceptor SetPubmedInterceptor(TPubInterceptor f)
    {
        TPubInterceptor old = m_pm_interceptor;
        m_pm_interceptor = f;
        return old;
    }

private:
    void xUpdatePubReferences(CSeq_entry& entry);
    void xUpdatePubReferences(CSeq_descr& descr);
    void xUpdateOrgTaxname(COrg_ref& org, FLogger logger);
    bool xUpdatePubPMID(list<CRef<CPub>>& pubs, TEntrezId id);
    bool xSetFromConfig();
    void xInitTaxCache();

    IObjtoolsListener* m_pMessageListener = nullptr;
    FLogger m_logger = nullptr; // wrapper for compatibility between IObjtoolsListener and old FLogger

    EPubmedSource              m_pm_source = EPubmedSource::eNone;
    string                     m_pm_url;
    unique_ptr<IPubmedUpdater> m_pubmed;
    bool                       m_pm_use_cache = true;
    bool                       m_pm_normalize = false;
    TPubInterceptor            m_pm_interceptor = nullptr;

    unique_ptr<CCachedTaxon3_impl> m_taxClient;
    taxupdate_func_t m_taxon_update;

    std::mutex m_Mutex;
    int m_MaxMlaAttempts = 3;

    bool m_TaxonTimeoutSet = false;
    unsigned m_TaxonTimeout = 20;   // in seconds
    unsigned m_TaxonAttempts = 5;
    bool m_TaxonExponential = false;
};

END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif
