/* $Id: Author.hpp 642756 2021-12-28 18:41:52Z stakhovv $
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

/// @Author.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'biblio.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: Author_.hpp


#ifndef OBJECTS_BIBLIO_AUTHOR_HPP
#define OBJECTS_BIBLIO_AUTHOR_HPP


// generated includes
#include <objects/biblio/Author_.hpp>
#include <objects/biblio/Auth_list.hpp>

#include <objects/biblio/citation_base.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class CPerson_id;

/////////////////////////////////////////////////////////////////////////////
class NCBI_BIBLIO_EXPORT CAuthor : public CAuthor_Base, public ICitationBase
{
    typedef CAuthor_Base Tparent;
public:
    // constructor
    CAuthor() {}
    // destructor
    ~CAuthor();

    static CRef<CAuthor> ConvertMlToStandard(const CAuthor& author,
                                    bool normalize_suffix=false);
    static CRef<CAuthor> ConvertMlToStandard(const string& ml_name,
                                    bool normalize_suffix=false);
protected:
    bool GetLabelV1(string* label, TLabelFlags flags) const override;
    bool GetLabelV2(string* label, TLabelFlags flags) const override;

private:
    // Prohibit copy constructor and assignment operator
    CAuthor(const CAuthor& value);
    CAuthor& operator=(const CAuthor& value);

    static bool x_GetLabelV2(string* label, TLabelFlags flags, CTempString name,
                             CTempString initials = kEmptyStr,
                             CTempString suffix = kEmptyStr);

    static CRef<CPerson_id> x_ConvertMlToStandard(const string& name,
                                                  bool normalize_suffix=false);

    friend class CAuth_list;
};


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_BIBLIO_AUTHOR_HPP
/* Original file checksum: lines: 94, chars: 2515, CRC32: d2aa87bb */
