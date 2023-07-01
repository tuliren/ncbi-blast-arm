#ifndef CORELIB___VERSION_API__HPP
#define CORELIB___VERSION_API__HPP

/*  $Id: version_api.hpp 657796 2022-10-17 18:54:35Z ivanov $
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
 * Authors:  Denis Vakatov, Vladimir Ivanov, Anatoliy Kuznetsov
 *
 *
 */

/// @file version.hpp
/// Define CVersionInfo, a version info storage class.


#include <corelib/ncbitime.hpp>
#include <corelib/ncbiobj.hpp>



BEGIN_NCBI_SCOPE

/** @addtogroup Version
 *
 * @{
 */

/////////////////////////////////////////////////////////////////////////////
// CVersionInfo


/// This class allows to add build info (date and tag) to application version.
///
/// This can be done by providing explicitly created SBuildInfo instance
///
/// If clients do not explicitly set their own build info,
/// C++ Toolkit build info will be used in the reporting instead.

struct NCBI_XNCBI_EXPORT SBuildInfo
{
    enum EExtra
    {
        eBuildDate,
        eBuildTag,
        eTeamCityProjectName,
        eTeamCityBuildConf,
        eTeamCityBuildNumber,
        eBuildID,
        eSubversionRevision, ///< Numeric if present
        eStableComponentsVersion,
        eDevelopmentVersion,
        eProductionVersion,
        eBuiltAs,
        eRevision ///< Not necessarily numeric
    };
    string date;
    string tag;
    vector< pair<EExtra,string> > m_extra;

    SBuildInfo(void);
    explicit
    SBuildInfo(const string& d, const string& t = kEmptyStr) : date(d), tag(t) {
    }

    SBuildInfo& Extra( EExtra key, const string& value);
    SBuildInfo& Extra( EExtra key, int value);
    string GetExtraValue( EExtra key, const string& default_value = kEmptyStr) const;

    static string ExtraName(EExtra key);
    static string ExtraNameXml(EExtra key);
    static string ExtraNameJson(EExtra key);
    static string ExtraNameAppLog(EExtra key);

    /// Converts 'date' parameter to CTime.
    /// Returns empty CTime object if unable to do an conversion or build date/time is unknown,
    /// so check CTime::IsEmpty().
    CTime GetBuildTime(void) const;

    string Print(size_t offset = 0) const;
    string PrintXml(void) const;
    string PrintJson(void) const;
};

#define NCBI_SBUILDINFO_DEFAULT_INSTANCE() SBuildInfo()


/////////////////////////////////////////////////////////////////////////////
///
/// CVersionInfo --
///
/// Define class for storing version information.

class NCBI_XNCBI_EXPORT CVersionInfo
{
public:
    /// Constructor
    CVersionInfo(int  ver_major,
                 int  ver_minor,
                 int  patch_level = 0,
                 const string& name = kEmptyStr);

    /// @param version
    ///    version string in rcs format (like 1.2.4)
    ///
    CVersionInfo(const string& version,
                 const string& name = kEmptyStr);

    enum EVersionFlags {
        kAny = 0,
        kLatest
    };
    CVersionInfo(EVersionFlags flags = kLatest);

    /// Destructor.
    virtual ~CVersionInfo() {}

    /// Take version info from string
    void FromStr(const string& version);

    void SetVersion(int  ver_major,
                    int  ver_minor,
                    int  patch_level = 0);

    /// Print version information.
    ///
    /// @return
    ///   String representation of the version,
    ///   Version information is printed in the following forms:
    ///     - <ver_major>.<ver_minor>.<patch_level>
    ///     - <ver_major>.<ver_minor>.<patch_level> (<name>)
    ///   Return empty string if major version is undefined (< 0).
    virtual string Print(void) const;

    /// Print version information as XML (see ncbi_version.xsd)
    virtual string PrintXml(void) const;

    /// Print version information as JSON
    virtual string PrintJson(void) const;

    /// Major version
    int GetMajor(void) const { return m_Major; }
    /// Minor version
    int GetMinor(void) const { return m_Minor; }
    /// Patch level
    int GetPatchLevel(void) const { return m_PatchLevel; }

    const string& GetName(void) const { return m_Name; }

    /// Version comparison result
    /// @sa Match
    enum EMatch {
        eNonCompatible,           ///< major, minor does not match
        eConditionallyCompatible, ///< patch level incompatibility
        eBackwardCompatible,      ///< patch level is newer
        eFullyCompatible          ///< exactly the same version
    };

    /// Check if version matches another version.
    /// @param version_info
    ///   Version Info to compare with
    EMatch Match(const CVersionInfo& version_info) const;

    /// Check if version is all zero (major, minor, patch)
    /// Convention is that all-zero version used in requests as 
    /// "get me anything". 
    /// @sa kAny
    bool IsAny() const 
        { return !(m_Major | m_Minor | m_PatchLevel); }

    /// Check if version is all -1 (major, minor, patch)
    /// Convention is that -1 version used in requests as 
    /// "get me the latest version". 
    /// @sa kLatest
    bool IsLatest() const 
       { return (m_Major == -1 && m_Minor == -1 && m_PatchLevel == -1); }

    /// Check if this version info is more contemporary version 
    /// than parameter cinfo (or the same version)
    ///
    /// @param cinfo
    ///    Version checked (all components must be <= than this)
    ///
    bool IsUpCompatible(const CVersionInfo &cinfo) const
    {
        return cinfo.m_Major <= m_Major && 
               cinfo.m_Minor <= m_Minor &&
               cinfo.m_PatchLevel <= m_PatchLevel;
    }

protected:
    int          m_Major;       ///< Major number
    int          m_Minor;       ///< Minor number
    int          m_PatchLevel;  ///< Patch level
    string       m_Name;        ///< Name
};


class NCBI_XNCBI_EXPORT CComponentVersionInfoAPI : public CVersionInfo
{
public:

    /// Constructor
    CComponentVersionInfoAPI(const string& component_name,
                             int  ver_major,
                             int  ver_minor,
                             int  patch_level,
                             const string& ver_name,
                             const SBuildInfo& build_info);

    /// Constructor
    ///
    /// @param component_name
    ///    component name
    /// @param version
    ///    version string (eg, 1.2.4)
    /// @param ver_name
    ///    version name
    CComponentVersionInfoAPI(const string& component_name,
                             const string& version,
                             const string& ver_name,
                             const SBuildInfo& build_info);

    /// Get component name
    const string& GetComponentName(void) const
    {
        return m_ComponentName;
    }

    /// Print version information.
    virtual string Print(void) const;

    /// Print version information ax XML.
    virtual string PrintXml(void) const;

    /// Print version information as JSON.
    virtual string PrintJson(void) const;

private:
    string m_ComponentName;
    SBuildInfo m_BuildInfo;
};


class NCBI_XNCBI_EXPORT CVersionAPI : public CObject
{
public:

    explicit
    CVersionAPI(const SBuildInfo& build_info = SBuildInfo());

    explicit
    CVersionAPI(const CVersionInfo& version,
                const SBuildInfo& build_info = SBuildInfo());

    CVersionAPI(const CVersionAPI& version);
    CVersionAPI(CVersionAPI&& version) = default;

    CVersionAPI& operator=(const CVersionAPI& version);
    CVersionAPI& operator=(CVersionAPI&& version) = default;

    /// Set version information
    void SetVersionInfo( int  ver_major,
                         int  ver_minor,
                         int  patch_level = 0,
                         const string& ver_name = kEmptyStr);
    void SetVersionInfo( int  ver_major,
                         int  ver_minor,
                         int  patch_level,
                         const string& ver_name,
                         const SBuildInfo& build_info);
    /// Set version information
    /// @note Takes the ownership over the passed VersionInfo object 
    void SetVersionInfo(CVersionInfo* version);
    void SetVersionInfo(CVersionInfo* version, const SBuildInfo& build_info);
    /// Get version information
    const CVersionInfo& GetVersionInfo( ) const;

    /// Add component version information
    void AddComponentVersion( const string& component_name,
                              int           ver_major,
                              int           ver_minor,
                              int           patch_level,
                              const string& ver_name,
                              const SBuildInfo& build_info);
    /// Add component version information
    /// @note Takes the ownership over the passed VersionInfo object 
    void AddComponentVersion( CComponentVersionInfoAPI* component);

    /// Get build info (date and tag, if set)
    const SBuildInfo& GetBuildInfo() const;

    static string GetPackageName(void);
    static CVersionInfo GetPackageVersion(void);
    static string GetPackageConfig(void);

    enum EPrintFlags {
        fVersionInfo    = 0x01,  ///< Print version info
        fComponents     = 0x02,  ///< Print components version info
        fPackageShort   = 0x04,  ///< Print package info, if available
        fPackageFull    = 0x08,  ///< Print package info, if available
        fBuildInfo      = 0x10,  ///< Print build info (date and tag)
        fBuildSignature = 0x20,  ///< Print build signature, if available
        fGI64bit        = 0x40,  ///< Print info about GI size
        fTCBuildNumber  = 0x0,   ///< obsolete, has no effect
        fPrintAll       = 0xFF   ///< Print all version data
    };
    typedef int TPrintFlags;  ///< Binary OR of EPrintFlags
    
    /// Print version data, plain text.
    string Print(const string& appname, TPrintFlags flags = fPrintAll) const;
    /// Print version data, XML.
    string PrintXml(const string& appname, TPrintFlags flags = fPrintAll) const;
    /// Print version data, JSON.
    string PrintJson(const string& appname, TPrintFlags flags = fPrintAll) const;

private:
    static void x_Copy(CVersionAPI& to, const CVersionAPI& from);

    unique_ptr<CVersionInfo> m_VersionInfo;
    vector<unique_ptr<CComponentVersionInfoAPI>> m_Components;
    SBuildInfo m_BuildInfo;
};


/// Return true if one version info is matches another better than
/// the best variant.
/// When condition satisfies, return true and the former best values 
/// are getting updated
/// @param info
///    Version info to search
/// @param cinfo
///    Comparison candidate
/// @param best_major
///    Best major version found (reference)
/// @param best_minor
///    Best minor version found (reference)
/// @param best_patch_level
///    Best patch levelfound (reference)
bool NCBI_XNCBI_EXPORT IsBetterVersion(const CVersionInfo& info, 
                                       const CVersionInfo& cinfo,
                                       int&  best_major, 
                                       int&  best_minor,
                                       int&  best_patch_level);

inline
bool operator==(const CVersionInfo& v1, const CVersionInfo& v2)
{
    return (v1.GetMajor() == v2.GetMajor() &&
            v1.GetMinor() == v2.GetMinor() &&
            v1.GetPatchLevel() == v2.GetPatchLevel());
}

inline
bool operator<(const CVersionInfo& v1, const CVersionInfo& v2)
{
    return (v1.GetMajor() < v2.GetMajor() ||
            (v1.GetMajor() == v2.GetMajor() &&
             (v1.GetMinor() < v2.GetMinor() ||
              (v1.GetMinor() == v2.GetMinor() &&
               (v1.GetPatchLevel() < v2.GetPatchLevel())))));
}

inline
ostream& operator << (ostream& strm, const CVersionInfo& v)
{
    strm << v.GetMajor() << "." << v.GetMinor() << "." << v.GetPatchLevel();
    
    return strm;
}

/// Algorithm function to find version in the container
///
/// Scans the provided iterator for version with the same major and
/// minor version and the newest patch level.
///
/// @param first
///    first iterator to start search 
/// @param last
///    ending iterator (typically returned by end() function of an STL
///    container)
/// @return 
///    iterator on the best version or last
template<class It>
It FindVersion(It first, It last, const CVersionInfo& info)
{
    It  best_version = last;  // not found by default
    int best_major = -1;
    int best_minor = -1;
    int best_patch_level = -1;

    for ( ;first != last; ++first) {
        const CVersionInfo& vinfo = *first;

        if (IsBetterVersion(vinfo, info, 
                            best_major, best_minor, best_patch_level))
        {
            best_version = first;
        }
    }        
    
    return best_version;
}


/// Algorithm function to find version in the container
///
/// Scans the provided container for version with the same major and
/// minor version and the newest patch level.
///
/// @param container
///    container object to search in 
/// @return 
///    iterator on the best fit version (last if no version found)
template<class TClass>
typename TClass::const_iterator FindVersion(const TClass& cont, 
                                            const CVersionInfo& info)
{
    typename TClass::const_iterator it = cont.begin();
    typename TClass::const_iterator it_end = cont.end();
    return FindVersion(it, it_end, info);
}

/// Parse string, extract version info and program name
/// (case insensitive)
///
/// Examples:
///   MyProgram 1.2.3
///   MyProgram version 1.2.3
///   MyProgram v. 1.2.3
///   MyProgram ver. 1.2.3
///   version 1.2.3
///
NCBI_XNCBI_EXPORT
void ParseVersionString(const string&  vstr, 
                        string*        program_name, 
                        CVersionInfo*  ver);

/* @} */

#ifndef CORELIB___VERSION__HPP
# define CVersion CVersionAPI
# define CComponentVersionInfo CComponentVersionInfoAPI
#endif

END_NCBI_SCOPE

#endif // CORELIB___VERSION_API__HPP
