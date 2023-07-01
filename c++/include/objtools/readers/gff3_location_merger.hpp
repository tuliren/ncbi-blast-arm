/*
 * $Id: gff3_location_merger.hpp 650647 2022-05-31 15:10:29Z ludwigf $
 *
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
 * Authors: Frank Ludwig
 *
 */

#ifndef _GFF3_LOCATION_MERGER_HPP_
#define _GFF3_LOCATION_MERGER_HPP_

#include <corelib/ncbistd.hpp>
#include <objtools/readers/gff3_reader.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects);

class CGff2Record;

//  ============================================================================
class CGff3LocationRecord
//  ============================================================================
{
public:
    CGff3LocationRecord(
        const CGff2Record&,
        unsigned int,
        CGff3ReadRecord::SeqIdResolver);

    CGff3LocationRecord(
        const CGff3LocationRecord&);

    CSeq_id mGffId;
    TSeqPos mStart;
    TSeqPos mStop;
    ENa_strand mStrand;
    string mType;
    size_t mPartNum;
    CCdregion::EFrame mFrame;
    string mSeqId;

    static bool ComparePartNumbers(
        const CGff3LocationRecord& lhs,
        const CGff3LocationRecord& rhs) { return lhs.mPartNum < rhs.mPartNum; };

    static bool ComparePositions(
        const CGff3LocationRecord& lhs,
        const CGff3LocationRecord& rhs);
};

//  ============================================================================
class CGffIdTrackRecord
//  ============================================================================
{
public:
    CGffIdTrackRecord(
        const CGff2Record& record):
        mSeqType(record.NormalizedType()),
        mSeqId(record.Id()),
        mSeqStart(record.SeqStart()),
        mSeqStop(record.SeqStop()),
        mSeqStrand(record.Strand())
    {};

    string mSeqType;
    string mSeqId;
    TSeqPos mSeqStart;
    TSeqPos mSeqStop;
    ENa_strand mSeqStrand;
};

//  ============================================================================
class CGffIdTracker
//  ============================================================================
{
public:
    void CheckAndIndexRecord(
        string id,
        const CGff2Record& record);

    void CheckAndIndexRecord(
        const CGff2Record& record);

    void CheckIntegrity();

private:
    map<string, list<CGffIdTrackRecord>> mIds;
    set<string> mParentIds;
};


//  ============================================================================
class NCBI_XOBJREAD_EXPORT CGff3LocationMerger
//  ============================================================================
{
    using LOCATIONS = list<CGff3LocationRecord>;
    using LOCATION_MAP = map<string, LOCATIONS>;

public:
    CGff3LocationMerger(
        unsigned int flags =0,
        CGff3ReadRecord::SeqIdResolver = CReadUtil::AsSeqId,
        TSeqPos sequenceSize =0);

    void Reset() {
        mMapIdToLocations.clear();
    };

    void SetSequenceSize(
        const string& seqId,
        TSeqPos sequenceSize) { mSequenceSizes[seqId] = sequenceSize; }

    bool AddRecord(
        const CGff2Record&);

    void AddRecordForId(
        const string&,
        const CGff2Record&);

    void VerifyRecordLocation(
        const CGff2Record&);

    LOCATION_MAP& LocationMap() { return mMapIdToLocations; }

    void GetLocation(
        const string&,
        CRef<CSeq_loc>&,
        CCdregion::EFrame&);

    void MergeLocation(
        CRef<CSeq_loc>&,
        CCdregion::EFrame&,
        LOCATIONS&);

    TSeqPos SequenceSize() const {
        if (mSequenceSizes.size() == 1) {
            return mSequenceSizes.begin()->second;
        } 
        return 0;
    }

    TSeqPos GetSequenceSize(
        const string&) const;

    void Validate();

private:
    static bool xGetLocationIds(
        const CGff2Record&,
        list<string>&);

    CRef<CSeq_loc> xGetRecordLocation(
        const CGff3LocationRecord&);

    static void xSortLocations(
        LOCATIONS&);

    unsigned int mFlags;
    map<string, TSeqPos> mSequenceSizes;
    CGff3ReadRecord::SeqIdResolver mIdResolver;

    LOCATION_MAP mMapIdToLocations;
    CGffIdTracker mIdTracker;
};

END_SCOPE(objects)
END_NCBI_SCOPE

#endif // _GFF3_LOCATION_MERGER_HPP_
