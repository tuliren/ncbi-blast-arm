/*  $Id: gene_info_reader_app.cpp 628650 2021-04-05 12:32:17Z grichenk $
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
 * Author:  Vahram Avagyan
 *
 */

/// @file gene_info_reader_app.cpp
/// Demo command-line application for reading Gene information from files.
///
/// This source file contains a command-line application that uses the
/// Gene info reader library to convert Gene IDs to/from Gis and
/// to produce the Gene Info description lines given a Gi or a Gene ID.

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>

#include <objtools/blast/gene_info_reader/gene_info_reader.hpp>

#ifndef SKIP_DOXYGEN_PROCESSING
USING_NCBI_SCOPE;
#endif /* SKIP_DOXYGEN_PROCESSING */

//==========================================================================//

/// CReadFilesApp
///
/// Class implementing the Gene Info reader application.
///
/// CReadFilesApp is an NCBI command-line application that provides a
/// simple interface for converting Gene IDs to and from Gis and for
/// retrieving and formatting Gene Information for given Gene IDs and Gis.

class CReadFilesApp : public CNcbiApplication
{
private:
    /// Initialize the Application.
    virtual void Init(void);
    /// Run the Application.
    virtual int  Run(void);
    /// Exit the Application.
    virtual void Exit(void);

    /// Output a list of integers to stdout.
    /// @param listVals
    ///     List of integer values to output.
    void OutputIntList(const list<int>& listVals);
    /// Output a list of TGi values to stdout.
    /// @param listVals
    ///     List of TGi values to output.
    void OutputGiList(const list<TGi>& listVals);
    /// Output a list of Gene Information objects to stdout.
    /// @param listInfos
    ///     List of Gene Information objects to output.
    void OutputInfoList(IGeneInfoInput::TGeneInfoList& listInfos);
};

//==========================================================================//

void CReadFilesApp::
        OutputIntList(const list<int>& listVals)
{
    list<int>::const_iterator it = listVals.begin();
    for (; it != listVals.end(); it++)
    {
        cout << *it << " ";
    }
    cout << endl;
}

void CReadFilesApp::
OutputGiList(const list<TGi>& listVals)
{
    list<TGi>::const_iterator it = listVals.begin();
    for (; it != listVals.end(); it++)
    {
        cout << *it << " ";
    }
    cout << endl;
}

void CReadFilesApp::
        OutputInfoList(IGeneInfoInput::TGeneInfoList& listInfos)
{
    IGeneInfoInput::TGeneInfoList::iterator it = listInfos.begin();
    for (; it != listInfos.end(); it++)
    {
        string strInfo;
        (*it)->ToString(strInfo, false);
        cout << strInfo << endl;
    }
}

void CReadFilesApp::Init(void)
{
    HideStdArgs(fHideLogfile | fHideConffile | fHideVersion);

    // Create command-line argument descriptions class
    unique_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
      "The program can be used to convert Gene IDs to/from Gis, "
      "and print Gene Info lines given Gis or Gene IDs.");

    // Gi to Gene ID
    arg_desc->AddDefaultKey ("gi2id", "Gi", 
        "The Gi to convert to a Gene ID",
        CArgDescriptions::eInteger,
        "0");

    // Gene ID to Gi
    arg_desc->AddDefaultKey ("id2gi", "GeneID", 
        "The Gene ID to convert to a Gi",
        CArgDescriptions::eInteger,
        "0");

    // Gi to Gene Info
    arg_desc->AddDefaultKey ("gi2info", "Gi", 
        "The Gi to print the Gene Info line(s) for",
        CArgDescriptions::eInteger,
        "0");

    // Gene ID to Gene Info
    arg_desc->AddDefaultKey ("id2info", "GeneID", 
        "The Gene ID to print the Gene Info line for",
        CArgDescriptions::eInteger,
        "0");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}

int CReadFilesApp::Run(void)
{
    int nRetval = 0;
    try
    {
        // Create the reader object. This version of the constructor reads
        // the path to the Gene Info files from the GENE_INFO_PATH
        // environment variable.
        CGeneInfoFileReader fileReader;

        TGi gi2id = GI_FROM(TIntId, GetArgs()["gi2id"].AsIntId());
        int id2gi = GetArgs()["id2gi"].AsInteger();
        TGi gi2info = GI_FROM(TIntId, GetArgs()["gi2info"].AsIntId());
        int id2info = GetArgs()["id2info"].AsInteger();

        if (gi2id > ZERO_GI)
        {
            IGeneInfoInput::TGeneIdList idList;
            if (fileReader.GetGeneIdsForGi(gi2id, idList))
            {
                cout << "Gene IDs for Gi=" << gi2id << ":" << endl;
                OutputIntList(idList);
            }
            else
            {
                cout << "No Gene IDs found for Gi=" << gi2id << endl;
            }
        }

        if (id2gi > 0)
        {
            IGeneInfoInput::TGiList giListRNA, giListProtein, giListGenomic;
            bool bRNA, bProtein, bGenomic;
            bRNA     = fileReader.GetRNAGisForGeneId(id2gi, giListRNA);
            bProtein = fileReader.GetProteinGisForGeneId(id2gi, giListProtein);
            bGenomic = fileReader.GetGenomicGisForGeneId(id2gi, giListGenomic);

            if (bRNA)
            {
                cout << "RNA Gis for Gene ID=" << id2gi << ":" << endl;
                OutputGiList(giListRNA);
            }
            else
            {
                cout << "No RNA Gis for Gene ID=" << id2gi << endl;
            }

            if (bProtein)
            {
                cout << "Protein Gis for Gene ID=" << id2gi << ":" << endl;
                OutputGiList(giListProtein);
            }
            else
            {
                cout << "No Protein Gis for Gene ID=" << id2gi << endl;
            }

            if (bGenomic)
            {
                cout << "Genomic Gis for Gene ID=" << id2gi << ":" << endl;
                OutputGiList(giListGenomic);
            }
            else
            {
                cout << "No Genomic Gis found for Gene ID=" << id2gi << endl;
            }
        }

        if (gi2info > ZERO_GI)
        {
            IGeneInfoInput::TGeneInfoList listInfos;
            if (fileReader.GetGeneInfoForGi(gi2info, listInfos))
            {
                cout << "Gene Info for Gi=" << gi2info << ":" << endl;
                OutputInfoList(listInfos);
            }
            else
            {
                cout << "No Gene Info found for Gi=" << gi2info << endl;
            }
        }

        if (id2info > 0)
        {
            IGeneInfoInput::TGeneInfoList listInfos;
            if (fileReader.GetGeneInfoForId(id2info, listInfos))
            {
                cout << "Gene Info for Gene ID=" << id2info << ":" << endl;
                OutputInfoList(listInfos);
            }
            else
            {
                cout << "No Gene Info found for Gene ID=" << id2info << endl;
            }
        }
    }
    catch (CException& e)
    {
        cerr << endl << "Reading Gene Info failed: "
             << e.what() << endl;
        nRetval = 1;
    }

    return nRetval;
}

void CReadFilesApp::Exit(void)
{
    SetDiagStream(0);
}

//==========================================================================//

int main(int argc, const char* argv[])
{
    // Execute main application function
    return CReadFilesApp().AppMain(argc, argv);
}

