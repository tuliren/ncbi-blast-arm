/*  $Id: gb_block_field.hpp 638615 2021-10-02 09:47:34Z stakhovv $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#ifndef _GB_BLOCK_FIELD_H_
#define _GB_BLOCK_FIELD_H_

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>

#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/Seq_feat.hpp>

#include <objtools/edit/text_desc_field.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
BEGIN_SCOPE(edit)


class NCBI_XOBJEDIT_EXPORT CGBBlockField : public CTextDescriptorField
{
public:
    enum EGBBlockFieldType {
        eGBBlockFieldType_Keyword = 0,
        eGBBlockFieldType_ExtraAccession,
        eGBBlockFieldType_Unknown };

    CGBBlockField(EGBBlockFieldType field_type = eGBBlockFieldType_Keyword)
        : m_FieldType(field_type),
          m_StringConstraint() { m_Subtype = CSeqdesc::e_Genbank; }
    string GetVal(const CObject& object) override;
    vector<string> GetVals(const CObject& object) override;
    bool IsEmpty(const CObject& object) const override;
    void ClearVal(CObject& object) override;
    bool SetVal(CObject& object, const string& val, EExistingText existing_text) override;
    string IsValid(const string& value) override { return string(); }
    vector<string> IsValid(const vector<string>& values) override { return vector<string>(); }
    void SetConstraint(const string& field, CConstRef<CStringConstraint> string_constraint) override;
    bool AllowMultipleValues() override;

    static EGBBlockFieldType GetTypeForLabel(string label);
    static string GetLabelForType(EGBBlockFieldType field_type);

protected:
    EGBBlockFieldType m_FieldType;
    CRef<CStringConstraint> m_StringConstraint;
};


END_SCOPE(edit)
END_SCOPE(objects)
END_NCBI_SCOPE

#endif

