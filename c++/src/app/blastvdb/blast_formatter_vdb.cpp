/*  $Id: blast_formatter_vdb.cpp 640927 2021-11-21 02:52:23Z vakatov $
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
 * Author: Christiam Camacho
 *
 */

/** @file blast_formatter.cpp
 * Stand-alone command line formatter for BLAST.
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbistre.hpp>
#include <serial/iterator.hpp>
#include <algo/blast/api/version.hpp>
#include <algo/blast/api/remote_blast.hpp>
#include <algo/blast/blastinput/blast_input_aux.hpp>
#include <algo/blast/format/blast_format.hpp>
#include <algo/blast/api/objmgr_query_data.hpp>
#include "../blast/blast_app_util.hpp"
#include <algo/blast/vdb/vdb2blast_util.hpp>
#include <algo/blast/vdb/vdbalias.hpp>
#include "blast_vdb_app_util.hpp"

USING_NCBI_SCOPE;
USING_SCOPE(blast);

/// The application class
class CBlastFormatterVdbApp : public CNcbiApplication
{
public:
    /** @inheritDoc */
    CBlastFormatterVdbApp() {
        CRef<CVersion> version(new CVersion());
        version->SetVersionInfo(new CBlastVersion());
        SetFullVersion(version);
        m_LoadFromArchive = false;
        m_StopWatch.Start();
        if (m_UsageReport.IsEnabled()) {
            m_UsageReport.AddParam(CBlastUsageReport::eVersion, GetVersion().Print());
            m_UsageReport.AddParam(CBlastUsageReport::eProgram, (string) "blast_formatter_vdb");
        }
    }
    ~CBlastFormatterVdbApp();

private:
    /** @inheritDoc */
    virtual void Init();
    /** @inheritDoc */
    virtual int Run();
    
    /// Prints the BLAST formatted output
    int PrintFormattedOutput(void);

    /// Extracts the queries to be formatted
    /// @param query_is_protein Are the queries protein sequences? [in]
    CRef<CBlastQueryVector> x_ExtractQueries(bool query_is_protein);

    /// Build the query from a PSSM
    /// @param pssm PSSM to inspect [in]
    CRef<CBlastSearchQuery> 
    x_BuildQueryFromPssm(const CPssmWithParameters& pssm);

    /// Package a scope and Seq-loc into a SSeqLoc from a Bioseq
    /// @param bioseq Bioseq to inspect [in]
    /// @param scope Scope object to add the sequence data to [in|out]
    SSeqLoc x_QueryBioseqToSSeqLoc(const CBioseq& bioseq, CRef<CScope> scope);

    void x_AddCmdOptions();

    /// Our link to the NCBI BLAST service
    CRef<CRemoteBlast> m_RmtBlast;

    /// The source of CScope objects for queries
    CRef<CBlastScopeSource> m_QueryScopeSource;

    /// Tracks whether results come from an archive file.
    bool m_LoadFromArchive;
    CBlastUsageReport m_UsageReport;
    CStopWatch m_StopWatch;
};

CBlastFormatterVdbApp::~CBlastFormatterVdbApp() {
   	m_UsageReport.AddParam(CBlastUsageReport::eRunTime, m_StopWatch.Elapsed());
}

void CBlastFormatterVdbApp::Init()
{
    HideStdArgs(fHideLogfile | fHideConffile | fHideFullVersion | fHideXmlHelp | fHideDryRun);

    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(), 
                  "Stand-alone BLAST formatter client, version " 
                  + CBlastVersion().Print());

    arg_desc->SetCurrentGroup("Input options");
    arg_desc->AddOptionalKey(kArgRid, "BLAST_RID", "BLAST Request ID (RID)", 
                     CArgDescriptions::eString);

    // add input file for seq-align here?
    arg_desc->AddOptionalKey(kArgArchive, "ArchiveFile", "File containing BLAST Archive format in ASN.1 (i.e.: output format 11)", 
                     CArgDescriptions::eInputFile);
    arg_desc->SetDependency(kArgRid, CArgDescriptions::eExcludes, kArgArchive);

    CFormattingArgs fmt_args(false, CFormattingArgs::eIsVDB_SAM);
    fmt_args.SetArgumentDescriptions(*arg_desc);

    arg_desc->SetCurrentGroup("Output configuration options");
    arg_desc->AddDefaultKey(kArgOutput, "output_file", "Output file name", 
                            CArgDescriptions::eOutputFile, "-");

    arg_desc->SetCurrentGroup("Miscellaneous options");
    arg_desc->AddFlag(kArgParseDeflines,
                 "Should the query and subject defline(s) be parsed?", true);
    arg_desc->SetCurrentGroup("");

    CDebugArgs debug_args;
    debug_args.SetArgumentDescriptions(*arg_desc);

    SetupArgDescriptions(arg_desc.release());
}

SSeqLoc
CBlastFormatterVdbApp::x_QueryBioseqToSSeqLoc(const CBioseq& bioseq,
                                           CRef<CScope> scope)
{
    static bool first_time = true;
    _ASSERT(scope);

    if ( !HasRawSequenceData(bioseq) && first_time ) {
        _ASSERT(m_QueryScopeSource);
        m_QueryScopeSource->AddDataLoaders(scope);
        first_time = false;
    }
    else {
        scope->AddBioseq(bioseq);
    }
    CRef<CSeq_loc> seqloc(new CSeq_loc);
    seqloc->SetWhole().Assign(*bioseq.GetFirstId());
    return SSeqLoc(seqloc, scope);
}

CRef<CBlastSearchQuery>
CBlastFormatterVdbApp::x_BuildQueryFromPssm(const CPssmWithParameters& pssm)
{
    if ( !pssm.HasQuery() ) {
        throw runtime_error("PSSM has no query");
    }
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    const CSeq_entry& seq_entry = pssm.GetQuery();
    if ( !seq_entry.IsSeq() ) {
        throw runtime_error("Cannot have multiple queries in a PSSM");
    }
    SSeqLoc ssl = x_QueryBioseqToSSeqLoc(seq_entry.GetSeq(), scope);
    CRef<CBlastSearchQuery> retval;
    retval.Reset(new CBlastSearchQuery(*ssl.seqloc, *ssl.scope));
    _ASSERT(ssl.scope.GetPointer() == scope.GetPointer());
    return retval;
}

CRef<CBlastQueryVector>
CBlastFormatterVdbApp::x_ExtractQueries(bool query_is_protein)
{
    CRef<CBlast4_queries> b4_queries = m_RmtBlast->GetQueries();
    _ASSERT(b4_queries);
    const size_t kNumQueries = b4_queries->GetNumQueries();

    CRef<CBlastQueryVector> retval(new CBlastQueryVector);

    SDataLoaderConfig dlconfig(query_is_protein);
    dlconfig.OptimizeForWholeLargeSequenceRetrieval(false);
    m_QueryScopeSource.Reset(new CBlastScopeSource(dlconfig));

    if (b4_queries->IsPssm()) {
        retval->AddQuery(x_BuildQueryFromPssm(b4_queries->GetPssm()));
    } else if (b4_queries->IsSeq_loc_list()) {
        CRef<CScope> scope = m_QueryScopeSource->NewScope();
        ITERATE(CBlast4_queries::TSeq_loc_list, seqloc,
                b4_queries->GetSeq_loc_list()) {
            _ASSERT( !(*seqloc)->GetId()->IsLocal() );
            CRef<CBlastSearchQuery> query(new CBlastSearchQuery(**seqloc,
                                                                *scope));
            retval->AddQuery(query);
        }
    } else if (b4_queries->IsBioseq_set()) {
        CTypeConstIterator<CBioseq> itr(ConstBegin(b4_queries->GetBioseq_set(),
                                                   eDetectLoops));
        CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
        for (; itr; ++itr) {
            SSeqLoc ssl = x_QueryBioseqToSSeqLoc(*itr, scope);
            CRef<CBlastSearchQuery> query(new CBlastSearchQuery(*ssl.seqloc,
                                                                *ssl.scope));
            retval->AddQuery(query);
        }
    }

    (void)kNumQueries;  // eliminate compiler warning;
    _ASSERT(kNumQueries == retval->size());
    return retval;
}

static void s_FillDBInfo(CBlastFormatUtil::SDbInfo & dbInfo, const string & dbNames, bool isProtein)
{
	Uint8 seqs, length;
	CVDBBlastUtil::GetVDBStats(dbNames, seqs, length);

	dbInfo.is_protein = false;
	dbInfo.name =dbNames;
	dbInfo.definition = dbInfo.name;
	dbInfo.total_length = (Int8)length;
	dbInfo.number_seqs = (int) seqs;
}

int CBlastFormatterVdbApp::PrintFormattedOutput(void)
{
    int retval = 0;
    const CArgs& args = GetArgs();
    CNcbiOstream& out = args[kArgOutput].AsOutputFile();
    CFormattingArgs fmt_args(false, CFormattingArgs::eIsVDB_SAM);

    CRef<CBlastOptionsHandle> opts_handle = m_RmtBlast->GetSearchOptions();
    CBlastOptions& opts = opts_handle->SetOptions();
    fmt_args.ExtractAlgorithmOptions(args, opts);
    {{
        CDebugArgs debug_args;
        debug_args.ExtractAlgorithmOptions(args, opts);
        if (debug_args.ProduceDebugOutput()) {
            opts.DebugDumpText(NcbiCerr, "BLAST options", 1);
        }
    }}

	string dbs = m_RmtBlast->GetDatabases()->GetName();
	NStr::ReplaceInPlace(dbs, "WGS_VDB://", kEmptyStr);
	vector<string> paths;
	CVDBAliasUtil::FindVDBPaths(dbs, false, paths, NULL, NULL, true, true);
	string dbAllNames = NStr::Join(paths, " ");

    // Initialize the object manager and the scope object
	CRef<CScope> scope = GetVDBScope(dbAllNames);

	const EBlastProgramType p = opts.GetProgramType();
	if((fmt_args.GetFormattedOutputChoice() == CFormattingArgs::eSAM) &&
	    (p != eBlastTypeBlastn )) {
	               NCBI_THROW(CInputException, eInvalidInput,
	                                       "SAM format is only applicable to blastn results" );
	}

	CRef<CBlastQueryVector> queries =  x_ExtractQueries(Blast_QueryIsProtein(p)?true:false);
    _ASSERT(queries);
    scope->AddScope(*(queries->GetScope(0)));

    // Create the DBInfo entries for dbs being searched
    vector< CBlastFormatUtil::SDbInfo > vecDbInfo(1);
    s_FillDBInfo(vecDbInfo[0], dbAllNames, false);

    // Get the formatting options and initialize the formatter
    CBlastFormat formatter(opts, vecDbInfo,
                           fmt_args.GetFormattedOutputChoice(),
                           static_cast<bool>(args[kArgParseDeflines]),
                           out,
                           fmt_args.GetNumDescriptions(),
                           fmt_args.GetNumAlignments(),
                           *scope,
                           fmt_args.ShowGis(),
                           fmt_args.DisplayHtmlOutput(),
                           false,
                           fmt_args.GetCustomOutputFormatSpec(),
                           true,
                           GetCmdlineArgs(GetArguments()));

    formatter.SetLineLength(fmt_args.GetLineLength());
    if( UseXInclude(fmt_args, args[kArgOutput].AsString()) ) {
       	formatter.SetBaseFile(args[kArgOutput].AsString());
    }
    CRef<CSearchResultSet> results = m_RmtBlast->GetResultSet();
    formatter.PrintProlog();
    bool isPsiBlast = ("psiblast" == kTask);
    if (fmt_args.ArchiveFormatRequested(args))
    {
    		CRef<IQueryFactory> query_factory(new CObjMgr_QueryFactory(*queries));
    		formatter.WriteArchive(*query_factory, *opts_handle, *results);
    } else {
        while (1)
        {
        	//BlastFormatter_PreFetchSequenceData(*results, scope, fmt_args.GetFormattedOutputChoice());
    		ITERATE(CSearchResultSet, result, *results) {
    			if(isPsiBlast)
    			{
    				formatter.PrintOneResultSet(**result, queries, 
					m_RmtBlast->GetPsiNumberOfIterations());
    			}
    			else
    			{
    				formatter.PrintOneResultSet(**result, queries);
    			}
    		}
		// The entire archive file (multiple sets) is formatted in this loop for XML.
		// That does not work for other formats.  Ugly, but that's where it's at now.
		if (m_LoadFromArchive == false || (fmt_args.GetFormattedOutputChoice() != CFormattingArgs::eXml
                        && fmt_args.GetFormattedOutputChoice() != CFormattingArgs::eXml2 
                        && fmt_args.GetFormattedOutputChoice() != CFormattingArgs::eJson
                        && fmt_args.GetFormattedOutputChoice() != CFormattingArgs::eXml2_S
                        && fmt_args.GetFormattedOutputChoice() != CFormattingArgs::eJson_S )
			|| !m_RmtBlast->LoadFromArchive()) {
			break;
                }
		// Reset these for next set from archive
    		results.Reset(m_RmtBlast->GetResultSet());
    		queries.Reset(x_ExtractQueries(Blast_QueryIsProtein(p)?true:false));
    		_ASSERT(queries);
                if (fmt_args.GetFormattedOutputChoice() == CFormattingArgs::eXml) {
    		    scope.Reset(queries->GetScope(0));
                }
                else {
    		    scope->AddScope(*(queries->GetScope(0)));
                }
    	// Add subjects
	}
    }
    formatter.PrintEpilog(opts);
    return retval;
}

#define EXIT_CODE__UNKNOWN_RID 1
#define EXIT_CODE__SEARCH_PENDING 2
#define EXIT_CODE__SEARCH_FAILED 3

int CBlastFormatterVdbApp::Run(void)
{
    int status = 0;
    const CArgs& args = GetArgs();

    try {
        if (args[kArgArchive].HasValue()) {
            CNcbiIstream& istr = args[kArgArchive].AsInputFile();
            try { m_RmtBlast.Reset(new CRemoteBlast(istr)); }
            catch (const CBlastException& e) {
                if (e.GetErrCode() == CBlastException::eInvalidArgument) {
                    NCBI_RETHROW(e, CInputException, eInvalidInput,
                                 "Invalid input format for BLAST Archive.");
                }
            }

	    m_LoadFromArchive = true;
            try {
                while (m_RmtBlast->LoadFromArchive())
                    status = PrintFormattedOutput();
            } catch (const CSerialException& e) {
                NCBI_RETHROW(e, CInputException, eInvalidInput,
                             "Invalid input format for BLAST Archive.");
            }
            x_AddCmdOptions();
            m_UsageReport.AddParam(CBlastUsageReport::eExitStatus, status);
    	    return status;
        }

        const string kRid = args[kArgRid].AsString();
        m_RmtBlast.Reset(new CRemoteBlast(kRid));
        {{
            CDebugArgs debug_args;
            CBlastOptions dummy_options;
            debug_args.ExtractAlgorithmOptions(args, dummy_options);
            if (debug_args.ProduceDebugRemoteOutput()) {
                m_RmtBlast->SetVerbose();
            }
        }}

        switch (m_RmtBlast->CheckStatus()) {
        case CRemoteBlast::eStatus_Unknown:
            cerr << "Unknown/invalid RID '" << kRid << "'." << endl;
            status = EXIT_CODE__UNKNOWN_RID;
            break;

        case CRemoteBlast::eStatus_Done:
            status = PrintFormattedOutput();
            break;

        case CRemoteBlast::eStatus_Pending:
            cerr << "RID '" << kRid << "' is still pending." << endl;
            status = EXIT_CODE__SEARCH_PENDING;
            break;

        case CRemoteBlast::eStatus_Failed:
            cerr << "RID '" << kRid << "' has failed" << endl;
            cerr << m_RmtBlast->GetErrors() << endl;
            status = EXIT_CODE__SEARCH_FAILED;
            break;
           
        default:
            abort();
        }

    } CATCH_ALL(status)
    x_AddCmdOptions();
    m_UsageReport.AddParam(CBlastUsageReport::eExitStatus, status);
    return status;
}

void CBlastFormatterVdbApp::x_AddCmdOptions()
{
    const CArgs & args = GetArgs();
    if (args[kArgRid].HasValue()) {
         m_UsageReport.AddParam(CBlastUsageReport::eRIDInput, args[kArgRid].AsString());
    }
    else if (args[kArgArchive].HasValue()) {
         m_UsageReport.AddParam(CBlastUsageReport::eArchiveInput, true);
    }

    if(args["outfmt"].HasValue()) {
        m_UsageReport.AddParam(CBlastUsageReport::eOutputFmt, args["outfmt"].AsString());
    }
}

#ifndef SKIP_DOXYGEN_PROCESSING
int main(int argc, const char* argv[] /*, const char* envp[]*/)
{
    return CBlastFormatterVdbApp().AppMain(argc, argv, 0, eDS_Default, "");
}
#endif /* SKIP_DOXYGEN_PROCESSING */
