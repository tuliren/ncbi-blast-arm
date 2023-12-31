/* $Id: PubmedBookArticle.hpp 650415 2022-05-24 14:31:50Z grichenk $
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

/// @file PubmedBookArticle.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'efetch.xsd'.
///
/// New methods or data members can be added to it if needed.
/// See also: PubmedBookArticle_.hpp


#ifndef eutils__OBJTOOLS_EUTILS_EFETCH_PUBMEDBOOKARTICLE_HPP
#define eutils__OBJTOOLS_EUTILS_EFETCH_PUBMEDBOOKARTICLE_HPP

#include <objects/pubmed/Pubmed_entry.hpp>

// generated includes
#include <objtools/eutils/efetch/PubmedBookArticle_.hpp>

// generated classes

BEGIN_eutils_SCOPE // namespace eutils::

/////////////////////////////////////////////////////////////////////////////
class CPubmedBookArticle : public CPubmedBookArticle_Base
{
    typedef CPubmedBookArticle_Base Tparent;
public:
    // constructor
    CPubmedBookArticle(void);
    // destructor
    ~CPubmedBookArticle(void);

    ncbi::CRef<ncbi::objects::CPubmed_entry> ToPubmed_entry(void) const;

private:
    // Prohibit copy constructor and assignment operator
    CPubmedBookArticle(const CPubmedBookArticle& value);
    CPubmedBookArticle& operator=(const CPubmedBookArticle& value);

};

/////////////////// CPubmedBookArticle inline methods

// constructor
inline
CPubmedBookArticle::CPubmedBookArticle(void)
{
}


/////////////////// end of CPubmedBookArticle inline methods


END_eutils_SCOPE // namespace eutils::


#endif // eutils__OBJTOOLS_EUTILS_EFETCH_PUBMEDBOOKARTICLE_HPP
/* Original file checksum: lines: 82, chars: 2553, CRC32: 6b45b3e6 */
