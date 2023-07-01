#ifndef UTIL___SGML_ENTITY__HPP
#define UTIL___SGML_ENTITY__HPP

/*  $Id: sgml_entity.hpp 164824 2009-07-01 15:29:16Z bollin $
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
 * Authors:  Mati Shomrat
 *
 * File Description:
 *   Functions to Convert SGML to ASCII for Backbone subset SGML
 */
#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE


/// Convert SGML entity to ASCII (in place)
/// @param sgml
///   String contianing SGML entities
void NCBI_XUTIL_EXPORT Sgml2Ascii(string& sgml);


/// Convert SGML entity to ASCII
/// @param sgml
///   String contianing SGML entities
/// @return
///   string with SGML entities converted to ASCII
string NCBI_XUTIL_EXPORT Sgml2Ascii(const string& sgml);

bool NCBI_XUTIL_EXPORT ContainsSgml(const string& str);


END_NCBI_SCOPE

#endif  // UTIL___SGML_ENTITY__HPP
