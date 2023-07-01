/* $Id: Seq_graph.hpp 349035 2012-01-06 18:32:21Z vasilche $
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
 */

/// @file Seq_graph.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'seqres.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: Seq_graph_.hpp


#ifndef OBJECTS_SEQRES_SEQ_GRAPH_HPP
#define OBJECTS_SEQRES_SEQ_GRAPH_HPP


// generated includes
#include <objects/seqres/Seq_graph_.hpp>

#include <serial/objhook.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class NCBI_SEQRES_EXPORT CSeq_graph : public CSeq_graph_Base
{
    typedef CSeq_graph_Base Tparent;
public:
    // constructor
    CSeq_graph(void);
    // destructor
    ~CSeq_graph(void);

    // reserve memory for data vectors
    class NCBI_SEQRES_EXPORT CReserveHook : public CPreReadChoiceVariantHook
    {
        virtual void PreReadChoiceVariant(CObjectIStream& in,
                                          const CObjectInfoCV& variant);
    };

private:
    // Prohibit copy constructor and assignment operator
    CSeq_graph(const CSeq_graph& value);
    CSeq_graph& operator=(const CSeq_graph& value);

};

/////////////////// CSeq_graph inline methods

// constructor
inline
CSeq_graph::CSeq_graph(void)
{
}


/////////////////// end of CSeq_graph inline methods


NCBISER_HAVE_GLOBAL_READ_VARIANT_HOOK(CSeq_graph, "graph.*",
                                      new CSeq_graph::CReserveHook)

END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_SEQRES_SEQ_GRAPH_HPP
