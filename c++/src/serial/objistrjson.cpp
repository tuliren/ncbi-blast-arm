/*  $Id: objistrjson.cpp 651126 2022-06-10 12:57:23Z ludwigf $
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
* Author: Andrei Gourianov
*
* File Description:
*   JSON object input stream
*
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbi_limits.h>

#include <serial/objistrjson.hpp>

#define NCBI_USE_ERRCODE_X   Serial_OStream

BEGIN_NCBI_SCOPE

CObjectIStream* CObjectIStream::CreateObjectIStreamJson()
{
    return new CObjectIStreamJson();
}


CObjectIStreamJson::CObjectIStreamJson(void)
    : CObjectIStream(eSerial_Json),
    m_FileHeader(false),
    m_BlockStart(false),
    m_ExpectValue(false),
    m_GotNameless(false),
    m_Closing(0),
    m_StringEncoding( eEncoding_UTF8 ),
    m_BinaryFormat(eDefault)
{
    m_Utf8Pos = m_Utf8Buf.begin();
}

CObjectIStreamJson::CObjectIStreamJson(CNcbiIstream& in, EOwnership deleteIn)
    : CObjectIStream(eSerial_Json),
    m_FileHeader(false),
    m_BlockStart(false),
    m_ExpectValue(false),
    m_GotNameless(false),
    m_Closing(0),
    m_StringEncoding( eEncoding_UTF8 ),
    m_BinaryFormat(eDefault)
{
    m_Utf8Pos = m_Utf8Buf.begin();
    Open(in, deleteIn);
}

CObjectIStreamJson::~CObjectIStreamJson(void)
{
}

void CObjectIStreamJson::ResetState(void)
{
    CObjectIStream::ResetState();
    if (GetStackDepth() > 1) {
        return;
    }
    m_LastTag.clear();
    m_RejectedTag.clear();
    m_Utf8Buf.clear();
    m_Utf8Pos = m_Utf8Buf.begin();
}

bool CObjectIStreamJson::EndOfData(void)
{
    if (CObjectIStream::EndOfData()) {
        return true;
    }
    try {
        SkipWhiteSpace();
    } catch (...) {
        return true;
    }
    return false;
}

void CObjectIStreamJson::Location(string& loc_type, size_t& loc) const
{
    loc_type = "line";
    loc = m_Input.GetLine();
}

void CObjectIStreamJson::SetDefaultStringEncoding(EEncoding enc)
{
    m_StringEncoding = enc;
}

EEncoding CObjectIStreamJson::GetDefaultStringEncoding(void) const
{
    return m_StringEncoding;
}

CObjectIStreamJson::EBinaryDataFormat
CObjectIStreamJson::GetBinaryDataFormat(void) const
{
    return m_BinaryFormat;
}
void CObjectIStreamJson::SetBinaryDataFormat(CObjectIStreamJson::EBinaryDataFormat fmt)
{
    m_BinaryFormat = fmt;
}

char CObjectIStreamJson::GetChar(void)
{
    return m_Input.GetChar();
}

char CObjectIStreamJson::PeekChar(void)
{
    return m_Input.PeekChar();
}

char CObjectIStreamJson::GetChar(bool skipWhiteSpace)
{
    return skipWhiteSpace? SkipWhiteSpaceAndGetChar(): m_Input.GetChar();
}

char CObjectIStreamJson::PeekChar(bool skipWhiteSpace)
{
    return skipWhiteSpace? SkipWhiteSpace(): m_Input.PeekChar();
}

void CObjectIStreamJson::SkipEndOfLine(char c)
{
    m_Input.SkipEndOfLine(c);
}

char CObjectIStreamJson::SkipWhiteSpace(void)
{
    try { // catch CEofException
        for ( ;; ) {
            char c = m_Input.SkipSpaces();
            switch ( c ) {
            case '\t':
                m_Input.SkipChar();
                continue;
            case '\r':
            case '\n':
                m_Input.SkipChar();
                SkipEndOfLine(c);
                continue;
            default:
                return c;
            }
        }
    } catch (CEofException& e) {
        ThrowError(fEOF, e.what());
    }
    return '\0';
}

char CObjectIStreamJson::SkipWhiteSpaceAndGetChar(void)
{
    char c = SkipWhiteSpace();
    m_Input.SkipChar();
    return c;
}

bool CObjectIStreamJson::GetChar(char expect, bool skipWhiteSpace /* = false*/)
{
    if ( PeekChar(skipWhiteSpace) != expect ) {
        return false;
    }
    m_Input.SkipChar();
    return true;
}

void CObjectIStreamJson::Expect(char expect, bool skipWhiteSpace /* = false*/)
{
    if ( !GetChar(expect, skipWhiteSpace) ) {
        string msg("\'");
        msg += expect;
        msg += "' expected";
        ThrowError(fFormatError, msg);
    }
}

void CObjectIStreamJson::UnexpectedMember(const CTempString& id,
                                          const CItemsInfo& items)
{
    string message =
        "\""+string(id)+"\": unexpected member, should be one of: ";
    for ( CItemsInfo::CIterator i(items); i.Valid(); ++i ) {
        message += '\"' + items.GetItemInfo(i)->GetId().ToString() + "\" ";
    }
    ThrowError(fFormatError, message);
}

template<typename Type> inline
Type CObjectIStreamJson::x_UseMemberDefault(void)
{
    return GetMemberDefault() ? CTypeConverter<Type>::Get(GetMemberDefault()) : Type();
}

int CObjectIStreamJson::ReadEscapedChar(bool* encoded /*=0*/)
{
    char c = GetChar();
    if (c == '\\') {
        if (encoded) {
            *encoded = true;
        }
        c = GetChar();
        if (c == 'u') {
            int v = 0;
            for (int p=0; p<4; ++p) {
                c = GetChar();
                if (c >= '0' && c <= '9') {
                    v = v * 16 + (c - '0');
                } else if (c >= 'A' && c <= 'F') {
                    v = v * 16 + (c - 'A' + 0xA);
                } else if (c >= 'a' && c <= 'f') {
                    v = v * 16 + (c - 'a' + 0xA);
                } else {
                    ThrowError(fFormatError,
                        "invalid symbol in escape sequence");
                }
            }
            return v;
        }
    } else {
        if (encoded) {
            *encoded = false;
        }
    }
    return c & 0xFF;
}

char CObjectIStreamJson::ReadEncodedChar(EStringType type, bool& encoded)
{
    EEncoding enc_out( type == eStringTypeUTF8 ? eEncoding_UTF8 : m_StringEncoding);
    EEncoding enc_in(eEncoding_UTF8);

    if (enc_out == eEncoding_UTF8 &&
        !m_Utf8Buf.empty() && m_Utf8Pos != m_Utf8Buf.end()) {
        if (++m_Utf8Pos != m_Utf8Buf.end()) {
            return char(*m_Utf8Pos & 0xFF);
        } else {
            m_Utf8Buf.clear();
        }
    }
    int c = ReadEscapedChar(&encoded);
    if (enc_out != eEncoding_Unknown) {
        if (encoded) {
            TUnicodeSymbol chU = c;
            if (enc_out == eEncoding_UTF8) {
                m_Utf8Buf = CUtf8::AsUTF8( &chU, 1);
                m_Utf8Pos = m_Utf8Buf.begin();
                return char(*m_Utf8Pos & 0xFF);
            } else {
                return CUtf8::SymbolToChar( chU, enc_out);
            }
        }
        if (enc_in != enc_out) {
            if (enc_out != eEncoding_UTF8) {
                TUnicodeSymbol chU = enc_in == eEncoding_UTF8 ?
                    ReadUtf8Char((char)c) : CUtf8::CharToSymbol((char)c, enc_in);
                Uint1 ch = CUtf8::SymbolToChar( chU, enc_out);
                return char(ch & 0xFF);
            }
            if ((c & 0x80) == 0) {
                return (char)c;
            }
            char ch = (char)c;
            m_Utf8Buf = CUtf8::AsUTF8( CTempString(&ch,1), enc_in);
            m_Utf8Pos = m_Utf8Buf.begin();
            return char(*m_Utf8Pos & 0xFF);
        }
    }
    return (char)c;
}

TUnicodeSymbol CObjectIStreamJson::ReadUtf8Char(char c)
{
    size_t more = 0;
    TUnicodeSymbol chU = CUtf8::DecodeFirst(c, more);
    while (chU && more--) {
        chU = CUtf8::DecodeNext(chU, m_Input.GetChar());
    }
    if (chU == 0) {
        ThrowError(fInvalidData, "invalid UTF8 string");
    }
    return chU;
}

string CObjectIStreamJson::x_ReadString(EStringType type)
{
    m_ExpectValue = false;
    Expect('\"',true);
    string str;
    for (;;) {
        bool encoded = false;
        char c = ReadEncodedChar(type, encoded);
        if (!encoded) {
            if (c == '\r' || c == '\n') {
                ThrowError(fFormatError, "end of line: expected '\"'");
            } else if (c == '\"') {
                break;
            }
        }
        str += char(c);
        // pre-allocate memory for long strings
        if ( str.size() > 128  &&  (double)str.capacity()/((double)str.size()+1.0) < 1.1 ) {
            str.reserve(str.size()*2);
        }
    }
    str.reserve(str.size());
    return str;
}

void CObjectIStreamJson::x_ReadData(string& str, EStringType type /*= eStringTypeVisible*/)
{
    SkipWhiteSpace();
    for (;;) {
        bool encoded = false;
        char c = ReadEncodedChar(type, encoded);
        if (!encoded && strchr(",]} \r\n", c)) {
            m_Input.UngetChar(c);
            break;
        }
        str += char(c);
        // pre-allocate memory for long strings
        if ( str.size() > 128  &&  (double)str.capacity()/((double)str.size()+1.0) < 1.1 ) {
            str.reserve(str.size()*2);
        }
    }
    str.reserve(str.size());
}

bool CObjectIStreamJson::x_ReadDataAndCheck(string& str, EStringType type)
{
    x_ReadData(str, type);
    if (str == "null") {
        if ((ExpectSpecialCase() & CObjectIStream::eReadAsNil)!=0) {
            SetSpecialCaseUsed(CObjectIStream::eReadAsNil);
            return false;
        } else {
            NCBI_THROW(CSerialException,eNullValue, kEmptyStr);
        }
    }
    return true;
}

void  CObjectIStreamJson::x_SkipData(void)
{
    m_ExpectValue = false;
    char to = GetChar(true);
    for (;;) {
        bool encoded = false;
        char c = ReadEncodedChar(eStringTypeUTF8, encoded);
        if (!encoded) {
            if (to == '\"') {
                if (c == to) {
                    break;
                }
            }
            else if (strchr(",]} \r\n", c)) {
                m_Input.UngetChar(c);
                break;
            }
        }
    }
}

string CObjectIStreamJson::ReadKey(void)
{
    if (!m_RejectedTag.empty()) {
        m_LastTag = m_RejectedTag;
        m_RejectedTag.erase();
    } else {
        SkipWhiteSpace();
        m_LastTag = x_ReadString(eStringTypeVisible);
        Expect(':', true);
        SkipWhiteSpace();
    }
    m_ExpectValue = true;
    return m_LastTag;
}

string CObjectIStreamJson::ReadValue(EStringType type /*= eStringTypeVisible*/)
{
    return x_ReadString(type);
}


void CObjectIStreamJson::StartBlock(char expect)
{
    if (expect) {
        Expect(expect, true);
    }
    m_BlockStart = true;
    m_ExpectValue = false;
}

void CObjectIStreamJson::EndBlock(char expect)
{
    if (expect) {
        Expect(expect, true);
    }
    m_BlockStart = false;
    m_ExpectValue = false;
}

bool CObjectIStreamJson::NextElement(void)
{
    if (!m_RejectedTag.empty()) {
        m_BlockStart = false;
        return true;
    }
    char c = SkipWhiteSpace();
    if ( m_BlockStart ) {
        // first element
        m_BlockStart = false;
        return c != '}' && c != ']';
    }
    else {
        // next element
        if ( c == ',' ) {
            m_Input.SkipChar();
            return true;
        }
        else if ( c != '}' && c != ']' )
            ThrowError(fFormatError, "',' or '}' or ']' expected");
        return false;
    }
}

string CObjectIStreamJson::ReadFileHeader(void)
{
    {
        char c = m_Input.PeekChar();
        if ((unsigned char)c == 0xEF) {
            if ((unsigned char)m_Input.PeekChar(1) == 0xBB &&
                (unsigned char)m_Input.PeekChar(2) == 0xBF) {
                m_Input.SkipChars(3);
            }
        }
    }
    if (!StackIsEmpty() && TopFrame().GetTypeInfo()->GetDataSpec() == EDataSpec::eJSON) {
        return kEmptyStr;
    }
    m_FileHeader = true;
    StartBlock('{');
    string str( ReadKey());
    if (!StackIsEmpty() && TopFrame().HasTypeInfo()) {
        const string& tname = TopFrame().GetTypeInfo()->GetName();
        if (tname.empty()) {
            UndoClassMember();
        }
        if (str != tname) {
            if (str == NStr::Replace(tname,"-","_")) {
                return tname;
            }
        }
    }
    return str;
}

void CObjectIStreamJson::EndOfRead(void)
{
    EndBlock(m_FileHeader ? '}' : 0);
    m_FileHeader = false;
    CObjectIStream::EndOfRead();
}



bool CObjectIStreamJson::ReadBool(void)
{
    string str;
    return x_ReadDataAndCheck(str) ? NStr::StringToBool(str) : x_UseMemberDefault<bool>();
}

void CObjectIStreamJson::SkipBool(void)
{
    x_SkipData();
}

char CObjectIStreamJson::ReadChar(void)
{
    string str;
    return x_ReadDataAndCheck(str) ? str.at(0) : x_UseMemberDefault<char>();
}

void CObjectIStreamJson::SkipChar(void)
{
    x_SkipData();
}

Int8 CObjectIStreamJson::ReadInt8(void)
{
    string str;
    if (x_ReadDataAndCheck(str)) {
        if (str.empty() || !(isdigit(str[0]) || str[0] == '+' || str[0] == '-')) {
            ThrowError(fFormatError, string("invalid number: ") + str);
        }
        return NStr::StringToInt8(str);
    }
    return x_UseMemberDefault<Int8>();
}

Uint8 CObjectIStreamJson::ReadUint8(void)
{
    string str;
    if (x_ReadDataAndCheck(str)) {
        if (str.empty() || !(isdigit(str[0]) || str[0] == '+')) {
            ThrowError(fFormatError, string("invalid number: ") + str);
        }
        return NStr::StringToUInt8(str);
    }
    return x_UseMemberDefault<Uint8>();
}

void CObjectIStreamJson::SkipSNumber(void)
{
    x_SkipData();
}

void CObjectIStreamJson::SkipUNumber(void)
{
    x_SkipData();
}

double CObjectIStreamJson::ReadDouble(void)
{
    string str;
    if (x_ReadDataAndCheck(str)) {
        char* endptr = nullptr;
        double result = NStr::StringToDoublePosix( str.c_str(), &endptr, NStr::fDecimalPosixFinite);
        if ( *endptr != 0 ) {
            ThrowError(fFormatError, string("invalid number: ") + str);
        }
        return result;
    }
    return x_UseMemberDefault<double>();
}

void CObjectIStreamJson::SkipFNumber(void)
{
    x_SkipData();
}

void CObjectIStreamJson::ReadString(string& s,
    EStringType type /*= eStringTypeVisible*/)
{
    char c = PeekChar(true);
    if (c == 'n') {
        if (m_Input.PeekChar(1) == 'u' &&
            m_Input.PeekChar(2) == 'l' &&
            m_Input.PeekChar(3) == 'l') {
            m_ExpectValue = false;
            m_Input.SkipChars(4);
            if ((ExpectSpecialCase() & CObjectIStream::eReadAsNil)!=0) {
                SetSpecialCaseUsed(CObjectIStream::eReadAsNil);
                return;
            }
            NCBI_THROW(CSerialException,eNullValue, kEmptyStr);
        }
    }
    s = ReadValue(type);
}

void CObjectIStreamJson::SkipString(EStringType /*type*/)
{
    x_SkipData();
}

void CObjectIStreamJson::ReadNull(void)
{
    if (m_ExpectValue) {
        string str;
        x_ReadData(str);
    }
}

void CObjectIStreamJson::SkipNull(void)
{
    if (m_ExpectValue) {
        x_SkipData();
    }
}

void CObjectIStreamJson::ReadAnyContentObject(CAnyContentObject& obj)
{
    m_ExpectValue = false;
    obj.Reset();
    string value;
    if (!m_RejectedTag.empty()) {
        obj.SetName( m_RejectedTag);
        m_RejectedTag.erase();
    } else if (!StackIsEmpty() && TopFrame().HasMemberId()) {
        obj.SetName( TopFrame().GetMemberId().GetName());
    } else {
        obj.SetName( ReadKey());
    }

    if (PeekChar(true) == '{') {
        ThrowError(fNotImplemented, "Not Implemented");
#if 0
        StartBlock('{');        
        while (NextElement()) {
            string name = ReadKey();
            value = ReadValue(eStringTypeUTF8);
            if (name[0] != '#') {
                obj.AddAttribute(name,kEmptyStr,CUtf8::AsUTF8(value,eEncoding_UTF8));
            } else {
                obj.SetValue(CUtf8::AsUTF8(value,eEncoding_UTF8));
            }
        }
        EndBlock('}');
#endif
        return;
    }
    if (PeekChar(true) == '\"') {
        value = ReadValue(eStringTypeUTF8);
    } else {
        x_ReadData(value);
    }
    obj.SetValue(CUtf8::AsUTF8(value,eEncoding_UTF8));
}

void CObjectIStreamJson::SkipAnyContent(void)
{
    char to = GetChar(true);
    if (to == '{') {
        to = '}';
    } else if (to == '[') {
        to = ']';
    } else if (to == '\"') {
    } else {
        to = '\n';
    }
    for (char c = m_Input.PeekChar(); ; c = m_Input.PeekChar()) {
        if (to == '\n') {
            if (c == ',') {
                return;
            }
        }
        if (c == to) {
            m_Input.SkipChar();
            if (c == '\n') {
                SkipEndOfLine(c);
            }
            return;
        }
        if (to != '\"') {
            if (c == '\"' || c == '{' || c == '[') {
                SkipAnyContent();
                continue;
            }
        }

        m_Input.SkipChar();
        if (c == '\n') {
            SkipEndOfLine(c);
        }
    }
}

void CObjectIStreamJson::SkipAnyContentObject(void)
{
    if (!m_RejectedTag.empty()) {
        m_RejectedTag.erase();
    }
    SkipAnyContent();
}

void CObjectIStreamJson::ReadBitString(CBitString& obj)
{
    m_ExpectValue = false;
#if BITSTRING_AS_VECTOR
    ThrowError(fNotImplemented, "Not Implemented");
#else
    if (IsCompressed()) {
        ThrowError(fNotImplemented, "Not Implemented");
        return;
    }
    Expect('\"');
    obj.clear();
    obj.resize(0);
    CBitString::size_type len = 0;
    for ( ;; ++len) {
        char c = GetChar();
        if (c == '1') {
            obj.resize(len+1);
            obj.set_bit(len);
        } else if (c != '0') {
            if ( c != 'B' ) {
                ThrowError(fFormatError, "invalid char in bit string");
            }
            break;
        }
    }
    obj.resize(len);
    Expect('\"');
#endif
}

void CObjectIStreamJson::SkipBitString(void)
{
    CBitString obj;
    ReadBitString(obj);
}


void CObjectIStreamJson::SkipByteBlock(void)
{
    CObjectIStream::ByteBlock block(*this);
    char buf[4096];
    while ( block.Read(buf, sizeof(buf)) != 0 )
        ;
    block.End();
}

TEnumValueType CObjectIStreamJson::ReadEnum(const CEnumeratedTypeValues& values)
{
    m_ExpectValue = false;
    TEnumValueType value;
    char c = SkipWhiteSpace();
    if (c == '\"') {
        value = values.FindValue( ReadValue());
    } else {
        value = (TEnumValueType)ReadInt8();
    }
    return value;
}

#ifdef VIRTUAL_MID_LEVEL_IO
void CObjectIStreamJson::ReadClassSequential(
    const CClassTypeInfo* classType, TObjectPtr classPtr)
{
    ReadClassRandom( classType, classPtr);
}

void CObjectIStreamJson::SkipClassSequential(const CClassTypeInfo* classType)
{
    SkipClassRandom(classType);
}
#endif

// container
void CObjectIStreamJson::BeginContainer(const CContainerTypeInfo* containerType)
{
    CObjectTypeInfo type(GetRealTypeInfo(containerType->GetElementType()));
    if (type.GetTypeFamily() == eTypeFamilyPrimitive && type.GetPrimitiveValueType() == ePrimitiveValueAny) {
        TopFrame().SetNotag();
        m_BlockStart = true;
        m_ExpectValue = false;
        return;
    }
    StartBlock('[');
}

void CObjectIStreamJson::EndContainer(void)
{
    if (TopFrame().GetNotag()) {
        TopFrame().SetNotag(false);
        return;
    }
    EndBlock(']');
}

bool CObjectIStreamJson::BeginContainerElement(TTypeInfo /*elementType*/)
{
    return NextElement();
}
void CObjectIStreamJson::EndContainerElement(void)
{
}

// class
void CObjectIStreamJson::BeginClass(const CClassTypeInfo* /*classInfo*/)
{
    StartBlock((GetStackDepth() > 1 && FetchFrameFromTop(1).GetNotag()) ? 0 : '{');
}

void CObjectIStreamJson::EndClass(void)
{
    m_GotNameless = false;
    EndBlock((GetStackDepth() > 1 && FetchFrameFromTop(1).GetNotag()) ? 0 : '}');
}

TMemberIndex CObjectIStreamJson::FindDeep(
    const CItemsInfo& items, const CTempString& name, bool& deep) const
{
    TMemberIndex i = items.Find(name);
    if (i != kInvalidMember) {
        deep = false;
        return i;
    }
    i = items.FindDeep(name, true);
    if (i != kInvalidMember) {
        deep = true;
        return i;
    }
// on writing, we replace hyphens with underscores;
// on reading, it complicates our life
    if (name.find_first_of("_") != CTempString::npos) {
        TMemberIndex first = items.FirstIndex();
        TMemberIndex last = items.LastIndex();
        for (i = first; i <= last; ++i) {
            const CItemInfo *itemInfo = items.GetItemInfo(i);
            string item_name = itemInfo->GetId().GetName();
            NStr::ReplaceInPlace(item_name,"-","_");
            if (name == item_name) {
                deep = false;
                return i;
            }
        }
        for (i = first; i <= last; ++i) {
            const CItemInfo* itemInfo = items.GetItemInfo(i);
            const CMemberId& id = itemInfo->GetId();
            if (id.IsAttlist() || id.HasNotag()) {
                const CClassTypeInfoBase* classType =
                    dynamic_cast<const CClassTypeInfoBase*>(
                        CItemsInfo::FindRealTypeInfo(itemInfo->GetTypeInfo()));
                if (classType) {
                    if (FindDeep(classType->GetItems(), name, deep) != kInvalidMember) {
                        deep = true;
                        return i;
                    }
                }
            }
        }
    }
    deep = true;
    return kInvalidMember;
}

TMemberIndex CObjectIStreamJson::BeginClassMember(const CClassTypeInfo* classType)
{
    TMemberIndex first = classType->GetMembers().FirstIndex();
    TMemberIndex last = classType->GetMembers().LastIndex();

    if ( !NextElement() ) {
        if (!m_GotNameless &&
            classType->GetMemberInfo(last)->GetId().HasNotag() &&
            classType->GetMemberInfo(last)->GetTypeInfo()->GetTypeFamily() == eTypeFamilyPrimitive) {
            TopFrame().SetNotag();
            m_GotNameless = true;
            return last;
        }
        return kInvalidMember;
    }
    m_GotNameless = false;

    char c = PeekChar();
    if (m_RejectedTag.empty() && (c == '[' || c == '{')) {
        for (TMemberIndex i = first; i <= last; ++i) {
            if (classType->GetMemberInfo(i)->GetId().HasNotag()) {
                TopFrame().SetNotag();
                return i;
            }
        }
    }
    string tagName = ReadKey();

    if (tagName[0] == '#') {
        tagName = tagName.substr(1);
        TopFrame().SetNotag();
        m_GotNameless = true;
    }

    bool deep = false;
    TMemberIndex ind = FindDeep(classType->GetMembers(), tagName, deep);
    if (ind == kInvalidMember) {
        if (m_TypeAlias && classType->GetMembers().GetItemInfo(last)->GetId().HasNotag()) {
            m_TypeAlias = nullptr;
            return last;
        }
        if (classType->GetMembers().GetItemInfo(last)->GetId().HasAnyContent()) {
            UndoClassMember();
            return last;
        }
        if (!m_GotNameless && classType->GetDataSpec() == EDataSpec::eJSON) {
            if (CanSkipUnknownMembers()) {
                SetFailFlags(fUnknownValue);
                SkipAnyContent();
                m_ExpectValue = false;
                return BeginClassMember(classType);
            } else {
                UnexpectedMember(tagName, classType->GetMembers());
            }
        }
    } else {
        if (classType->GetMembers().GetItemInfo(ind)->GetId().HasNotag()) {
            TopFrame().SetNotag();
            m_GotNameless = true;
        }
    }

/*
    skipping unknown members is not present here
    this method has to do with random order of members, for example in XML attribute list
    there is no special marker for end-of-list, so it is "ended" by unknown member.
    In theory, I could find out memberId and check if it is Attlist
    If the data type is SET and not attlist, then skipping unknowns can be dangerous 
*/
    if (deep) {
        if (ind != kInvalidMember) {
            TopFrame().SetNotag();
        }
        UndoClassMember();
    }
    return ind;
}

TMemberIndex CObjectIStreamJson::BeginClassMember(const CClassTypeInfo* classType,
                                      TMemberIndex pos)
{
    TMemberIndex first = classType->GetMembers().FirstIndex();
    TMemberIndex last = classType->GetMembers().LastIndex();
    if (m_RejectedTag.empty()) {
        if (pos == first) {
            if (classType->GetMemberInfo(first)->GetId().IsAttlist()) {
                TopFrame().SetNotag();
                return first;
            }
        }
    }

    if ( !NextElement() ) {
        if (pos == last &&
            classType->GetMemberInfo(pos)->GetId().HasNotag() &&
            classType->GetMemberInfo(pos)->GetTypeInfo()->GetTypeFamily() == eTypeFamilyPrimitive) {
            TopFrame().SetNotag();
            return pos;
        }
        return kInvalidMember;
    }
    char c = PeekChar();
    if (m_RejectedTag.empty() && (c == '[' || c == '{')) {
        for (TMemberIndex i = pos; i <= last; ++i) {
            if (classType->GetMemberInfo(i)->GetId().HasNotag()) {
                TopFrame().SetNotag();
                return i;
            }
        }
    }
    string tagName = ReadKey();
    if (tagName[0] == '#') {
        tagName = tagName.substr(1);
        TopFrame().SetNotag();
    }
    bool deep = false;
    TMemberIndex ind = FindDeep(classType->GetMembers(), tagName, deep);
    if ( ind == kInvalidMember ) {
        if (CanSkipUnknownMembers()) {
            SetFailFlags(fUnknownValue);
            SkipAnyContent();
            m_ExpectValue = false;
            return BeginClassMember(classType,pos);
        } else {
            UnexpectedMember(tagName, classType->GetMembers());
        }
    }
    if (deep) {
        if (ind != kInvalidMember) {
            TopFrame().SetNotag();
        }
        UndoClassMember();
    } else if (ind != kInvalidMember) {
        if (classType->GetMembers().GetItemInfo(ind)->GetId().HasAnyContent()) {
            UndoClassMember();
        }
    }
    return ind;
}
void CObjectIStreamJson::EndClassMember(void)
{
    TopFrame().SetNotag(false);
}

void CObjectIStreamJson::UndoClassMember(void)
{
    m_RejectedTag = m_LastTag;
}

// choice
void CObjectIStreamJson::BeginChoice(const CChoiceTypeInfo* /*choiceType*/)
{
    StartBlock((GetStackDepth() > 1 && FetchFrameFromTop(1).GetNotag()) ? 0 : '{');
}

void CObjectIStreamJson::EndChoice(void)
{
    EndBlock((GetStackDepth() > 1 && FetchFrameFromTop(1).GetNotag()) ? 0 : '}');
}

TMemberIndex CObjectIStreamJson::BeginChoiceVariant(const CChoiceTypeInfo* choiceType)
{
    if ( !NextElement() )
        return kInvalidMember;
    string tagName = ReadKey();
    bool deep = false;
    TMemberIndex ind = FindDeep(choiceType->GetVariants(), tagName, deep);
    if ( ind == kInvalidMember ) {
        if (CanSkipUnknownVariants()) {
            SetFailFlags(fUnknownValue);
        } else {
            UnexpectedMember(tagName, choiceType->GetVariants());
        }
    }
    if (deep) {
        if (ind != kInvalidMember) {
            TopFrame().SetNotag();
        }
        UndoClassMember();
    }
    return ind;
}

void CObjectIStreamJson::EndChoiceVariant(void)
{
    TopFrame().SetNotag(false);
}

// byte block
void CObjectIStreamJson::BeginBytes(ByteBlock& /*block*/)
{
    char c = SkipWhiteSpaceAndGetChar();
    if (c == '\"') {
        m_Closing = '\"';
    } else if (c == '[') {
        m_Closing = ']';
    } else {
        ThrowError(fFormatError, "'\"' or '[' expected");
    }
}

void CObjectIStreamJson::EndBytes(const ByteBlock& /*block*/)
{
    Expect(m_Closing);
    m_Closing = 0;
}

int CObjectIStreamJson::GetHexChar(void)
{
    char c = m_Input.GetChar();
    if ( c >= '0' && c <= '9' ) {
        return c - '0';
    }
    else if ( c >= 'A' && c <= 'Z' ) {
        return c - 'A' + 10;
    }
    else if ( c >= 'a' && c <= 'z' ) {
        return c - 'a' + 10;
    }
    else {
        m_Input.UngetChar(c);
    }
    return -1;
}

int CObjectIStreamJson::GetBase64Char(void)
{
    char c = SkipWhiteSpace();
    if ( (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z') ||
         (c == '+' || c == '/'  || c == '=')) {
        return c;
    }
    return -1;
}

size_t CObjectIStreamJson::ReadBytes(
    ByteBlock& block, char* dst, size_t length)
{
    m_ExpectValue = false;
    if (m_BinaryFormat != CObjectIStreamJson::eDefault) {
        return ReadCustomBytes(block,dst,length);
    }
    if (IsCompressed()) {
        return ReadBase64Bytes( block, dst, length );
    }
    return ReadHexBytes( block, dst, length );
}

size_t CObjectIStreamJson::ReadCustomBytes(
    ByteBlock& block, char* dst, size_t length)
{
    if (m_BinaryFormat == eString_Base64) {
        return ReadBase64Bytes(block, dst, length);
    } else if (m_BinaryFormat == eString_Hex) {
        return ReadHexBytes(block, dst, length);
    }
    bool end_of_data = false;
    size_t count = 0;
    while ( !end_of_data && length-- > 0 ) {
        Uint1 c = 0;
        Uint1 mask=0x80;
        switch (m_BinaryFormat) {
        case eArray_Bool:
            for (; !end_of_data && mask!=0; mask = Uint1(mask >> 1)) {
                if (ReadBool()) {
                    c |= mask;
                }
                end_of_data = !GetChar(',', true);
            }
            ++count;
            *dst++ = c;
            break;
        case eArray_01:
            for (; !end_of_data && mask!=0; mask = Uint1(mask >> 1)) {
                if (ReadChar() != '0') {
                    c |= mask;
                }
                end_of_data = !GetChar(',', true);
            }
            ++count;
            *dst++ = c;
            break;
        default:
        case eArray_Uint:
            c = (Uint1)ReadUint8();
            end_of_data = !GetChar(',', true);
            ++count;
            *dst++ = c;
            break;
        case eString_01:
        case eString_01B:
            for (; !end_of_data && mask!=0; mask = Uint1(mask >> 1)) {
                char t = GetChar();
                end_of_data = t == '\"' || t == 'B';
                if (!end_of_data && t != '0') {
                    c |= mask;
                }
                if (t == '\"') {
                    m_Input.UngetChar(t);
                }
            }
            if (mask != 0x40) {
                ++count;
                *dst++ = c;
            }
            break;
        }
    }
    if (end_of_data) {
        block.EndOfBlock();
    }
    return count;
}

size_t CObjectIStreamJson::ReadBase64Bytes(
    ByteBlock& block, char* dst, size_t length)
{
    size_t count = 0;
    bool end_of_data = false;
    const size_t chunk_in = 80;
    char src_buf[chunk_in];
    size_t bytes_left = length;
    size_t src_size, src_read, dst_written;
    while (!end_of_data && bytes_left > chunk_in && bytes_left <= length) {
        for ( src_size = 0; src_size < chunk_in; ) {
            int c = GetBase64Char();
            if (c < 0) {
                end_of_data = true;
                break;
            }
            /*if (c != '=')*/ {
                src_buf[ src_size++ ] = (char)c;
            }
            m_Input.SkipChar();
        }
        BASE64_Decode( src_buf, src_size, &src_read,
                    dst, bytes_left, &dst_written);
        if (src_size != src_read) {
            ThrowError(fFail, "error decoding base64Binary data");
        }
        count += dst_written;
        bytes_left -= dst_written;
        dst += dst_written;
    }
    if (end_of_data) {
        block.EndOfBlock();
    }
    return count;
}

size_t CObjectIStreamJson::ReadHexBytes(
    ByteBlock& block, char* dst, size_t length)
{
    size_t count = 0;
    while ( length-- > 0 ) {
        int c1 = GetHexChar();
        if ( c1 < 0 ) {
            block.EndOfBlock();
            return count;
        }
        int c2 = GetHexChar();
        if ( c2 < 0 ) {
            *dst++ = char(c1 << 4);
            count++;
            block.EndOfBlock();
            return count;
        }
        else {
            *dst++ = char((c1 << 4) | c2);
            count++;
        }
    }
    return count;
}

// char block
void CObjectIStreamJson::BeginChars(CharBlock& /*block*/)
{
}
size_t CObjectIStreamJson::ReadChars(CharBlock& /*block*/, char* /*buffer*/, size_t /*count*/)
{
    ThrowError(fNotImplemented, "Not Implemented");
    return 0;
}
void CObjectIStreamJson::EndChars(const CharBlock& /*block*/)
{
}

CObjectIStream::EPointerType CObjectIStreamJson::ReadPointerType(void)
{
    char c = PeekChar(true);
    if (c == 'n') {
        string s;
        x_ReadData(s);
        if (s != "null") {
            ThrowError(fFormatError, "null expected");
        }
        return eNullPointer;
    }
    return eThisPointer;
}

CObjectIStream::TObjectIndex CObjectIStreamJson::ReadObjectPointer(void)
{
    ThrowError(fNotImplemented, "Not Implemented");
    return 0;
}

string CObjectIStreamJson::ReadOtherPointer(void)
{
    ThrowError(fNotImplemented, "Not Implemented");
    return "";
}

END_NCBI_SCOPE
