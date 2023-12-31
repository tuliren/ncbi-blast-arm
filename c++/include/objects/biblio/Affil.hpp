/* $Id: Affil.hpp 642610 2021-12-23 13:49:47Z stakhovv $
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

/// @Affil.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'biblio.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: Affil_.hpp


#ifndef OBJECTS_BIBLIO_AFFIL_HPP
#define OBJECTS_BIBLIO_AFFIL_HPP


// generated includes
#include <objects/biblio/Affil_.hpp>

#include <objects/biblio/citation_base.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class NCBI_BIBLIO_EXPORT CAffil : public CAffil_Base, public ICitationBase
{
    typedef CAffil_Base Tparent;
public:
    // constructor
    CAffil(void);
    // destructor
    ~CAffil(void);

protected:
    bool GetLabelV1(string* label, TLabelFlags flags) const override;
    bool GetLabelV2(string* label, TLabelFlags flags) const override;

private:
    // Prohibit copy constructor and assignment operator
    CAffil(const CAffil& value);
    CAffil& operator=(const CAffil& value);

};

/////////////////// CAffil inline methods

// constructor
inline
CAffil::CAffil(void)
{
}


/////////////////// end of CAffil inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_BIBLIO_AFFIL_HPP
/* Original file checksum: lines: 94, chars: 2496, CRC32: d8bc16d5 */
