/* $Id: Auth_list.cpp 642756 2021-12-28 18:41:52Z stakhovv $
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
 *   using the following specifications:
 *   'biblio.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

// generated includes
#include <objects/biblio/Auth_list.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CAuth_list::~CAuth_list()
{
}


size_t CAuth_list::GetNameCount() const
{
    switch (GetNames().Which()) {
    case TNames::e_not_set:
        return 0;
    case TNames::e_Std:
        return GetNames().GetStd().size();
    case TNames::e_Ml:
        return GetNames().GetMl().size();
    case TNames::e_Str:
        return GetNames().GetStr().size();
    }
    return 0;
}


bool CAuth_list::GetLabelV1(string* label, TLabelFlags flags) const
{
    const C_Names& names = GetNames();
    switch (names.Which()) {
    case C_Names::e_not_set:
        break;
    case C_Names::e_Std:
        if (!names.GetStd().empty()) {
            return names.GetStd().front()->GetLabel(label, flags, eLabel_V1);
        }
        break;
    case C_Names::e_Ml:
        if (!names.GetMl().empty()) {
            *label += names.GetMl().front();
            return true;
        }
        break;
    case C_Names::e_Str:
        if (!names.GetStr().empty()) {
            *label += names.GetStr().front();
            return true;
        }
        break;
    }

    return false;
}


bool CAuth_list::GetLabelV2(string* label, TLabelFlags flags) const
{
    const C_Names& names = GetNames();
    string         prefix;
    unsigned int   count = 0;

    switch (names.Which()) {
    case C_Names::e_not_set:
        return false;

    case C_Names::e_Std:
    {
        C_Names::TStd individuals;
        for (const CRef<CAuthor>& it : names.GetStd()) {
            switch (it->GetName().Which()) {
            case CPerson_id::e_Name:
            case CPerson_id::e_Ml:
            case CPerson_id::e_Str:
                if ((flags & fLabel_Consortia) == 0) {
                    individuals.push_back(it);
                }
                break;
            case CPerson_id::e_Consortium:
                if ((flags & fLabel_Consortia) != 0) {
                    if (it->GetLabel(label, flags, eLabel_V2)) {
                        ++count;
                    }
                    prefix = "; ";
                }
                break;
            default:
                break;
            }
        }

        if ((flags & fLabel_Consortia) == 0) {
            ITERATE (C_Names::TStd, it, individuals) {
                if (count > 0) {
                    if (&*it == &individuals.back()
                        &&  (flags & fLabel_FlatNCBI) != 0) {
                        prefix = " and ";
                    } else {
                        prefix = ", ";
                    }
                }
                *label += prefix;
                if ((*it)->GetLabel(label, flags, eLabel_V2)) {
                    ++count;
                } else if (NStr::EndsWith(*label, prefix)) { // It should!
                    label->resize(label->size() - prefix.size());
                }
            }
        }

        break;
    }

    case C_Names::e_Ml:
    case C_Names::e_Str:
        if ((flags & fLabel_Consortia) == 0) {
            C_Names::TMl nl = names.IsMl() ? names.GetMl() : names.GetStr();
            ITERATE (C_Names::TMl, it, nl) {
                if (count > 0) {
                    if (&*it == &nl.back() && (flags & fLabel_FlatNCBI) != 0) {
                        prefix = " and ";
                    } else {
                        prefix = ", ";
                    }
                }
                *label += prefix;
                if (CAuthor::x_GetLabelV2(label, flags, *it)) {
                    ++count;
                } else if (NStr::EndsWith(*label, prefix)) { // It should!
                    label->resize(label->size() - prefix.size());
                }
            }
        }
        break;
    }

    return count > 0;
}


void CAuth_list::ConvertMlToStandard(bool normalize_suffix)
{
    if (!IsSetNames() || !GetNames().IsMl()) {
        return;
    }

    list<CRef<CAuthor>> standard_names;
    for (const string& author_ml_str : GetNames().GetMl()) {
        if (!NStr::IsBlank(author_ml_str)) {
            CRef<CAuthor> new_auth = CAuthor::ConvertMlToStandard(author_ml_str, normalize_suffix);
            standard_names.push_back(new_auth);
        }
    }
    SetNames().Reset();
    SetNames().SetStd().insert(SetNames().SetStd().begin(), standard_names.cbegin(), standard_names.cend());
}

void CAuth_list::ConvertMlToStd(bool normalize_suffix)
{
    if (IsSetNames()) {
        if (GetNames().IsMl()) {
            ConvertMlToStandard(normalize_suffix);
        } else if (GetNames().IsStd()) {
            for (CRef<CAuthor>& auth : SetNames().SetStd()) {
                if (auth->IsSetName() && auth->GetName().IsMl()) {
                    auth = CAuthor::ConvertMlToStandard(*auth, normalize_suffix);
                }
            }
        }
    }
}

string s_GetAuthorMatchString(const CAuthor& auth)
{
    string comp;
    if (!auth.IsSetName()) {
        return comp;
    }

    if (auth.GetName().IsName()) {
        if (auth.GetName().GetName().IsSetLast()) {
            comp = auth.GetName().GetName().GetLast();
        }
    } else if (auth.GetName().IsConsortium()) {
        comp = auth.GetName().GetConsortium();
    } else if (auth.GetName().IsStr()) {
        comp = auth.GetName().GetStr();
    }
    return comp;
}


bool s_AuthorMatch(const CAuthor& auth1, const CAuthor& auth2)
{
    string comp1 = s_GetAuthorMatchString(auth1);
    string comp2 = s_GetAuthorMatchString(auth2);
    return NStr::EqualNocase(comp1, comp2);
}


vector<string> GetAuthorMatchStrings(const CAuth_list::TNames& names)
{
    vector<string> list;

    if (names.IsStd()) {
        for (const CRef<CAuthor>& it : names.GetStd()) {
            list.push_back(s_GetAuthorMatchString(*it));
        }
    } else if (names.IsStr()) {
        for (const string& it : names.GetStr()) {
            list.push_back(it);
        }
    }
    return list;
}


bool CAuth_list::SameCitation(const CAuth_list& other) const
{
    if (!IsSetNames() && !other.IsSetNames()) {
        return true;
    } else if (!IsSetNames() || !other.IsSetNames()) {
        return false;
    } else if (GetNames().Which() == TNames::e_not_set &&
               other.GetNames().Which() == TNames::e_not_set) {
        return true;
    } else if (GetNames().Which() != TNames::e_Std && 
               GetNames().Which() != TNames::e_Str) {
        return false;
    } else if (other.GetNames().Which() != TNames::e_Std &&
               other.GetNames().Which() != TNames::e_Str) {
        return false;
    }

    bool match = true;
    const vector<string> match_str1 = GetAuthorMatchStrings(GetNames());
    const vector<string> match_str2 = GetAuthorMatchStrings(other.GetNames());

    vector<string>::const_iterator it1 = match_str1.begin();
    vector<string>::const_iterator it2 = match_str2.begin();
    while (it1 != match_str1.end() && it2 != match_str2.end()) {
        if (!NStr::EqualNocase(*it1, *it2)) {
            match = false;
        }
        it1++;
        it2++;
    }
    if (it1 != match_str1.end() || it2 != match_str2.end()) {
        match = false;
    }

    return match;
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 65, chars: 1889, CRC32: d99a2868 */
