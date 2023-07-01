/*  $Id: test_vdbgraph_loader.cpp 632697 2021-06-04 20:03:56Z vasilche $
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
* Author:  Eugene Vasilchenko
*
* File Description:
*   Unit tests for VDB graph data loader
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <sra/data_loaders/vdbgraph/vdbgraphloader.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_table_ci.hpp>
#include <objects/seqtable/seqtable__.hpp>
#include <sra/readers/ncbi_traces_path.hpp>

#include <corelib/test_boost.hpp>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;
USING_SCOPE(objects);

static string s_LoaderName;

static const size_t kMainChunkSize = 100000;

static CRef<CScope> s_MakeScope(const string& file_name = kEmptyStr)
{
    CRef<CObjectManager> om = CObjectManager::GetInstance();
    if ( !s_LoaderName.empty() ) {
        om->RevokeDataLoader(s_LoaderName);
        s_LoaderName.erase();
    }
    if ( file_name.empty() ) {
        s_LoaderName =
            CVDBGraphDataLoader::RegisterInObjectManager(*om)
            .GetLoader()->GetName();
    }
    else {
        s_LoaderName =
            CVDBGraphDataLoader::RegisterInObjectManager(*om, file_name)
            .GetLoader()->GetName();
    }
    CRef<CScope> scope(new CScope(*om));
    scope->AddDataLoader(s_LoaderName);
    return scope;
}


static void s_CheckFast(CScope& scope, const CSeq_id& id, TSeqPos seq_len,
                        const vector<string>& naccs,
                        bool only_main = true,
                        bool no_annots = false,
                        const vector<string>* other_accs = 0)
{
    NcbiCout << "Test with "<<naccs.size()<<" accs"<<NcbiEndl;
    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    ITERATE ( vector<string>, it, naccs ) {
        sel.IncludeNamedAnnotAccession(*it);
    }
    if ( other_accs ) {
        ITERATE ( vector<string>, it, *other_accs ) {
            sel.IncludeNamedAnnotAccession(*it);
        }
    }
    CSeq_loc loc;
    loc.SetWhole(*SerialClone(id));
    CGraph_CI graph_it(scope, loc, sel);
    CSeq_table_CI table_it(scope, loc, sel);
    if ( no_annots ) {
        BOOST_CHECK(!graph_it);
        if ( graph_it ) {
            NcbiCout << "Unexpected " << MSerial_AsnText
                     << graph_it->GetMappedGraph();
        }
        BOOST_CHECK(!table_it);
        if ( table_it ) {
            NcbiCout << "Unexpected " << MSerial_AsnText
                     << table_it.GetAnnot().GetCompleteObject()->GetData().GetSeq_table();
        }
        return;
    }
    if ( only_main ) {
        BOOST_CHECK_EQUAL(graph_it.GetSize()+table_it.GetSize(),
                          ((seq_len-1)/kMainChunkSize+1)*naccs.size());
    }
    else {
        BOOST_CHECK(graph_it.GetSize()+table_it.GetSize());
    }
    typedef map<string, CRange<TSeqPos> > TRanges;
    TRanges all, tables, graphs;
    for ( ; table_it; ++table_it ) {
        const string& name = table_it.GetAnnot().GetName();
        const CSeq_loc& loc = table_it.GetMappedLocation();
        CRange<TSeqPos> range = loc.GetTotalRange();
        if ( tables.count(name) == 0 ) {
            tables[name] = range;
        }
        else {
            BOOST_CHECK(tables[name].AbuttingWith(range));
            tables[name] += range;
        }
        if ( all.count(name) == 0 ) {
            all[name] = range;
        }
        else {
            BOOST_CHECK(all[name].AbuttingWith(range));
            all[name] += range;
        }
    }
    ITERATE ( TRanges, it, tables ) {
        NcbiCout << "Table "<<it->first << " " << it->second << NcbiEndl;
    }
    for ( ; graph_it; ++graph_it ) {
        const CSeq_graph& graph = graph_it->GetOriginalGraph();
        string name = graph_it.GetAnnot().GetName();
        if ( graph.IsSetComment() ) {
            name += "/"+graph.GetComment();
        }
        const CSeq_interval& interval = graph.GetLoc().GetInt();
        CRange<TSeqPos> range(interval.GetFrom(), interval.GetTo());
        if ( graphs.count(name) == 0 ) {
            graphs[name] = range;
        }
        else {
            BOOST_CHECK(graphs[name].AbuttingWith(range));
            graphs[name] += range;
        }
        if ( all.count(name) == 0 ) {
            all[name] = range;
        }
        else {
            BOOST_CHECK(all[name].AbuttingWith(range));
            all[name] += range;
        }
    }
    ITERATE ( TRanges, it, graphs ) {
        NcbiCout << "Graph "<<it->first << " " << it->second << NcbiEndl;
    }
    table_it.Rewind();
    graph_it.Rewind();
}


static void s_CheckFast(CScope& scope, const CSeq_id& id, TSeqPos seq_len,
                        const string& nacc,
                        bool only_main = true,
                        bool no_annots = false)
{
    s_CheckFast(scope, id, seq_len, vector<string>(1, nacc), only_main, no_annots);
    return;
    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    CSeq_loc loc;
    loc.SetWhole(*SerialClone(id));
    CGraph_CI graph_it(scope, loc, sel);
    CSeq_table_CI table_it(scope, loc, sel);
    if ( no_annots ) {
        BOOST_CHECK(!graph_it);
        BOOST_CHECK(!table_it);
        return;
    }
    
    BOOST_CHECK(graph_it.GetSize());
    if ( 0 ) {
        for ( ; graph_it; ++graph_it ) {
            NcbiCout << MSerial_AsnText << graph_it->GetOriginalGraph();
        }
        graph_it.Rewind();
    }
    if ( 1 ) {
        typedef map<string, CRange<TSeqPos> > TRanges;
        TRanges tables, graphs;
        for ( ; table_it; ++table_it ) {
            const string& name = table_it.GetAnnot().GetName();
            const CSeq_loc& loc = table_it.GetMappedLocation();
            CRange<TSeqPos> range = loc.GetTotalRange();
            if ( tables.count(name) == 0 ) {
                tables[name] = range;
            }
            else {
                BOOST_CHECK(tables[name].AbuttingWith(range));
                tables[name] += range;
            }
        }
        ITERATE ( TRanges, it, tables ) {
            NcbiCout << "Table "<<it->first << " " << it->second << NcbiEndl;
        }
        for ( ; graph_it; ++graph_it ) {
            const CSeq_graph& graph = graph_it->GetOriginalGraph();
            string name = graph_it.GetAnnot().GetName();
            if ( graph.IsSetComment() ) {
                name += "/"+graph.GetComment();
            }
            const CSeq_interval& interval = graph.GetLoc().GetInt();
            CRange<TSeqPos> range(interval.GetFrom(), interval.GetTo());
            if ( graphs.count(name) == 0 ) {
                graphs[name] = range;
            }
            else {
                BOOST_CHECK(tables[name].AbuttingWith(range));
                graphs[name] += range;
            }
        }
        ITERATE ( TRanges, it, graphs ) {
            NcbiCout << "Graph "<<it->first << " " << it->second << NcbiEndl;
        }
        table_it.Rewind();
        graph_it.Rewind();
    }
}


static void s_VerifyGraphs(CScope& scope, const CSeq_id& id, TSeqPos seq_len,
                           const string& nacc)
{
}


BOOST_AUTO_TEST_CASE(FetchSeq1)
{
    string nacc = "NA000008777.1";
    string nacc2 = "NA000008778.1";
    CSeq_id seqid1("NC_002333.2");
    TSeqPos seq_len = 16596;

    CRef<CScope> scope =
        s_MakeScope(NCBI_TRACES04_PATH "/nannot01/000/008/"+nacc);
    BOOST_REQUIRE_EQUAL(kInvalidSeqPos, scope->GetSequenceLength(seqid1));
    CBioseq_Handle handle1 = scope->GetBioseqHandle(seqid1);
    BOOST_REQUIRE(!handle1);

    s_CheckFast(*scope, seqid1, seq_len, nacc, false);
    s_VerifyGraphs(*scope, seqid1, seq_len, nacc);
    // VDB graph loader with fixed file should not find other NA
    s_CheckFast(*scope, seqid1, seq_len, nacc2, false, true);
}


BOOST_AUTO_TEST_CASE(FetchSeq2)
{
    string nacc = "NA000008777.1";
    string nacc2 = "NA000008778.1";
    CSeq_id seqid1("NC_002333.2");
    TSeqPos seq_len = 16596;

    CRef<CScope> scope = s_MakeScope();
    BOOST_REQUIRE_EQUAL(kInvalidSeqPos, scope->GetSequenceLength(seqid1));
    CBioseq_Handle handle1 = scope->GetBioseqHandle(seqid1);
    BOOST_REQUIRE(!handle1);

    s_CheckFast(*scope, seqid1, seq_len, nacc, false);
    s_VerifyGraphs(*scope, seqid1, seq_len, nacc);
    // VDB graph loader with auto NA search should find other NAs
    s_CheckFast(*scope, seqid1, seq_len, nacc2, false);

    SAnnotSelector sel;
    sel.SetSearchUnresolved();
    sel.IncludeNamedAnnotAccession("NA000008777.1");
    CSeq_loc loc;
    loc.SetWhole(seqid1);
    CGraph_CI graph_it(*scope, loc, sel);
    CSeq_table_CI table_it(*scope, loc, sel);
    BOOST_CHECK_EQUAL(graph_it.GetSize()+table_it.GetSize(),
                      (seq_len-1)/kMainChunkSize+1);
    if ( 0 ) {
        for ( ; graph_it; ++graph_it ) {
            NcbiCout << MSerial_AsnText << graph_it->GetOriginalGraph();
        }
        graph_it.Rewind();
    }
    if ( 1 ) {
        typedef map<string, CRange<TSeqPos> > TRanges;
        TRanges all, tables, graphs;
        for ( ; table_it; ++table_it ) {
            const string& name = table_it.GetAnnot().GetName();
            const CSeq_loc& loc = table_it.GetMappedLocation();
            CRange<TSeqPos> range = loc.GetTotalRange();
            if ( tables.count(name) == 0 ) {
                tables[name] = range;
            }
            else {
                BOOST_CHECK(tables[name].AbuttingWith(range));
                tables[name] += range;
            }
            if ( all.count(name) == 0 ) {
                all[name] = range;
            }
            else {
                BOOST_CHECK(all[name].AbuttingWith(range));
                all[name] += range;
            }
        }
        ITERATE ( TRanges, it, tables ) {
            NcbiCout << "Table "<<it->first << " " << it->second << NcbiEndl;
        }
        for ( ; graph_it; ++graph_it ) {
            const CSeq_graph& graph = graph_it->GetOriginalGraph();
            string name = graph_it.GetAnnot().GetName();
            if ( graph.IsSetComment() ) {
                name += "/"+graph.GetComment();
            }
            const CSeq_interval& interval = graph.GetLoc().GetInt();
            CRange<TSeqPos> range(interval.GetFrom(), interval.GetTo());
            if ( graphs.count(name) == 0 ) {
                graphs[name] = range;
            }
            else {
                BOOST_CHECK(tables[name].AbuttingWith(range));
                graphs[name] += range;
            }
            if ( all.count(name) == 0 ) {
                all[name] = range;
            }
            else {
                BOOST_CHECK(all[name].AbuttingWith(range));
                all[name] += range;
            }
        }
        ITERATE ( TRanges, it, graphs ) {
            NcbiCout << "Graph "<<it->first << " " << it->second << NcbiEndl;
        }
        table_it.Rewind();
        graph_it.Rewind();
    }
    s_VerifyGraphs(*scope, seqid1, seq_len, nacc);
}

BOOST_AUTO_TEST_CASE(FetchSeq3)
{
    CSeq_id seqid1("NC_002333.2");
    TSeqPos seq_len = 16596;

    CRef<CScope> scope0 = s_MakeScope();
    for ( int t = 0; t < 4; ++t ) {
        CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
        scope->AddDataLoader(s_LoaderName);

        BOOST_REQUIRE_EQUAL(kInvalidSeqPos, scope->GetSequenceLength(seqid1));
        
        CBioseq_Handle handle1 = scope->GetBioseqHandle(seqid1);
        BOOST_REQUIRE(!handle1);
        vector<string> naccs;
        int first_na = 8777, last_na = t < 2? 8781: 8788;
        for ( int na = first_na; na <= last_na; ++na ) {
            char buf[99];
            sprintf(buf, "NA%09d.1", na);
            naccs.push_back(buf);
        }
        s_CheckFast(*scope, seqid1, seq_len, naccs);
        ITERATE ( vector<string>, it, naccs ) {
            s_VerifyGraphs(*scope, seqid1, seq_len, *it);
        }
    }
}

BOOST_AUTO_TEST_CASE(FetchSeq4)
{
    CSeq_id seqid1("NW_001877341.3");
    TSeqPos seq_len = 12372269;
    
    CRef<CScope> scope(new CScope(*CObjectManager::GetInstance()));
    scope->AddDataLoader(s_LoaderName);

    BOOST_REQUIRE_EQUAL(kInvalidSeqPos, scope->GetSequenceLength(seqid1));
        
    CBioseq_Handle handle1 = scope->GetBioseqHandle(seqid1);
    BOOST_REQUIRE(!handle1);
    vector<string> naccs;
    int first_na = 8777, last_na = 8781;
    for ( int na = first_na; na <= last_na; ++na ) {
        char buf[99];
        sprintf(buf, "NA%09d.1", na);
        naccs.push_back(buf);
    }
    s_CheckFast(*scope, seqid1, seq_len, naccs);
    ITERATE ( vector<string>, it, naccs ) {
        s_VerifyGraphs(*scope, seqid1, seq_len, *it);
    }
}

BOOST_AUTO_TEST_CASE(FetchWithSRR)
{
    string nacc = "NA000008777.1";
    string srr = "SRR000010";
    CSeq_id seqid1("NC_002333.2");
    TSeqPos seq_len = 16596;

    CRef<CScope> scope = s_MakeScope();
    BOOST_REQUIRE_EQUAL(kInvalidSeqPos, scope->GetSequenceLength(seqid1));
    CBioseq_Handle handle1 = scope->GetBioseqHandle(seqid1);
    BOOST_REQUIRE(!handle1);
    
    vector<string> other_accs(1, srr);
    s_CheckFast(*scope, seqid1, seq_len, vector<string>(1, nacc), false, false, &other_accs);
    s_VerifyGraphs(*scope, seqid1, seq_len, nacc);
}


BOOST_AUTO_TEST_CASE(CheckGI64VDBNAZoom)
{
    LOG_POST("Checking 64-bit GI VDB NA Tracks");
    string id = "NC_054141.5";
    string na_acc = "NA000300276.1";

    for ( int t = 0; t < 4; ++t ) {
        SAnnotSelector sel;
        sel.SetSearchUnresolved();
        sel.IncludeNamedAnnotAccession(na_acc, -1);
        if ( t&1 ) {
            sel.AddNamedAnnots(CombineWithZoomLevel(na_acc, -1));
        }
        if ( t&2 ) {
            sel.AddNamedAnnots(na_acc);
            sel.AddNamedAnnots(CombineWithZoomLevel(na_acc, 100));
        }
        sel.SetCollectNames();

        CRef<CSeq_loc> loc(new CSeq_loc);
        loc->SetWhole().Set(id);
        set<int> tracks;
        CRef<CScope> scope = s_MakeScope();
        CGraph_CI it(*scope, *loc, sel);
        ITERATE ( CGraph_CI::TAnnotNames, i, it.GetAnnotNames() ) {
            if ( !i->IsNamed() ) {
                continue;
            }
            int zoom;
            string acc;
            if ( !ExtractZoomLevel(i->GetName(), &acc, &zoom) ) {
                continue;
            }
            if ( acc != na_acc ) {
                continue;
            }
            tracks.insert(zoom);
        }
        BOOST_CHECK(tracks.count(100));
    }
}


BOOST_AUTO_TEST_CASE(CheckGI64VDBNAZoom100)
{
    LOG_POST("Checking 64-bit GI NA VDB Graph Track");
    string id = "NC_054141.5";
    string na_acc = "NA000300276.1";

    for ( int t = 0; t < 2; ++t ) {
        SAnnotSelector sel;
        sel.SetSearchUnresolved();
        sel.IncludeNamedAnnotAccession(na_acc, 100);
        if ( t&1 ) {
            sel.AddNamedAnnots(CombineWithZoomLevel(na_acc, -1));
        }
        else {
            sel.AddNamedAnnots(CombineWithZoomLevel(na_acc, 100));
        }
        
        CRef<CScope> scope = s_MakeScope();
        CRef<CSeq_id> seq_id(new CSeq_id(id));
        CRef<CSeq_loc> loc(new CSeq_loc);
        loc->SetWhole(*seq_id);

        BOOST_CHECK(CGraph_CI(*scope, *loc, sel));
    }
}


NCBITEST_INIT_TREE()
{
#ifdef NCBI_INT4_GI
    NCBITEST_DISABLE(CheckGI64VDBNAZoom)
    NCBITEST_DISABLE(CheckGI64VDBNAZoom100);
#endif
}
