/* $Id: Seq_align.hpp 646952 2022-03-17 13:39:09Z mozese2 $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the data definition file
 *   'seqalign.asn'.
 */

#ifndef OBJECTS_SEQALIGN_SEQ_ALIGN_HPP
#define OBJECTS_SEQALIGN_SEQ_ALIGN_HPP


// generated includes
#include <objects/seqalign/Seq_align_.hpp>
#include <objects/seqloc/Na_strand.hpp>
#include <util/range_coll.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE /// namespace ncbi::objects::

class CSeq_id;

class NCBI_SEQALIGN_EXPORT CSeq_align : public CSeq_align_Base
{
    typedef CSeq_align_Base Tparent;
public:
    /// enum controlling known named scores
    ///
    /// A word on scores, since definitions are quite important:  Scores here
    /// are to be represented as a standard set of computations, comparable
    /// across alignment generation algorithms.  The class CScoreBuilder holds
    /// reference implementations for most of the scores below (modulo
    /// interpretation difficulties with a score named 'score').  The intended
    /// meanings of scores are as follows:
    ///
    /// 'score'
    ///     generic score, definable by any algorithm; not comparable
    ///     across algorithms.
    ///
    /// 'bit_score'
    ///     BLAST-specific bit score; reference implementation in the BLAST
    ///     code, which is also exposed in CScoreBuilder.
    ///
    /// 'e_value'
    ///     BLAST-specific e-value; reference implementation in the BLAST
    ///     code, which is also exposed in CScoreBuilder.
    ///
    /// alignment length
    ///     not a score per se, but a useful metric nonetheless.  This is the
    ///     sum of all aligned segments and all gaps; this excludes introns and
    ///     discontinuities
    ///
    /// percent identity
    ///     percent identity, computed as *UNGAPPED* percent identity.  That
    ///     is, the computation is strictly (matches) / (matches + mismatches)
    ///     NOTE: there are, historically, at least four separate methods of
    ///     computation for this:
    ///         1.  C++ toolkit (CScoreBuilder): ungapped percent identity:
    ///                 (matches) / (matches + mismatches)
    ///         2.  BLAST: gapped percent identity:
    ///                 (matches) / (alignment length)
    ///         3.  gbDNA and contig processes: modified alignment length to
    ///             count each gap as 1 base long always
    ///                 (matches) / (matches + mismatches + gap count)
    ///         4.  Spidey: compute identity per coverage:
    ///                 (matches) / (stop - start + 1)
    ///     Several of these are provided directly, as:
    ///         pct_identity_gap
    ///         pct_identity_ungap
    ///         pct_identity_gapopen_only
    ///
    ///  alignable bases
    ///     More a concept than a score.  The alignable region of a sequence,
    ///     as defined by the underlying sequencing technology or biology -
    ///     that is, the region of a sequence noted as high quality minus a
    ///     poly-A tail in the case of a cDNA.  This quantity can be used in
    ///     computing more appropriate coverage scores.
    ///
    /// 'pct_coverage'
    ///     Percent of a sequence (query) aligned to another (subject), minus
    ///     poly-A tail:
    ///         (matches + mismatches) / (query length - polyA tail)
    ///
    /// alignable percent coverage
    ///     (NB: no named score yet)
    ///     Percent of the alignable region actually aligned to a subject
    ///         (matches + mismatches in alignable region) / (length of alignable region)
    ///
    /// exonic coverage
    ///     (NB: no named score yet)
    ///     Measure of the percent of the query represented in a set of exons
    ///         (sum of lengths of exon on query) / (query length - polyA)
    ///
    /// @warning The order of this enumeration and the sc_ScoreNames,
    ///     sc_ScoreHelpText, and sc_IsInteger arrays must correspond!
    ///
    enum EScoreType {
        //< generic 'score'
        eScore_Score,

        //< blast 'score'
        eScore_Blast,

        //< blast-style 'bit_score'
        eScore_BitScore,

        //< blast-style 'e_value'
        eScore_EValue,

        //< alignment length (align_length)
        eScore_AlignLength,

        //< count of identities (num_ident)
        eScore_IdentityCount,

        //< count of positives (num_positives); protein-to-DNA score
        eScore_PositiveCount,

        //< count of negatives (num_negatives)
        eScore_NegativeCount,

        //< count of mismatches (num_mismatch)
        eScore_MismatchCount,

        //< number of gap bases in the alignment
        //< (= length of all gap segments)
        eScore_GapCount,

        //< percent identity (0.0-100.0) (pct_identity)
        //< NOTE: there are multiple ways to express this; the default is to
        //< consider a gap as a mismatch, as noted above.
        eScore_PercentIdentity_Gapped,
        eScore_PercentIdentity_Ungapped,
        eScore_PercentIdentity_GapOpeningOnly,

        //< percent coverage (0.0-100.0) (pct_coverage)
        eScore_PercentCoverage,

        //< blast-style 'sum_e'
        eScore_SumEValue,

        //< Composition-adjustment method from BLAST (comp_adjustment_method)
        eScore_CompAdjMethod,

        //< percent coverage (0.0-100.0) of high quality region (high_quality_pct_coverage)
        eScore_HighQualityPercentCoverage,

        //< Scores calculated by Splign
        eScore_Matches,
        eScore_OverallIdentity,
        eScore_Splices,
        eScore_ConsensusSplices,
        eScore_ProductCoverage,
        eScore_ExonIdentity,

        //< generic percent identity is an alias for gapped percent identity
        //< (i.e., BLAST-style percent identity)
        eScore_PercentIdentity = eScore_PercentIdentity_Gapped
    };

    typedef pair<TSeqPos, TSeqPos> TLengthRange;

    typedef map<string, EScoreType> TScoreNameMap;

    struct NCBI_SEQALIGN_EXPORT SIndel {
        TSeqPos product_pos;
        TSeqPos genomic_pos;
        TDim row;
        TSeqPos length;

        SIndel(TSeqPos p = 0, TSeqPos g = 0, TDim r = 0, TSeqPos l = 0)
        : product_pos(p), genomic_pos(g), row(r), length(l)
        {} 

        string AsString(int row_pos) const;
    };

    /// constructor
    CSeq_align(void);
    /// destructor
    ~CSeq_align(void);

    /// Validatiors
    TDim CheckNumRows(void)                   const;
    void Validate    (bool full_test = false) const;

    /// GetSeqRange
    /// NB: On a Spliced-seg, in case the product-type is protein,
    /// these only return the amin part of Prot-pos.  The frame is
    /// ignored.
    CRange<TSeqPos> GetSeqRange(TDim row) const;
    TSeqPos         GetSeqStart(TDim row) const;
    TSeqPos         GetSeqStop (TDim row) const;

    /// Get strand (the first one if segments have different strands).
    ENa_strand      GetSeqStrand(TDim row) const;

    /// Get seq-id (the first one if segments have different ids).
    /// Throw exception if row is invalid.
    const CSeq_id&  GetSeq_id(TDim row) const;

    /// Retrieves the total number of gaps in the given row an alignment;
    /// all gaps by default
    /// @throws CSeqalignException if alignment type is not supported
    TSeqPos         GetTotalGapCount(TDim row = -1) const;
    TSeqPos         GetTotalGapCountWithinRange(const TSeqRange &range,
                                                TDim row = -1) const;
    TSeqPos         GetTotalGapCountWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                                 TDim row = -1) const;

    /// Retrieves the number of gap openings in a given row in an alignment
    /// (ignoring how many gaps are in the gapped region); all gaps by default
    /// @throws CSeqalignException if alignment type is not supported
    TSeqPos         GetNumGapOpenings(TDim row = -1) const;
    TSeqPos         GetNumGapOpeningsWithinRange(const TSeqRange &range,
                                                 TDim row = -1) const;
    TSeqPos         GetNumGapOpeningsWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                                  TDim row = -1) const;

    /// Retrieves the number of times a given row shifts frames; i.e. the number
    /// of gaps with a length that is not a multiple of 3.
    /// @throws CSeqalignException if alignment type is not supported
    TSeqPos         GetNumFrameshifts(TDim row = -1) const;
    TSeqPos         GetNumFrameshiftsWithinRange(const TSeqRange &range,
                                                 TDim row = -1) const;
    TSeqPos         GetNumFrameshiftsWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                                  TDim row = -1) const;

    /// Retrieves descriptions of all indels on a given row
    /// @throws CSeqalignException if alignment type is not supported
    vector<SIndel>  GetIndels(TDim row = -1) const;
    vector<SIndel>  GetIndelsWithinRange(const TSeqRange &range,
                                              TDim row = -1) const;
    vector<SIndel>  GetIndelsWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                               TDim row = -1) const;

    /// Retrieves descriptions of all frameshifts on a given row; i.e.
    /// all gaps with a length that is not a multiple of 3.
    /// @throws CSeqalignException if alignment type is not supported
    vector<SIndel>  GetFrameshifts(TDim row = -1) const;
    vector<SIndel>  GetFrameshiftsWithinRange(const TSeqRange &range,
                                              TDim row = -1) const;
    vector<SIndel>  GetFrameshiftsWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                               TDim row = -1) const;

    /// Retrieves descriptions of all non-frameshift indels on a given row; i.e.
    /// all gaps with a length that is a multiple of 3.
    /// @throws CSeqalignException if alignment type is not supported
    vector<SIndel>  GetNonFrameshifts(TDim row = -1) const;
    vector<SIndel>  GetNonFrameshiftsWithinRange(const TSeqRange &range,
                                              TDim row = -1) const;
    vector<SIndel>  GetNonFrameshiftsWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                               TDim row = -1) const;

    /// Retrieves the locations of aligned bases in the given row, excluding
    /// gaps and incontinuities
    /// @throws CSeqalignException if alignment type is not supported
    CRangeCollection<TSeqPos> GetAlignedBases(TDim row) const;

    /// Get the length of this alignment.  This length corresponds to the score
    /// 'align_length'.  By default, this function computes an alignment length
    /// including all gap segments.
    TSeqPos         GetAlignLength(bool include_gaps = true) const;

    /// Get the length of this alignment within a specified range
    /// By default, this function computes an alignment length
    /// including all gap segments.
    TSeqPos         GetAlignLengthWithinRange(const TSeqRange &range,
                                              bool include_gaps = true) const;

    /// Get the length of this alignment within a specified range
    /// By default, this function computes an alignment length
    /// including all gap segments.
    TSeqPos         GetAlignLengthWithinRanges(const CRangeCollection<TSeqPos> &ranges,
                                               bool include_gaps = true) const;

    double          AlignLengthRatio() const;

    /// Get score
    bool GetNamedScore(const string& id, int &score) const;
    bool GetNamedScore(const string& id, double &score) const;

    bool GetNamedScore(EScoreType type, int &score) const;
    bool GetNamedScore(EScoreType type, double &score) const;

    void SetNamedScore(const string& id, int score);
    void SetNamedScore(const string& id, double score);

    void SetNamedScore(EScoreType type, int score);
    void SetNamedScore(EScoreType type, double score);

    void ResetNamedScore(const string& name);
    void ResetNamedScore(EScoreType    type);


    /// Reverse the segments' orientation
    /// NOTE: currently *only* works for dense-seg
    void Reverse(void);

    /// Swap the position of two rows in the alignment
    /// NOTE: currently *only* works for dense-seg & disc
    void SwapRows(TDim row1, TDim row2);

    /// Create a Dense-seg from a Std-seg
    /// Used by AlnMgr to handle nucl2prot alignments
    //

    /// NOTE: Here we assume that the same rows on different segments
    /// contain the same sequence. Without access to OM we can only check
    /// if the ids are the same via SerialEquals, and we throw an exception
    /// if not equal. Since the same sequence can be represented with a 
    /// different type of seq-id, we provide an optional callback mechanism
    /// to compare id1 and id2, and if both resolve to the same sequence 
    /// and id2 is preferred, to SerialAssign it to id1. Otherwise, again,
    /// an exception should be thrown.
    struct SSeqIdChooser : CObject
    {
        virtual void ChooseSeqId(CSeq_id& id1, const CSeq_id& id2) = 0;
    };
    CRef<CSeq_align> CreateDensegFromStdseg(SSeqIdChooser* SeqIdChooser = 0) const;

    CRef<CSeq_align> CreateDensegFromDisc(SSeqIdChooser* SeqIdChooser = 0) const;

    /// Create a Dense-seg with widths from Dense-seg of nucleotides
    /// Used by AlnMgr to handle translated nucl2nucl alignments
    /// IMPORTANT NOTE: Do *NOT* use for alignments containing proteins;
    ///                 the code will not check for this
    CRef<CSeq_align> CreateTranslatedDensegFromNADenseg(void) const;

    /// Split the alignment at any discontinuity greater than threshold;
    /// populate aligns list with new alignments. If alignment contains no long
    /// discontinuities, populate aligns list with a singleton reference
    /// to self.
    /// Splitting works only for Denseg and Disc; for all other segment types, this
    /// function simply populates aligns with a singleton reference to self.
    /// This function is only implemented for pairwise alignments; it throws an
    /// exception if Dim is more than 2.
    void SplitOnLongDiscontinuity(list< CRef<CSeq_align> >& aligns,
                                  TSeqPos discontinuity_threshold) const;


    /// Offset row's coords
    void OffsetRow(TDim row, TSignedSeqPos offset);

    /// @deprecated (use sequence::RemapAlignToLoc())
    /// @sa RemapAlignToLoc
    NCBI_DEPRECATED void RemapToLoc(TDim row,
                                    const CSeq_loc& dst_loc,
                                    bool ignore_strand = false);

    CRef<CSeq_loc> CreateRowSeq_loc(TDim row) const;

    TLengthRange GapLengthRange() const;

    TLengthRange IntronLengthRange() const;

    TLengthRange ExonLengthRange() const;

    /// Find extension by type in ext container.
    /// @param ext_type
    ///   String id of the extension to find.
    /// @result
    ///   User-object of the requested type or NULL.
    CConstRef<CUser_object> FindExt(const string& ext_type) const;
    /// Non-const version of FindExt().
    CRef<CUser_object> FindExt(const string& ext_type);

    static const TScoreNameMap &ScoreNameMap();

    static string ScoreName(EScoreType score);

    static string HelpText(EScoreType score);

    static bool IsIntegerScore(EScoreType score);

protected:
    /// retrieve a named score object
    CConstRef<CScore> x_GetNamedScore(const string& name) const;
    CRef<CScore>      x_SetNamedScore(const string& name);

private:

    /// Prohibit copy constructor and assignment operator
    CSeq_align(const CSeq_align& value);
    CSeq_align& operator=(const CSeq_align& value);

    /// Create a partial alignment containing the specified range of segments.
    /// Can only be called for Denseg alignments.
    CRef<CSeq_align>  x_CreateSubsegAlignment(int from, int to) const;
};


/// Remap seq-align row to the seq-loc.
/// Treats the given row as being relative to the location, maps it
/// to the sequence(s) referenced by this location.
/// @param align
///   The seq-align object to be mapped (the object will be modified!).
/// @param row
///   Row to be mapped.
/// @param loc
///   Seq-loc to which the row should be mapped.
/// @result
///   Reference to the new seq-align with the mapped row.
NCBI_SEQALIGN_EXPORT
CRef<CSeq_align> RemapAlignToLoc(const CSeq_align& align,
                                 CSeq_align::TDim  row,
                                 const CSeq_loc&   loc);


/////////////////// CSeq_align inline methods

// constructor
inline
CSeq_align::CSeq_align(void)
{
}


/////////////////// end of CSeq_align inline methods


END_objects_SCOPE /// namespace ncbi::objects::

END_NCBI_SCOPE

#endif /// OBJECTS_SEQALIGN_SEQ_ALIGN_HPP
