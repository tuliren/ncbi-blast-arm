/*  $Id: struct_cmt_reader.cpp 652899 2022-07-18 19:02:48Z ludwigf $
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
* Author:  Sergiy Gotvyanskyy, NCBI
*
* File Description:
*   Reader for structured comments for sequences
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/seqloc/Giimport_id.hpp>
#include <objects/biblio/Id_pat.hpp>
#include <objects/seqloc/Patent_seq_id.hpp>
#include <objects/seqloc/PDB_seq_id.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_field.hpp>

#include <util/line_reader.hpp>
#include <objtools/readers/struct_cmt_reader.hpp>


#include <common/test_assert.h>  /* This header must go last */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CStructuredCommentsReader::CStructuredCommentsReader(ILineErrorListener* logger) : m_logger(logger)
{
}

CStructuredCommentsReader::~CStructuredCommentsReader()
{
}

const string& CStructuredCommentsReader::CStructComment::GetPrefix(const objects::CSeqdesc& desc)
{
    if (!desc.IsUser())
        return kEmptyStr;

    auto& user = desc.GetUser();
    if (user.IsSetType() && user.GetType().IsStr() && NStr::Equal(user.GetType().GetStr(), "StructuredComment"))
    {
        if (user.IsSetData() && user.GetData().size() > 0)
        {
            const auto& fdata = *user.GetData().front();
            if (fdata.IsSetLabel() && fdata.GetLabel().IsStr() && NStr::Equal(fdata.GetLabel().GetStr(), "StructuredCommentPrefix"))
                return fdata.GetData().GetStr();
        }
    }

    return kEmptyStr;
}

objects::CUser_object* CStructuredCommentsReader::_AddStructuredComment(objects::CUser_object* user_obj, CStructComment& cmt, const CTempString& name, const CTempString& value)
{
    if (name.compare("StructuredCommentPrefix") == 0)
        user_obj = 0; // reset user obj so to create a new one

    if (user_obj == 0)
    {
        // create new user object
        CRef<CSeqdesc> user_desc(new CSeqdesc);
        user_obj = &(user_desc->SetUser());
        user_obj->SetType().SetStr("StructuredComment");
        cmt.m_descs.push_back(user_desc);
    }
    user_obj->AddField(name, value);
    // signal to create next user object
    if (name.compare("StructuredCommentSuffix") == 0)
        return 0;
    else
        return user_obj;
}

void CStructuredCommentsReader::_BuildStructuredComment(CStructComment& cmt, const vector<string>& cols, const vector<CTempString>& values)
{
    cmt.m_descs.reserve(values.size() - 1);
    objects::CUser_object* user = 0;

    for (size_t i = 1; i<values.size(); i++)
    {
        if (!values[i].empty())
        {
            // create new user object
            user = _AddStructuredComment(user, cmt, cols[i], values[i]);
        }
    }
}

void CStructuredCommentsReader::_LoadHeaderLine(ILineReader& reader, vector<string>& cols)
{
    cols.clear();

    while (!reader.AtEOF() && cols.empty())
    {
        reader.ReadLine();
        // First line is a collumn definitions
        CTempString current = reader.GetCurrentLine();
        if (NStr::StartsWith(current, '#'))
            continue;

        NStr::Split(current, "\t", cols);
    }
}

bool CStructuredCommentsReader::SeqIdMatchesCommentId(
    const CSeq_id& seqID, const CSeq_id& commentID)
{
    // idea: try match the raw text of the commentID with the "money" part of the given ID

    if (seqID.Compare(commentID) == CSeq_id::e_YES) {
        return true;
    }
    if (!commentID.IsLocal()) {
        return false;
    }

    const auto& commentIdText = commentID.GetLocal().GetStr();
    const CTextseq_id* pTsid = seqID.GetTextseq_Id();
    if (pTsid) {
        if (pTsid->IsSetAccession()) {
            return (pTsid->GetAccession() == commentIdText);
        }
        if (pTsid->IsSetName()) {
            return (pTsid->GetName() == commentIdText);
        }
        return false;
    }

    string seqIdText;
    switch (seqID.Which()) {
    default:
        return false;
    case CSeq_id::e_Gibbsq:
        seqIdText = NStr::IntToString(seqID.GetGibbsq());
        break;
    case CSeq_id::e_Gibbmt:
        seqIdText = NStr::IntToString(seqID.GetGibbmt());
        break;
    case CSeq_id::e_Giim:
        seqIdText = NStr::IntToString(seqID.GetGiim().GetId());
        break;
    case CSeq_id::e_General: {
        const auto& general = seqID.GetGeneral();
        if (general.IsSetTag()) {
            if (general.GetTag().IsStr()) {
                seqIdText = general.GetTag().GetStr();
            }
            else {
                seqIdText = NStr::IntToString(general.GetTag().GetId());
            }
        }
        break;
    }
    case CSeq_id::e_Patent: {
        const CId_pat& idp = seqID.GetPatent().GetCit();
        seqIdText = idp.GetId().IsNumber() ?
            idp.GetId().GetNumber() : idp.GetId().GetApp_number();
        seqIdText += '_';
        seqIdText += NStr::IntToString(seqID.GetPatent().GetSeqid());
        break;
    }
    case CSeq_id::e_Gi:
        seqIdText = NStr::NumericToString(seqID.GetGi());
        break;
    case CSeq_id::e_Pdb: {
        const CPDB_seq_id& pid = seqID.GetPdb();
        seqIdText = pid.GetMol().Get();
        if (pid.IsSetChain_id()) {
            seqIdText += '_';
            seqIdText += pid.GetChain_id();
        }
        else if (pid.IsSetChain()) {
            unsigned char chain = static_cast<unsigned char>(pid.GetChain());
            if (chain > ' ') {
                seqIdText += '_';
                seqIdText += static_cast<char>(chain);
            }
        }
        break;
    }
    }
    return (seqIdText == commentIdText);
}




END_NCBI_SCOPE
