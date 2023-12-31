/* $Id: Cit_pat.hpp 642610 2021-12-23 13:49:47Z stakhovv $
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
 *   using specifications from the ASN data definition file
 *   'biblio.asn'.
 */

#ifndef OBJECTS_BIBLIO_CIT_PAT_HPP
#define OBJECTS_BIBLIO_CIT_PAT_HPP


// generated includes
#include <objects/biblio/Cit_pat_.hpp>

#include <objects/biblio/citation_base.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class NCBI_BIBLIO_EXPORT CCit_pat : public CCit_pat_Base, public ICitationBase
{
    typedef CCit_pat_Base Tparent;
public:
    // constructor
    CCit_pat(void);
    // destructor
    ~CCit_pat(void);
    
protected:    
    // Appends a label onto "label" based on content
    bool GetLabelV1(string* label, TLabelFlags flags) const override;
    bool GetLabelV2(string* label, TLabelFlags flags) const override;

private:
    // Prohibit copy constructor and assignment operator
    CCit_pat(const CCit_pat& value);
    CCit_pat& operator=(const CCit_pat& value);

    static bool x_GetLabelV2(string* label, const CAuth_list& authors,
                             string prefix);
};



/////////////////// CCit_pat inline methods

// constructor
inline
CCit_pat::CCit_pat(void)
{
}


/////////////////// end of CCit_pat inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_BIBLIO_CIT_PAT_HPP
/* Original file checksum: lines: 90, chars: 2366, CRC32: eacba6b0 */
