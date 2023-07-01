/*  $Id: blast_vdb_cmd.cpp 640928 2021-11-21 02:52:31Z vakatov $
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
 * Author: Amelia Fong
 *
** @file blast_vdb_cmd.cpp
 * Command line tool to get vdb info.
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <objmgr/util/sequence.hpp>
#include <algo/blast/vdb/vdb2blast_util.hpp>
#include <algo/blast/api/blast_usage_report.hpp>
#include <algo/blast/blastinput/blast_input_aux.hpp>
#include <algo/blast/blastinput/blast_input.hpp>
#include <algo/blast/vdb/vdbalias.hpp>

USING_NCBI_SCOPE;
USING_SCOPE(blast);

static const NStr::TNumToStringFlags kFlags = NStr::fWithCommas;

/// The application class
class CBlastVdbCmdApp : public CNcbiApplication
{
public:
    /** @inheritDoc */
    CBlastVdbCmdApp();
    ~CBlastVdbCmdApp() {
    	m_UsageReport.AddParam(CBlastUsageReport::eRunTime, m_StopWatch.Elapsed());
    }
private:
    /** @inheritDoc */
    virtual void Init();
    /** @inheritDoc */
    virtual int Run();
    
    /// Initializes the application's data members
    void x_InitApplicationData();

    /// Get vdb util
    CRef<CVDBBlastUtil> x_GetVDBBlastUtil(bool isCSRA);

    /// Prints the BLAST database information (e.g.: handles -info command line
    /// option)
    int x_PrintBlastDatabaseInformation();

    /// Processes all requests except printing the BLAST database information
    /// @return 0 on success; 1 if some sequences were not retrieved
    int x_ProcessSearchRequest();

    /// Print vdb paths
    int x_PrintVDBPaths(bool recursive);

    /// Resolve vdb paths
    void x_GetFullPaths();

    /// Retrieve the queries from the command line arguments
    vector<string> x_GetQueries();

    string x_FormatRuntime(const CStopWatch& sw) const;

    void x_AddCmdOptions();

    // Store all db names
    string m_allDbs;
    string m_origDbs;
    bool m_isRef;
    CBlastUsageReport m_UsageReport;
    CStopWatch m_StopWatch;
};


CBlastVdbCmdApp::CBlastVdbCmdApp(): m_allDbs(kEmptyStr), m_origDbs(kEmptyStr), m_isRef(false) {
        CRef<CVersion> version(new CVersion());
        version->SetVersionInfo(new CBlastVersion());
        SetFullVersion(version);
        m_StopWatch.Start();
        if (m_UsageReport.IsEnabled()) {
        	m_UsageReport.AddParam(CBlastUsageReport::eVersion, GetVersion().Print());
        	m_UsageReport.AddParam(CBlastUsageReport::eProgram, (string) "blast_vdb_cmd");
        }
    }

/** Class to extract FASTA (as returned by the blast_sra library) from SRA
 * data.
 *
 * Inspired by the CSeqFormatter class 
 */
class CVdbFastaExtractor {
public:
    CVdbFastaExtractor(CRef<CVDBBlastUtil> sraobj, CNcbiOstream& out, const string & fmt_spec,
		       	   	   TSeqPos line_width = 80);

    void Write(CRef<CSeq_id> seqid);
    void Write(CRef<CBioseq> bioseq, int oid);
    void DumpAll();

private:
    CRef<CVDBBlastUtil> m_VdbBlastDB;
    CNcbiOstream& m_Out;
    const string m_FmtSpec;
    TSeqPos m_LineWidth;
    /// Vector of offsets where the replacements will take place
    vector<string> m_Seperators;
    /// Vector of convertor objects
    vector<char> m_ReplTypes;
    bool m_FastaOnly;
    bool m_LoadSeq;

};

CVdbFastaExtractor::CVdbFastaExtractor(CRef<CVDBBlastUtil> sraobj, CNcbiOstream& out, const string & fmt_spec, TSeqPos line_width)
     : m_VdbBlastDB(sraobj), m_Out(out), m_FmtSpec(fmt_spec), m_LineWidth(line_width), m_FastaOnly(false), m_LoadSeq(false)
{
    string sp = kEmptyStr;
    for (SIZE_TYPE i = 0; i < m_FmtSpec.size(); i++) {
        if (m_FmtSpec[i] == '%') {
            if ( m_FmtSpec[i+1] == '%') {
                // remove the escape character for '%'
                i++;
                sp += m_FmtSpec[i];
                continue;
            }
            i++;
            m_ReplTypes.push_back(m_FmtSpec[i]);
            m_Seperators.push_back(sp);
            sp = kEmptyStr;
        }
        else {
            sp += m_FmtSpec[i];
        }
    }
    m_Seperators.push_back(sp);

    if (m_ReplTypes.empty() || (m_ReplTypes.size() + 1 != m_Seperators.size())) {
        NCBI_THROW(CBlastException, eInvalidOptions,
                   "Invalid format specification");
    }

   	for (unsigned int i=0; i < m_ReplTypes.size(); i++) {
   		if(m_ReplTypes[i] == 'f') {
   			m_FastaOnly = true;
   			m_LoadSeq = true;
   			break;
   		}
   		else if(m_ReplTypes[i] == 's') {
    			m_LoadSeq = true;
    	}
    }
}

void CVdbFastaExtractor::Write(CRef<CSeq_id> seqid)
{
    CRef<CBioseq> bioseq = m_VdbBlastDB->CreateBioseqFromVDBSeqId(seqid);
    if (bioseq.Empty()) {
        ERR_POST("Failed to find Bioseq for '" + seqid->AsFastaString() + "'");
        return;
    }

    if (m_FastaOnly) {
    	CFastaOstream fasta(m_Out);
    	fasta.SetWidth(m_LineWidth);
    	fasta.SetAllFlags(CFastaOstream::fKeepGTSigns|CFastaOstream::fNoExpensiveOps);
    	fasta.Write(*bioseq);
    }
    else {
    	int oid = -1;
    	Write(bioseq, oid);
    }
}

void CVdbFastaExtractor::Write(CRef<CBioseq> bioseq, int oid)
{
	if ((!bioseq->IsSetInst()) || (!bioseq->GetInst().IsSetSeq_data())) {
        ERR_POST("Bioseq constains no sequence data");
        return;
	}
	const CSeq_inst & si = bioseq->GetInst();

    for(unsigned int i =0; i < m_ReplTypes.size(); i++) {
	    m_Out << m_Seperators[i];
	    switch (m_ReplTypes[i]) {
	        case 's':
	        {
	        	const CSeq_data & d = si.GetSeq_data();
	        	string sa = "N/A";
	        	if (d.IsIupacna()) {
	        		sa = d.GetIupacna().Get();
	        	}
	            m_Out << sa;
	            break;
	        }
	        case 'a':
	        {
	        	const CSeq_id * id = bioseq->GetFirstId();
	            m_Out << id->GetSeqIdString(true);
	            break;
	        }
	        case 'i':
	        {
	        	const CSeq_id * id = bioseq->GetFirstId();
	            m_Out << id->AsFastaString() ;
	            break;
	        }
	        case 'o':
	        {
	        	if (oid == -1) {
	        		CRef<CSeq_id> cid(const_cast<CSeq_id *> (bioseq->GetFirstId()));
	        		oid = (int) m_VdbBlastDB->GetOIDFromVDBSeqId(cid);
	        	}
	            m_Out << NStr::NumericToString(oid);
	            break;
	        }
	        case 't':
	        {
	        	string t = "N/A";
	        	if(bioseq->IsSetDescr() && bioseq->GetDescr().IsSet()) {
	        		CRef<CSeqdesc> descTitle = bioseq->GetDescr().Get().front();
	        		t = descTitle->GetTitle();
	        	}
        		m_Out << t;
	            break;
	        }
	        case 'l':
	        {
	        	string l = "N/A";
	        	if(si.IsSetLength()){
	        		l = NStr::NumericToString(si.GetLength());
	        	}
	            m_Out << l;
	            break;
	        }
	        default:
	            CNcbiOstrstream os;
	            os << "Unrecognized format specification: '%" << m_ReplTypes[i] << "'";
	            NCBI_THROW(CInputException, eInvalidInput, CNcbiOstrstreamToString(os));
	        }
	    }
	    m_Out << m_Seperators.back();
	    m_Out << endl;
}

 void CVdbFastaExtractor::DumpAll() {
     BlastSeqSrc* seqsrc = m_VdbBlastDB->GetSRASeqSrc();
     BlastSeqSrcGetSeqArg seq_arg = { '\0' };
     BlastSeqSrcIterator * itr = BlastSeqSrcIteratorNewEx(1);
	 if (m_FastaOnly) {
         CFastaOstream fasta(m_Out);
         fasta.SetWidth(m_LineWidth);
         fasta.SetAllFlags(CFastaOstream::fKeepGTSigns|CFastaOstream::fNoExpensiveOps);
	     while ((seq_arg.oid = BlastSeqSrcIteratorNext(seqsrc, itr)) != BLAST_SEQSRC_EOF) {
	    	 if (seq_arg.oid == BLAST_SEQSRC_ERROR) {
       			ERR_POST("Iterator returns BLAST_SEQSRC_ERROR");
        		return;
             }
	    	 CRef<CBioseq> bioseq = m_VdbBlastDB->CreateBioseqFromOid(seq_arg.oid);
             if (bioseq.Empty()) {
     	  		ERR_POST("Empty Bioseq");
     	    	return;
             }
             fasta.Write(*bioseq);
         }
	 }
	 else {
	     while ((seq_arg.oid = BlastSeqSrcIteratorNext(seqsrc, itr)) != BLAST_SEQSRC_EOF) {
	    	 if (seq_arg.oid == BLAST_SEQSRC_ERROR) {
       			ERR_POST("Iterator returns BLAST_SEQSRC_ERROR");
        		return;
             }
	    	 CRef<CBioseq> bioseq = m_VdbBlastDB->CreateBioseqFromOid(seq_arg.oid);
             if (bioseq.Empty()) {
     	  		ERR_POST("Empty Bioseq");
     	  		continue;
             }
             Write(bioseq, seq_arg.oid);
         }

	 }
 }

string s_GetCSRADBs(const string & db_list, string & not_csra_list) {
	vector<string> dbs;
	string csra_list = kEmptyStr;
	not_csra_list = kEmptyStr;
	NStr::Split(db_list, " ", dbs);
	for(unsigned int i=0; i < dbs.size(); i++) {
		if(CVDBBlastUtil::IsCSRA(dbs[i])) {
			csra_list += dbs[i] + " ";
		}
		else {
			not_csra_list += dbs[i] + " ";
		}
	}
	return csra_list;
}

vector<string>
CBlastVdbCmdApp::x_GetQueries()
{
    const CArgs& args = GetArgs();
    vector<string> retval;

    if (args["entry"].HasValue()) {

        static const string kDelim(",");
        const string& entry = args["entry"].AsString();

        if (entry.find(kDelim[0]) != string::npos) {
            vector<string> tokens;
            NStr::Split(entry, kDelim, tokens);
            retval.swap(tokens);
        } else {
            retval.push_back(entry);
        }

    } else if (args["entry_batch"].HasValue()) {

        CNcbiIstream& input = args["entry_batch"].AsInputFile();
        retval.reserve(256); // arbitrary value
        while (input) {
            string line;
            NcbiGetlineEOL(input, line);
            if ( !line.empty() ) {
                retval.push_back(line);
            }
        }
    } else {
        NCBI_THROW(CInputException, eInvalidInput, 
                   "Must specify query type: one of 'entry', or 'entry_batch'");
    }

    if (retval.empty()) {
        NCBI_THROW(CInputException, eInvalidInput,
                   "Entry not found in BLAST database");
    }

    return retval;
}

int
CBlastVdbCmdApp::x_ProcessSearchRequest()
{
    const CArgs& args = GetArgs();
    CNcbiOstream& out = args["out"].AsOutputFile();

    bool errors_found = false;

    /* Special case: full db dump */
    if (args["entry"].HasValue() && args["entry"].AsString() == "all") {
        try {
        	CRef<CVDBBlastUtil> util = x_GetVDBBlastUtil(m_isRef);
        	CVdbFastaExtractor seq_fmt(util, out, args["outfmt"].AsString(), args["line_length"].AsInteger());
            seq_fmt.DumpAll();
        } catch (const CException& e) {
            ERR_POST(Error << e.GetMsg());
            errors_found = true;
        } catch (...) {
            ERR_POST(Error << "Failed to retrieve requested item");
            errors_found = true;
        }
        return errors_found ? 1 : 0;
    }

    vector<string> queries = x_GetQueries();
    _ASSERT( !queries.empty() );

    CRef<CVDBBlastUtil> util = x_GetVDBBlastUtil(false);
    CVdbFastaExtractor seq_fmt(util, out, args["outfmt"].AsString(), args["line_length"].AsInteger());

   	CRef<CVDBBlastUtil> util_csra = x_GetVDBBlastUtil(true);
    CVdbFastaExtractor * seq_fmt_csra = NULL;
   	if(util_csra.NotEmpty()) {
    	seq_fmt_csra = new CVdbFastaExtractor(util_csra, out, args["outfmt"].AsString(), args["line_length"].AsInteger());
    }

    NON_CONST_ITERATE(vector<string>, itr, queries) {
    	try {
    		CRef<CSeq_id> seq_id;
    		try {
    			seq_id.Reset(new CSeq_id(*itr));
    		} catch (const CException & e) {
    			*itr = "SRA:" + *itr;
    			seq_id.Reset(new CSeq_id(*itr));
    	    }
        	switch (CVDBBlastUtil::VDBIdType(*seq_id)) {
        	case CVDBBlastUtil::eSRAId:
        	case CVDBBlastUtil::eWGSId:
        			seq_fmt.Write(seq_id);
        	break;
        	case CVDBBlastUtil::eCSRALocalRefId:
        	case CVDBBlastUtil::eCSRARefId:
        	{
        		if(seq_fmt_csra == NULL) {
           			NCBI_THROW(CInputException, eInvalidInput, *itr + ": CSRA ref seq id for non CSRA db");
           		}
           		seq_fmt_csra->Write(seq_id);
           	}
        	break;
        	default :
       			NCBI_THROW(CInputException, eInvalidInput, *itr + " is not a valid SRA, CSRA ref or WGS id");
       		break;
        	}

        } catch (const CException& e) {
            ERR_POST(e.GetMsg());
            errors_found = true;
        } catch (...) {
            ERR_POST("Failed to retrieve requested item");
            errors_found = true;
        }

    }
    if(seq_fmt_csra != NULL) {
       	delete seq_fmt_csra;
    }
    return errors_found ? 1 : 0;
}

string
CBlastVdbCmdApp::x_FormatRuntime(const CStopWatch& sw) const
{
    return sw.AsSmartString();
}

void
CBlastVdbCmdApp::x_InitApplicationData()
{
    const CArgs& args = GetArgs();
    string strAllRuns;
    if (args["db"]) {
        strAllRuns = args["db"].AsString();

    } else {
        CNcbiIstream& in = args["dbs_file"].AsInputFile();
        string line;
        while (NcbiGetline(in, line, "\n")) {
            if (line.empty()) {
                continue;
            }
            strAllRuns += line + " ";
        }
    }
    list<string> tmp;
    NStr::Split(strAllRuns, "\n\t ", tmp, NStr::fSplit_Tokenize);
    m_origDbs = NStr::Join(tmp, " ");
    if (args["ref"]) {
    	m_isRef = true;
    }
    else {
    	m_isRef = false;
    }
}

void
CBlastVdbCmdApp::x_GetFullPaths()
{
	vector<string> vdbs;
	vector<string> vdb_alias;
	vector<string> db_alias;
	CVDBAliasUtil::FindVDBPaths(m_origDbs, false, vdbs, &db_alias, &vdb_alias, true, true);

	m_allDbs = NStr::Join(vdbs, " ");
}

CRef<CVDBBlastUtil>
CBlastVdbCmdApp::x_GetVDBBlastUtil(bool isCSRA)
{
	CRef<CVDBBlastUtil>  util;
	if (isCSRA) {
		string not_csra_list = kEmptyStr;
		string csra_list = s_GetCSRADBs(m_allDbs, not_csra_list);
		if(csra_list == kEmptyStr) {
			return util;
		}

		CStopWatch sw;
		sw.Start();
		util.Reset(new CVDBBlastUtil(csra_list, true, true));
		sw.Stop();
		LOG_POST(Info << "PERF: blast_vdb library csra initialization: " << x_FormatRuntime(sw));
	}
	else {
		CStopWatch sw;
    	sw.Start();
    	util.Reset(new CVDBBlastUtil(m_allDbs, true, false));
    	sw.Stop();
    	LOG_POST(Info << "PERF: blast_vdb library initialization: " << x_FormatRuntime(sw));
	}
    return util;
}

void s_PrintStr(const string & str, unsigned int line_width, CNcbiOstream & out)
{
	list<string> print_str;
	NStr::Wrap(str, line_width, print_str);
	ITERATE(list<string>, itr, print_str) {
		out << *itr << endl;
	}
}

int
CBlastVdbCmdApp::x_PrintBlastDatabaseInformation()
{
    const string kLetters("bases");
    const CArgs& args = GetArgs();
    CNcbiOstream& out = args["out"].AsOutputFile();
    unsigned int line_width = args["line_length"].AsInteger();
    Uint8 num_seqs(0), length(0), max_seq_length(0), av_seq_length(0);
    Uint8 ref_num_seqs(0), ref_length(0);
    string not_csra_dbs = kEmptyStr;
    string csra_dbs = s_GetCSRADBs(m_allDbs, not_csra_dbs);
    CStopWatch sw;
    sw.Start();
    CVDBBlastUtil::GetVDBStats(m_allDbs, num_seqs, length, max_seq_length, av_seq_length);
    if(csra_dbs != kEmptyStr) {
    	CVDBBlastUtil::GetVDBStats(csra_dbs, ref_num_seqs, ref_length, true);
    }
    sw.Stop();

    // Print basic database information
    out << "Database(s): ";
    if(m_origDbs.size() > line_width) {
    	out << endl;
    	s_PrintStr(m_origDbs, line_width, out);
    }
    else {
    	out << m_origDbs << endl;
    }

    out << "Database(s) Full Path: ";
    if(m_allDbs.size() > line_width) {
    	out << endl;
    	s_PrintStr(m_allDbs, line_width, out);
    }
    else {
    	out << m_allDbs << endl;
    }
    out << "\t" << NStr::ULongToString(num_seqs, kFlags) << " sequences; ";
    out << NStr::ULongToString(length, kFlags)  << " total " << kLetters << " (unclipped)" << endl;
    out << "\tLongest sequence: "  << NStr::ULongToString(max_seq_length, kFlags) << " " << kLetters << endl;
    out << "\tAverage sequence: "  << NStr::ULongToString(av_seq_length, kFlags) << " " << kLetters << endl;

    if(csra_dbs != kEmptyStr) {
    	if(not_csra_dbs != kEmptyStr) {
    		out << "CSRA Database(s): ";
    		if(csra_dbs.size() > line_width) {
    		   	out << endl;
    		   	s_PrintStr(csra_dbs, line_width, out);
    		}
    		else {
    		   	out << csra_dbs << endl;
    		}
    	}
    	out << "\t" << NStr::ULongToString(ref_num_seqs, kFlags) << " ref sequences; ";
    	out << NStr::ULongToString(ref_length, kFlags)  << " total ref " << kLetters << endl;
    }

    LOG_POST(Info << "PERF: Get all BLASTDB metadata: " << x_FormatRuntime(sw));
    return 0;
}

int
CBlastVdbCmdApp::x_PrintVDBPaths(bool recursive)
{
    const CArgs& args = GetArgs();
    CNcbiOstream& out = args["out"].AsOutputFile();
    vector<string> vdbs;
    vector<string> vdb_alias;
    vector<string> db_alias;

    CStopWatch sw;
    sw.Start();
    CVDBAliasUtil::FindVDBPaths(m_origDbs, false, vdbs, &db_alias, &vdb_alias, recursive, true);
    sw.Stop();

    // Print basic database information
    out << "VDB(s): ";
    if(vdbs.empty()) {
    	out << "None" << endl;
    }
    else {
    	out << endl;
    	ITERATE(vector<string>, itr, vdbs)
    		out << *itr << endl;
   	}

    if(recursive) {
    	out << "VDB Alias File(s): ";
    	if(vdb_alias.empty()) {
       		out << "None" << endl;
    	}
    	else {
       		out << endl;
       		ITERATE(vector<string>, itr, vdb_alias)
     		out << *itr << endl;
    	}

    	out << "Blats DB Alias File(s): ";
    	if(db_alias.empty()) {
       		out << "None" << endl;
    	}
    	else {
       		out << endl;
       		ITERATE(vector<string>, itr, db_alias)
       			out << *itr << endl;
    	}
    }
    LOG_POST(Info << "PERF: Get Paths : " << x_FormatRuntime(sw));
    return 0;
}

void CBlastVdbCmdApp::Init()
{
    HideStdArgs(fHideConffile | fHideFullVersion | fHideXmlHelp | fHideDryRun);

    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(), "BLAST-VDB Cmd");

    // SRA-related parameters
    arg_desc->SetCurrentGroup("VDB 'BLASTDB' options");
    arg_desc->AddKey("db", "VDB_ACCESSIONS",
                     "List of whitespace-separated VDB accessions",
                     CArgDescriptions::eString);
    arg_desc->AddKey("dbs_file", "Input_File_with_VDB_ACCESSIONS",
                     "File with a newline delimited list of VDB Run accessions",
                     CArgDescriptions::eInputFile);
    arg_desc->SetDependency("db", CArgDescriptions::eExcludes, "dbs_file");
    // The format specifiers below should be handled in
    // CSeqFormatter::x_Builder
    arg_desc->AddDefaultKey("outfmt", "format",
                            "Output format, where the available format specifiers are:\n"
                            "\t\t%f means sequence in FASTA format\n"
                            "\t\t%s means sequence data (without defline)\n"
                            "\t\t%a means accession\n"
                            "\t\t%o means ordinal id (OID)\n"
                            "\t\t%i means sequence id\n"
                            "\t\t%t means sequence title\n"
                            "\t\t%l means sequence length\n"
                            "\tFor every format except '%f', each line of output will "
                            "correspond\n\tto a sequence.\n",
                            CArgDescriptions::eString, "%f");

    arg_desc->SetCurrentGroup("Retrieval options");
    arg_desc->AddOptionalKey("entry", "sequence_identifier",
                             "Comma-delimited search string(s) of sequence identifiers"
                             ":\n\te.g.: 'gnl|SRR|SRR066117.18823.2', or 'all' "
                             "to select all\n\tsequences in the database",
                             CArgDescriptions::eString);
    arg_desc->AddOptionalKey("entry_batch", "input_file", 
                             "Input file for batch processing (Format: one entry per line)",
                             CArgDescriptions::eInputFile);
    arg_desc->SetDependency("entry_batch", CArgDescriptions::eExcludes, "entry");
    arg_desc->AddDefaultKey("line_length", "number", "Line length for output",
                            CArgDescriptions::eInteger,
                            NStr::IntToString(80));
    arg_desc->SetConstraint("line_length", 
                            new CArgAllowValuesGreaterThanOrEqual(1));

    const char* exclusions[]  = { "entry", "entry_batch"};
    for (size_t i = 0; i < sizeof(exclusions)/sizeof(*exclusions); i++) {
        arg_desc->SetDependency(exclusions[i], CArgDescriptions::eExcludes, "info");
    }

    arg_desc->AddFlag("info", "Print VDB information", true);
    arg_desc->AddFlag("ref",
                      "Dump reference seqs", true);
    arg_desc->SetDependency("ref", CArgDescriptions::eExcludes, "info");
    arg_desc->SetDependency("ref", CArgDescriptions::eExcludes, "entry_batch");

    arg_desc->AddFlag("paths", "Get top level paths", true);
    arg_desc->AddFlag("paths_all", "Get all vdb and alias paths", true);
    const char* exclude_paths[]  = { "scan_uncompressed", "scan_compressed", "info", "entry", "entry_batch"};
    for (size_t i = 0; i < sizeof(exclude_paths)/sizeof(*exclude_paths); i++) {
        arg_desc->SetDependency("paths", CArgDescriptions::eExcludes, exclude_paths[i]);
        arg_desc->SetDependency("paths_all", CArgDescriptions::eExcludes, exclude_paths[i]);
    }
    arg_desc->SetCurrentGroup("Output configuration options");
    arg_desc->AddDefaultKey("out", "output_file", "Output file name", 
                            CArgDescriptions::eOutputFile, "-");

    SetupArgDescriptions(arg_desc.release());
}

int CBlastVdbCmdApp::Run(void)
{
    int status = 0;
    const CArgs& args = GetArgs();

    SetDiagPostLevel(eDiag_Warning);
    SetDiagPostPrefix("blast_vdb_cmd");
   	x_InitApplicationData();
    try {
       	if(args["paths"].HasValue()) {
        		status = x_PrintVDBPaths(false);
        		return status;
        }
       	if(args["paths_all"].HasValue()) {
        		status = x_PrintVDBPaths(true);
        		return status;
        }

       	x_GetFullPaths();
        if (args["info"]) {
            status = x_PrintBlastDatabaseInformation();
        }
        else if (args["entry"].HasValue() || args["entry_batch"].HasValue()) {
            	status = x_ProcessSearchRequest();
        }
    } catch (const CException& e) {
        LOG_POST(Error << "VDB Blast error: " << e.GetMsg());          \
        status = 1;
    } catch (const exception& e) {
        status = 1;
    } catch (...) {
        LOG_POST(Error << "Unknown exception!");
        status = 1;
    }
    x_AddCmdOptions();
    m_UsageReport.AddParam(CBlastUsageReport::eExitStatus, status);
    return status;
}

void CBlastVdbCmdApp::x_AddCmdOptions()
{
    const CArgs & args = GetArgs();
    if (args["info"]) {
         m_UsageReport.AddParam(CBlastUsageReport::eDBInfo, true);
    }
    else if(args["entry"].HasValue() || args["entry_batch"].HasValue()) {
         m_UsageReport.AddParam(CBlastUsageReport::eDBEntry, true);
         if (args["entry"].HasValue() && args["entry"].AsString() == "all") {
            m_UsageReport.AddParam(CBlastUsageReport::eDBDumpAll, true);
        }
    }
    if(args["outfmt"].HasValue()) {
        m_UsageReport.AddParam(CBlastUsageReport::eOutputFmt, args["outfmt"].AsString());
    }

    if (m_origDbs != kEmptyStr) {
    	m_UsageReport.AddParam(CBlastUsageReport::eDBName, m_origDbs);
    }
}


#ifndef SKIP_DOXYGEN_PROCESSING
int main(int argc, const char* argv[] /*, const char* envp[]*/)
{
    return CBlastVdbCmdApp().AppMain(argc, argv, 0, eDS_Default, "");
}
#endif /* SKIP_DOXYGEN_PROCESSING */
