/*  $Id: blastn_vdb_args.cpp 664823 2023-03-22 17:59:58Z ivanov $
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

/** @file blastn_args.cpp
 * Implementation of the BLASTN command line arguments
 */

#include <ncbi_pch.hpp>
#include <algo/blast/api/disc_nucl_options.hpp>
#include <algo/blast/api/blast_exception.hpp>
#include <algo/blast/blastinput/blast_input_aux.hpp>
#include <algo/blast/api/version.hpp>
#include <algo/blast/vdb/blastn_vdb_args.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(blast)
USING_SCOPE(objects);

const string kArgSRASearchMode("sra_mode");

void
CBlastVDatabaseArgs::SetArgumentDescriptions(CArgDescriptions& arg_desc)
{
    arg_desc.SetCurrentGroup("General search options");
    // database filename
    arg_desc.AddOptionalKey(kArgDb, "database_name", "SRA or WGS database name",
                            CArgDescriptions::eString);

    arg_desc.SetCurrentGroup("");

    vector<string> database_args;
    database_args.push_back(kArgDb);

    // DB size
    arg_desc.SetCurrentGroup("Statistical options");
    arg_desc.AddOptionalKey(kArgDbSize, "num_letters",
                            "Effective length of the database ",
                            CArgDescriptions::eInt8);

    arg_desc.SetCurrentGroup("");
}

void
CSRASearchModeArgs::SetArgumentDescriptions(CArgDescriptions& arg_desc)
{
    arg_desc.SetCurrentGroup("General search options");
    string kSRASearchMode = string(
        "SRA Search Mode:\n"
        "0 = unaligned reads only\n"
        "1 = aligned reference seqs only\n"
    	"2 = both unaligned reads and aligned reference seqs n");
    arg_desc.AddDefaultKey(kArgSRASearchMode, "SRA_search_mode", kSRASearchMode,
    		               CArgDescriptions::eInteger, "0");
}




CBlastnVdbAppArgs::CBlastnVdbAppArgs()
{
    CRef<IBlastCmdLineArgs> arg;
    static const string kProgram("blastn");
    arg.Reset(new CProgramDescriptionArgs(kProgram,
                                          "Nucleotide-Nucleotide BLAST"));
    const bool kQueryIsProtein = false;
    m_Args.push_back(arg);
    m_ClientId = kProgram + " " + CBlastVersion().Print();

    static const string kDefaultTask = "megablast";
    SetTask(kDefaultTask);
    set<string> tasks
        (CBlastOptionsFactory::GetTasks(CBlastOptionsFactory::eNuclNucl));
    tasks.erase("vecscreen"); // vecscreen has its own program
    arg.Reset(new CTaskCmdLineArgs(tasks, kDefaultTask));
    m_Args.push_back(arg);

    m_BlastDbArgs.Reset(new CBlastVDatabaseArgs);
    m_BlastDbArgs->SetDatabaseMaskingSupport(false);
    arg.Reset(m_BlastDbArgs);
    m_Args.push_back(arg);

    m_StdCmdLineArgs.Reset(new CStdCmdLineArgs);
    arg.Reset(m_StdCmdLineArgs);
    m_Args.push_back(arg);

    arg.Reset(new CGenericSearchArgs(eBlastTypeBlastn));
    m_Args.push_back(arg);

    arg.Reset(new CNuclArgs);
    m_Args.push_back(arg);

    arg.Reset(new CDiscontiguousMegablastArgs);
    m_Args.push_back(arg);

    arg.Reset(new CFilteringArgs(kQueryIsProtein));
    m_Args.push_back(arg);

    arg.Reset(new CGappedArgs);
    m_Args.push_back(arg);

    m_HspFilteringArgs.Reset(new CHspFilteringArgs);
    arg.Reset(m_HspFilteringArgs);
    m_Args.push_back(arg);

    arg.Reset(new CWindowSizeArg);
    m_Args.push_back(arg);

    arg.Reset(new COffDiagonalRangeArg);
    m_Args.push_back(arg);

    m_QueryOptsArgs.Reset(new CQueryOptionsArgs(kQueryIsProtein));
    arg.Reset(m_QueryOptsArgs);
    m_Args.push_back(arg);

    m_FormattingArgs.Reset(new CFormattingArgs(false, CFormattingArgs::eIsVDB_SAM));
    arg.Reset(m_FormattingArgs);
    m_Args.push_back(arg);

    m_MTArgs.Reset(new CMTArgs);
    arg.Reset(m_MTArgs);
    m_Args.push_back(arg);

    m_VDBSearchModeArgs.Reset(new CSRASearchModeArgs);
    arg.Reset(m_VDBSearchModeArgs);
    m_Args.push_back(arg);

    m_RemoteArgs.Reset(new CRemoteArgs);

    m_DebugArgs.Reset(new CDebugArgs);
    arg.Reset(m_DebugArgs);
    m_Args.push_back(arg);
}

CRef<CBlastOptionsHandle> 
CBlastnVdbAppArgs::x_CreateOptionsHandle(CBlastOptions::EAPILocality locality,
                                      const CArgs& args)
{
    CRef<CBlastOptionsHandle> retval;
    SetTask(args[kTask].AsString());
    retval.Reset(CBlastOptionsFactory::CreateTask(GetTask(), locality));
    _ASSERT(retval.NotEmpty());
    return retval;
}

int
CBlastnVdbAppArgs::GetQueryBatchSize() const
{
    bool is_remote = (m_RemoteArgs.NotEmpty() && m_RemoteArgs->ExecuteRemotely());
    return blast::GetQueryBatchSize(ProgramNameToEnum(GetTask()), m_IsUngapped, is_remote, false);
}

END_SCOPE(blast)
END_NCBI_SCOPE

