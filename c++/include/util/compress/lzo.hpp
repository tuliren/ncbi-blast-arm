#ifndef UTIL_COMPRESS__LZO__HPP
#define UTIL_COMPRESS__LZO__HPP

/*  $Id: lzo.hpp 659027 2022-11-14 18:44:15Z ivanov $
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
 * Author:  Vladimir Ivanov
 *
 */

/// @file lzo.hpp
///
/// LZO Compression API.
///
/// LZO is a data compression library which is suitable for data
/// (de)compression in real-time. This means it favours speed
/// over compression ratio.
///
/// LZO is good to compress some sort of data only, that have a limited
/// set of characters or many recurring sequences. It is not suitable
/// for a random data, that limits its usage. It is better to test 
/// compression on yours own data before making decision to use LZO.
/// Use zlib if you need a more universal and robust solution, that is also
/// slower and needs more memory.
///
/// We don't support all possible algorithms, implemented in LZO.
/// Only LZO1X is used in this API. Author of LZO says that it is 
/// often the best choice of all.
/// 
/// NOTE:
///   LZO is a just an compression algorithm to compress buffers,
///   it doesnt support streaming or file compression out from the box.
///   We use our own stream/file format to add such support.
///
/// CLZOCompression        - base methods for compression/decompression
///                          memory buffers and files.
/// CLZOCompressionFile    - allow read/write operations on files.
///                          LZO doesn't support files, so we use here
///                          our own file format (very simple).
/// CLZOCompressor         - LZO based compressor
///                          (used in CLZOStreamCompressor). 
/// CLZODecompressor       - LZO based decompressor 
///                          (used in CLZOStreamDecompressor). 
/// CLZOStreamCompressor   - LZO based compression stream processor
///                          (see util/compress/stream.hpp for details).
/// CLZOStreamDecompressor - LZO based decompression stream processor
///                          (see util/compress/stream.hpp for details).
///
/// For more details see LZO documentation:
///    http://www.oberhumer.com/opensource/lzo/
/// 
/// @warning
///   LZO ia an optional compression component and can be missed on a current
///   platform. It is recommended to guard its usage:
/// 
///   #if defined(HAVE_LIBLZO)
///       // use LZO related code here
///   #else
///      // some backup code, or error reporting
///   #endif
/// 
/// See also comments at the beginning of "compress.hpp".


#include <util/compress/stream.hpp>

#if defined(HAVE_LIBLZO)

#include <stdio.h>

/** @addtogroup Compression
 *
 * @{
 */

BEGIN_NCBI_SCOPE


/// Default LZO block size.
/// @deprecated Please use CLZOCompression::GetBlockSizeDefault() instead
const size_t kLZODefaultBlockSize = 24*1024;

// Forward declaration of structure to define parameters for some level of compression.
struct SCompressionParam;


/////////////////////////////////////////////////////////////////////////////
///
/// CLZOCompression --
///
/// Define a base methods for compression/decompression memory buffers
/// and files.

class NCBI_XUTIL_EXPORT CLZOCompression : public CCompression 
{
public:
    /// Initialize LZO library.
    ///
    /// You should call this method only once, before any real
    /// compression/decompression operations.
    /// @li <b>Multi-Thread safety:</b>
    ///   If you are using this API in a multi-threaded application, and there
    ///   is more than one thread using this API, it is safe to call
    ///   Initialize() explicitly in the beginning of your main thread,
    ///   before you run any other threads.
    static bool Initialize(void);

    /// Compression/decompression flags.
    enum EFlags {
        /// Allow transparent reading data from buffer/file/stream
        /// regardless is it compressed or not. But be aware,
        /// if data source contains broken data and API cannot detect that
        /// it is compressed data, that you can get binary instead of
        /// decompressed data. By default this flag is OFF.
        fAllowTransparentRead = (1<<0),
        /// Allow to "compress/decompress" empty data. Buffer compression
        /// functions starts to return TRUE instead of FALSE for zero-length
        /// input. And, if this flag is used together with fStreamFormat
        /// than the output will have header and footer only.
        fAllowEmptyData       = (1<<1),
        /// Add/check (accordingly to compression or decompression)
        /// the compressed data checksum. A checksum is a form of
        /// redundancy check. We use the safe decompressor, but this can be
        /// not enough, because many data errors will not result in
        /// a compressed data violation.
        fChecksum             = (1<<2),
        /// Use stream compatible format for data compression.
        /// This flag have an effect for CompressBuffer/DecompressBuffer only.
        /// File and stream based compressors always use it by default.
        /// Use this flag with DecompressBuffer() to decompress data,
        /// compressed using streams, or compress data with CompressBuffer(),
        /// that can be decompressed using decompression stream.
        /// @note 
        ///   This flag is reguired to compress data > 4GB.
        ///   LZO default single block compression cannot handle such large
        ///   amount of data on some platforms.
        fStreamFormat         = (1<<3),
        /// Store file information like file name and file modification date
        /// of the compressed file into the file/stream.
        /// Works only with fStreamFormat flag.
        fStoreFileInfo        = (1<<4) | fStreamFormat
    }; 
    typedef CLZOCompression::TFlags TLZOFlags; ///< Bitwise OR of EFlags

    /// Constructor.
    CLZOCompression(ELevel level = eLevel_Default);

    /// @deprecated 
    ///   Use CLZOCompression(ELevel) constructor without block size, that can be set separately
    NCBI_DEPRECATED_CTOR(CLZOCompression(
        ELevel level,
        size_t blocksize
    ));

    /// Destructor.
    virtual ~CLZOCompression(void);

    /// Return name and version of the compression library.
    virtual CVersionInfo GetVersion(void) const;

    /// Get compression level.
    ///
    /// @note
    ///   This API use only two compression levels for LZO method.
    ///   All compression levels will be translated only into 2 real values. 
    ///   We use LZO1X-999 compression for eLevel_Best, and LZO1X-1 for all
    ///   other levels of compression.
    virtual ELevel GetLevel(void) const;

    /// Returns default compression level for a compression algorithm.
    virtual ELevel GetDefaultLevel(void) const
        { return eLevel_Lowest; };

    /// Check if compression have support for a specified feature
    virtual bool HaveSupport(ESupportFeature feature);


    //=======================================================================
    // Utility functions 
    //=======================================================================

    /// Compress data in the buffer.
    ///
    /// @param src_buf
    ///   [in] Source buffer.
    /// @param src_len
    ///   [in] Size of data in source  buffer.
    /// @param dst_buf
    ///   [in] Destination buffer.
    /// @param dst_size
    ///   [in] Size of destination buffer.
    ///    The size of the destination buffer must be a little more
    ///    then size of the source buffer.
    /// @param dst_len
    ///   [out] Size of compressed data in destination buffer.
    /// @return
    ///   Return TRUE if operation was successfully or FALSE otherwise.
    ///   On success, 'dst_buf' contains compressed data of dst_len size.
    /// @note
    ///   Use fStreamFormat flag to compress data > 4GB.
    ///   LZO default single block compression cannot handle such large
    ///   amount of data on some platforms.
    /// @sa
    ///   EstimateCompressionBufferSize, DecompressBuffer
    virtual bool CompressBuffer(
        const void* src_buf, size_t  src_len,
        void*       dst_buf, size_t  dst_size,
        /* out */            size_t* dst_len
    );

    /// Decompress data in the buffer.
    ///
    /// @param src_buf
    ///   Source buffer.
    /// @param src_len
    ///   Size of data in source buffer.
    /// @param dst_buf
    ///   Destination buffer.
    /// @param dst_size
    ///   Size of destination buffer.
    ///   It must be large enough to hold all of the uncompressed data for the operation to complete.
    /// @param dst_len
    ///   Size of decompressed data in destination buffer.
    /// @return
    ///   Return TRUE if operation was successfully or FALSE otherwise.
    ///   On success, 'dst_buf' contains decompressed data of dst_len size.
    /// @note
    ///   Use fStreamFormat flag to decompress data, compressed using streams,
    ///   or CompressBuffer() with this flag.
    /// @sa
    ///   CompressBuffer
    virtual bool DecompressBuffer(
        const void* src_buf, size_t  src_len,
        void*       dst_buf, size_t  dst_size,
        /* out */            size_t* dst_len
    );

    /// Estimate buffer size for data compression.
    ///
    /// Simplified method for estimation of the size of buffer required
    /// to compress specified number of bytes of data, uses current block size and flags.
    /// @sa
    ///   EstimateCompressionBufferSize, CompressBuffer
    virtual size_t EstimateCompressionBufferSize(size_t src_len);

    /// Estimate buffer size for data compression (advanced version).
    ///
    /// The function shall estimate the size of buffer required to compress
    /// specified number of bytes of data. This function return a conservative
    /// value that larger than 'src_len'. 
    /// @param src_len
    ///   Size of data in the source buffer.
    /// @blocksize
    ///   Size of blocks that will be used for compression.
    ///   Value 0 means default block size, same as GetBlockSizeDefault().
    /// @flags
    ///   Flags that will be used for compression.
    /// @return
    ///   Estimated buffer size.
    /// @sa
    ///   CompressBuffer, GetBlockSizeDefault
    static size_t EstimateCompressionBufferSize(size_t src_len, size_t blocksize, TLZOFlags flags);

    /// Get recommended buffer sizes for stream/file I/O.
    ///
    /// These buffer sizes are softly recommended. They are not required, (de)compression
    /// streams accepts any reasonable buffer size, for both input and output.
    /// Respecting the recommended size just makes it a bit easier for (de)compressor,
    /// reducing the amount of memory shuffling and buffering, resulting in minor 
    /// performance savings. If compression library doesn't have preferences about 
    /// I/O buffer sizes, kCompressionDefaultBufSize will be used.
    /// @param round_up_by
    ///   If specified, round up a returned value by specified amount. 
    ///   Useful for better memory management. For example you can round up to virtual
    ///   memory page size.
    /// @return
    ///   Structure with recommended buffer sizes.
    /// @note
    ///   Applicable for streaming/file operations.
    /// @sa
    ///   kCompressionDefaultBufSize, CSystemInfo::GetVirtualMemoryPageSize()
    /// 
    static SRecommendedBufferSizes GetRecommendedBufferSizes(size_t round_up = 0);

    /// Compress file.
    ///
    /// @param src_file
    ///   File name of source file.
    /// @param dst_file
    ///   File name of result file.
    /// @param file_io_bufsize
    ///   Size of the buffer used to read from a source file. 
    ///   Writing happens immediately on receiving some data from a compressor.
    /// @param compression_in_bufsize
    ///   Size of the internal buffer holding input data to be compressed.
    ///   It can be different from 'file_io_bufsize' depending on a using 
    ///   compression method, OS and file system.
    /// @param compression_out_bufsize
    ///   Size of the internal buffer to receive data from a compressor.
    /// @return
    ///   Return TRUE on success, FALSE on error.
    /// @note
    ///   This method don't store any file meta information like name, date/time, owner or attributes.
    /// @sa
    ///   DecompressFile, GetRecommendedBufferSizes, CLZOCompressionFile
    /// 
    virtual bool CompressFile(
        const string& src_file,
        const string& dst_file,
        size_t        file_io_bufsize         = kCompressionDefaultBufSize,
        size_t        compression_in_bufsize  = kCompressionDefaultBufSize,
        size_t        compression_out_bufsize = kCompressionDefaultBufSize
    );

    /// Decompress file.
    ///
    /// @param src_file
    ///   File name of source file.
    /// @param dst_file
    ///   File name of result file.
    /// @param file_io_bufsize
    ///   Size of the buffer used to read from a source file. 
    ///   Writing happens immediately on receiving some data from a decompressor.
    /// @param decompression_in_bufsize
    ///   Size of the internal buffer holding input data to be decompressed.
    ///   It can be different from 'file_io_bufsize' depending on a using 
    ///   compression method, OS and file system.
    /// @param decompression_out_bufsize
    ///   Size of the internal buffer to receive data from a decompressor.
    /// @return
    ///   Return TRUE on success, FALSE on error.
    /// @sa
    ///   CompressFile, GetRecommendedBufferSizes, CLZOCompressionFile
    /// 
    virtual bool DecompressFile(
        const string& src_file,
        const string& dst_file, 
        size_t        file_io_bufsize           = kCompressionDefaultBufSize,
        size_t        decompression_in_bufsize  = kCompressionDefaultBufSize,
        size_t        decompression_out_bufsize = kCompressionDefaultBufSize
    );

    /// Structure to keep compressed file information.
    struct SFileInfo {
        string  name;
        string  comment;
        time_t  mtime;
        SFileInfo(void) : mtime(0) {};
    };

    /// @warning No dictionary support for LZO. Always return FALSE.
    /// @sa HaveSupport
    virtual bool SetDictionary(
        CCompressionDictionary& dict, 
        ENcbiOwnership          own = eNoOwnership
    );

    //=======================================================================
    // Advanced compression-specific parameters
    //=======================================================================
    // Allow to tune up (de)compression for a specific needs.
    //
    // - Pin down compression parameters to some specific values, so these
    //   values are no longer dynamically selected by the compressor.
    // - All setting parameters should be in the range [min,max], 
    //   or equal to default.
    // - All parameters should be set before starting (de)compression, 
    //   or it will be ignored for current operation.
    //=======================================================================

    /// Block size 
    /// 
    /// LZO is a block compression algorithm - it compresses and decompresses
    /// blocks of data. Block size must be the same for compression and 
    /// decompression. This parameter define a block size used
    /// for file/stream based compression/decompression to divide big 
    /// data to chunks and compress them separately.
    /// 
    /// Block size set a memory budget for streaming (de)compression,
    /// with larger values of block size requiring more memory and typically
    /// better compression.
    ///
    /// Methods operated with all data located in memory, like CompressBuffer()
    /// or DecompressBuffer() works by default with one big block, except
    /// 'fStreamFormat' flag has specified. Stream format stores used block
    /// size, so it is used automatically for decompression instead of provided
    /// value.
    /// 
    void SetBlockSize(size_t block_size);
    size_t GetBlockSize(void) const { return m_BlockSize; }
    static size_t GetBlockSizeDefault(void);
    static size_t GetBlockSizeMin(void);
    static size_t GetBlockSizeMax(void);

protected:
    /// Initialize compression parameters.
    void InitCompression(ELevel level);

    /// Get error description for specified error code.
    const char* GetLZOErrorDescription(int errcode);

    /// Format string with last error description.
    string FormatErrorMessage(string where) const;

    /// Compress block of data.
    ///
    /// @return
    ///   Return compressor error code.
    int CompressBlock(const void* src_buf, size_t  src_len,
                            void* dst_buf, size_t* dst_len /* out */);

    /// Compress block of data for stream format (fStreamFormat flag).
    ///
    /// @return
    ///   Return compressor error code.
    int CompressBlockStream(
                      const void* src_buf, size_t  src_len,
                            void* dst_buf, size_t* dst_len /* out */);

    /// Decompress block of data.
    ///
    /// @return
    ///   Return decompressor error code.
    int DecompressBlock(const void* src_buf, size_t  src_len,
                              void* dst_buf, size_t* dst_len /* out */,
                              TLZOFlags flags);

    /// Decompress block of data for stream format (fStreamFormat flag).
    ///
    /// @return
    ///   Return decompressor error code.
    int DecompressBlockStream(
                        const void* src_buf, size_t  src_len,
                              void* dst_buf, size_t* dst_len /* out */,
                              TLZOFlags flags,
                              size_t* processed /* out */);
protected:
    size_t                        m_BlockSize;  ///< Block size for (de)compression.
    // Compression parameters
    AutoArray<char>               m_WorkMem;    ///< Working memory for compressor.
    unique_ptr<SCompressionParam> m_Param;      ///< Compression parameters.

private:
    /// Private copy constructor to prohibit copy.
    CLZOCompression(const CLZOCompression&);
    /// Private assignment operator to prohibit assignment.
    CLZOCompression& operator= (const CLZOCompression&);
};


//////////////////////////////////////////////////////////////////////////////
///
/// CLZOCompressionFile class --
///
/// Throw exceptions on critical errors.

class NCBI_XUTIL_EXPORT CLZOCompressionFile : public CLZOCompression,
                                              public CCompressionFile
{
public:
    // TODO: add compression_in_bufsize / compression_out_bufsize parameters after 
    // removing deprecated constructors only, to avoid conflicts. See zst as example.
    // JIRA: CXX-12640

    /// Constructor.
    ///
    /// Automatically calls Open() with given file name, mode and compression level.
    /// @note
    ///   This constructor don't allow to use any advanced compression parameters
    ///   or a dictionary. If you need to set any of them, please use simplified
    ///   conventional constructor, set advanced parameters and use Open().
    /// 
    CLZOCompressionFile(
        const string& file_name,
        EMode         mode,
        ELevel        level = eLevel_Default
    );
    /// Conventional constructor.
    CLZOCompressionFile(
        ELevel        level = eLevel_Default
    );
    /// @deprecated Use CLZOCompressionFile(const string&, EMode, ELevel) construtor instead.
    NCBI_DEPRECATED_CTOR(CLZOCompressionFile(
        const string& file_name,
        EMode         mode,
        ELevel        level,
        size_t        blocksize
    ));
    /// @deprecated Use CLZOCompressionFile(ELevel) construtor instead.
    NCBI_DEPRECATED_CTOR(CLZOCompressionFile(
        ELevel        level,
        size_t        blocksize
    ));

    /// Destructor
    ~CLZOCompressionFile(void);

    /// Opens a compressed file for reading or writing.
    ///
    /// @param file_name
    ///   File name of the file to open.
    /// @param mode
    ///   File open mode.
    /// @param compression_in_bufsize
    ///   Size of the internal buffer holding input data to be (de)compressed.
    /// @param compression_out_bufsize
    ///   Size of the internal buffer to receive data from a (de)compressor.
    /// @return
    ///   TRUE if file was opened successfully or FALSE otherwise.
    /// @sa
    ///   CLZOCompression, Read, Write, Close
    /// @note
    ///   All advanced compression parameters or a dictionary should be set before
    ///   Open() method, otherwise they will not have any effect.
    /// 
    virtual bool Open(
        const string& file_name, 
        EMode         mode,
        size_t        compression_in_bufsize  = kCompressionDefaultBufSize,
        size_t        compression_out_bufsize = kCompressionDefaultBufSize
    );

    /// Opens a compressed file for reading or writing.
    ///
    /// Do the same as standard Open(), but can also get/set file info.
    /// @param file_name
    ///   File name of the file to open.
    /// @param mode
    ///   File open mode.
    /// @param info
    ///   Pointer to file information structure. If not NULL, that it will
    ///   be used to get information about compressed file in the read mode,
    ///   and set it in the write mode for compressed files.
    /// @param compression_in_bufsize
    ///   Size of the internal buffer holding input data to be (de)compressed.
    /// @param compression_out_bufsize
    ///   Size of the internal buffer to receive data from a (de)compressor.
    /// @return
    ///   TRUE if file was opened successfully or FALSE otherwise.
    /// @sa
    ///   CLZOCompression, Read, Write, Close
    ///
    virtual bool Open(
        const string& file_name, 
        EMode         mode, 
        SFileInfo*    info,
        size_t        compression_in_bufsize  = kCompressionDefaultBufSize,
        size_t        compression_out_bufsize = kCompressionDefaultBufSize
    );

    /// Read data from compressed file.
    /// 
    /// Read up to "len" uncompressed bytes from the compressed file "file"
    /// into the buffer "buf". 
    /// @param buf
    ///    Buffer for requested data.
    /// @param len
    ///    Number of bytes to read.
    /// @return
    ///   Number of bytes actually read (0 for end of file, -1 for error).
    ///   The number of really read bytes can be less than requested.
    /// @sa
    ///   Open, Write, Close
    ///
    virtual long Read(void* buf, size_t len);

    /// Write data to compressed file.
    /// 
    /// Writes the given number of uncompressed bytes from the buffer
    /// into the compressed file.
    /// @param buf
    ///    Buffer with written data.
    /// @param len
    ///    Number of bytes to write.
    /// @return
    ///   Number of bytes actually written or -1 for error.
    ///   Returned value can be less than "len".
    /// @sa
    ///   Open, Read, Close
    ///
    virtual long Write(const void* buf, size_t len);

    /// Close compressed file.
    ///
    /// Flushes all pending output if necessary, closes the compressed file.
    /// @return
    ///   TRUE on success, FALSE on error.
    /// @sa
    ///   Open, Read, Write
    ///
    virtual bool Close(void);

protected:
    /// Get error code/description of last stream operation (m_Stream).
    /// It can be received using GetErrorCode()/GetErrorDescription() methods.
    void GetStreamError(void);

protected:
    EMode                  m_Mode;     ///< I/O mode (read/write).
    CNcbiFstream*          m_File;     ///< File stream.
    CCompressionIOStream*  m_Stream;   ///< [De]compression stream.

private:
    /// Private copy constructor to prohibit copy.
    CLZOCompressionFile(const CLZOCompressionFile&);
    /// Private assignment operator to prohibit assignment.
    CLZOCompressionFile& operator= (const CLZOCompressionFile&);
};


/////////////////////////////////////////////////////////////////////////////
///
/// CLZOBuffer -- 
///
/// Auxiliary base class for stream compressor/decompressor to manage
/// buffering of data for LZO blocking I/O.
///
/// @sa CLZOCompressor, CLZODecompressor

class NCBI_XUTIL_EXPORT CLZOBuffer
{
public:
    /// Constructor.
    CLZOBuffer(void);

protected:
    /// Reset internal state.
    void ResetBuffer(size_t in_bufsize, size_t out_bufsize);

private:
    size_t          m_Size;      ///< Size of in/out buffers.
    AutoArray<char> m_Buf;       ///< Buffer for caching (size of m_Size*2).
    char*           m_InBuf;     ///< Pointer to input buffer.
    size_t          m_InSize;    ///< Size of the input buffer.
    size_t          m_InLen;     ///< Length of data in the input buffer.
    char*           m_OutBuf;    ///< Pointer to output buffer. 
    size_t          m_OutSize;   ///< Size of the output buffer.
    char*           m_OutBegPtr; ///< Pointer to begin of data in out buffer.
    char*           m_OutEndPtr; ///< Pointer to end of data in out buffer.

    // Friend classes
    friend class CLZOCompressor;
    friend class CLZODecompressor;

private:
    /// Private copy constructor to prohibit copy.
    CLZOBuffer(const CLZOBuffer&);
    /// Private assignment operator to prohibit assignment.
    CLZOBuffer& operator= (const CLZOBuffer&);
};


/////////////////////////////////////////////////////////////////////////////
///
/// CLZOCompressor -- LZO based compressor
///
/// Used in CLZOStreamCompressor.
/// @sa CLZOStreamCompressor, CLZOCompression, CCompressionProcessor

class NCBI_XUTIL_EXPORT CLZOCompressor : public CLZOCompression,
                                         public CCompressionProcessor,
                                         public CLZOBuffer
{
public:
    /// Constructor.
    CLZOCompressor(
        ELevel    level = eLevel_Default,
        TLZOFlags flags = 0
    );
    /// @deprecated 
    ///   Use CLZOCompressor(ELevel = eLevel_Default, TLZOFlags = 0) constructor
    ///   without block size parameter, that can be set separately if necessary.
    NCBI_DEPRECATED_CTOR(CLZOCompressor(
        ELevel    level,
        size_t    blocksize,
        TLZOFlags flags = 0
    ));

    /// Destructor.
    virtual ~CLZOCompressor(void);

    /// Return TRUE if fAllowEmptyData flag is set. 
    /// @note
    ///   Used by stream buffer, that don't have access to specific
    ///   compression implementation flags.
    virtual bool AllowEmptyData() const
        { return (GetFlags() & fAllowEmptyData) == fAllowEmptyData; }

    /// Set information about compressed file.
    void SetFileInfo(const SFileInfo& info);

protected:
    virtual EStatus Init   (void);
    virtual EStatus Process(const char* in_buf,  size_t  in_len,
                            char*       out_buf, size_t  out_size,
                            /* out */            size_t* in_avail,
                            /* out */            size_t* out_avail);
    virtual EStatus Flush  (char*       out_buf, size_t  out_size,
                            /* out */            size_t* out_avail);
    virtual EStatus Finish (char*       out_buf, size_t  out_size,
                            /* out */            size_t* out_avail);
    virtual EStatus End    (int abandon = 0);

protected:
    /// Compress block of data in the cache buffer.
    bool CompressCache(void);

private:
    bool       m_NeedWriteHeader;  ///< TRUE if needed to write a header.
    SFileInfo  m_FileInfo;         ///< Compressed file info.
};


/////////////////////////////////////////////////////////////////////////////
///
/// CLZODecompressor -- LZO based decompressor
///
/// Used in CLZOStreamCompressor.
/// @sa CLZOStreamCompressor, CLZOCompression, CCompressionProcessor

class NCBI_XUTIL_EXPORT CLZODecompressor : public CLZOCompression,
                                           public CCompressionProcessor,
                                           public CLZOBuffer
{
public:
    /// Constructor.
    CLZODecompressor(TLZOFlags flags = 0);

    /// @deprecated 
    ///   Use CLZODecompressor(TLZOFlags = 0) constructor
    ///   without block size parameter, that can be set separately if necessary.
    NCBI_DEPRECATED_CTOR(CLZODecompressor(
        size_t    blocksize,
        TLZOFlags flags     = 0
    ));

    /// Destructor.
    virtual ~CLZODecompressor(void);

    /// Return TRUE if fAllowEmptyData flag is set. 
    /// @note
    ///   Used by stream buffer, that don't have access to specific
    ///   compression implementation flags.
    virtual bool AllowEmptyData() const
        { return (GetFlags() & fAllowEmptyData) == fAllowEmptyData; }

protected:
    virtual EStatus Init   (void); 
    virtual EStatus Process(const char* in_buf,  size_t  in_len,
                            char*       out_buf, size_t  out_size,
                            /* out */            size_t* in_avail,
                            /* out */            size_t* out_avail);
    virtual EStatus Flush  (char*       out_buf, size_t  out_size,
                            /* out */            size_t* out_avail);
    virtual EStatus Finish (char*       out_buf, size_t  out_size,
                            /* out */            size_t* out_avail);
    virtual EStatus End    (int abandon = 0);

protected:
    /// Decompress block of data in the cache buffer.
    bool DecompressCache(void);

private:
    size_t    m_BlockLen;     ///< Length of the compressed data in the block
    string    m_Cache;        ///< Buffer to cache header.

    // Parameters read from header (used for compression).
    // See fStreamFormat flag description.
    size_t    m_HeaderLen;    ///< Length of the header.
    TLZOFlags m_HeaderFlags;  ///< Flags used for compression.
};



//////////////////////////////////////////////////////////////////////////////
///
/// CLZOStreamCompressor -- lzo based compression stream processor
///
/// See util/compress/stream.hpp for details of stream processing.
/// @note
///   Stream compressor always produce data in stream format,
///   see fStreamFormat flag description.
/// @sa CCompressionStreamProcessor

class NCBI_XUTIL_EXPORT CLZOStreamCompressor
    : public CCompressionStreamProcessor
{
public:
    /// Full constructor
    CLZOStreamCompressor(
        CLZOCompression::ELevel    level,
        streamsize                 in_bufsize,
        streamsize                 out_bufsize,
        CLZOCompression::TLZOFlags flags = 0
        )
        : CCompressionStreamProcessor(
              new CLZOCompressor(level, flags), eDelete, in_bufsize, out_bufsize)
    {}

    /// @deprecated 
    ///   Use CLZOStreamCompressor() constructor
    ///   without block size parameter, that can be set separately if necessary.
    NCBI_DEPRECATED_CTOR(CLZOStreamCompressor(
        CLZOCompression::ELevel    level,
        streamsize                 in_bufsize,
        streamsize                 out_bufsize,
        size_t                     blocksize,
        CLZOCompression::TLZOFlags flags = 0
    ));

    /// Conventional constructor
    CLZOStreamCompressor(
        CLZOCompression::ELevel    level,
        CLZOCompression::TLZOFlags flags = 0
        )
        : CCompressionStreamProcessor(
              new CLZOCompressor(level, flags),
              eDelete, kCompressionDefaultBufSize, kCompressionDefaultBufSize)
    {}

    /// Conventional constructor
    CLZOStreamCompressor(CLZOCompression::TLZOFlags flags = 0)
        : CCompressionStreamProcessor(
              new CLZOCompressor(CLZOCompression::eLevel_Default, flags),
              eDelete, kCompressionDefaultBufSize, kCompressionDefaultBufSize)
    {}

    /// Return a pointer to compressor.
    /// Can be used mostly for setting an advanced compression-specific parameters.
    CLZOCompressor* GetCompressor(void) const {
        return dynamic_cast<CLZOCompressor*>(GetProcessor());
    }
};


/////////////////////////////////////////////////////////////////////////////
///
/// CLZOStreamDecompressor -- lzo based decompression stream processor
///
/// See util/compress/stream.hpp for details of stream processing.
/// @note
///   The stream decompressor always suppose that data is in stream format
///   and use fStreamFormat flag automatically.
/// @sa CCompressionStreamProcessor

class NCBI_XUTIL_EXPORT CLZOStreamDecompressor
    : public CCompressionStreamProcessor
{
public:
    /// Full constructor
    CLZOStreamDecompressor(
        streamsize                 in_bufsize,
        streamsize                 out_bufsize,
        CLZOCompression::TLZOFlags flags   = 0
        )
        : CCompressionStreamProcessor(
             new CLZODecompressor(flags), eDelete, in_bufsize, out_bufsize)
    {}

    /// @deprecated 
    ///   Use CLZOStreamDecompressor() constructor
    ///   without block size parameter, that can be set separately if necessary.
    NCBI_DEPRECATED_CTOR(CLZOStreamDecompressor(
        streamsize                 in_bufsize,
        streamsize                 out_bufsize,
        size_t                     blocksize,
        CLZOCompression::TLZOFlags flags = 0
    ));

    /// Conventional constructor
    CLZOStreamDecompressor(CLZOCompression::TLZOFlags flags = 0)
        : CCompressionStreamProcessor( 
              new CLZODecompressor(flags),
              eDelete, kCompressionDefaultBufSize, kCompressionDefaultBufSize)
    {}

    /// Return a pointer to decompressor.
    /// Can be used mostly for setting an advanced compression-specific parameters.
    CLZODecompressor* GetDecompressor(void) const {
        return dynamic_cast<CLZODecompressor*>(GetProcessor());
    }
};


END_NCBI_SCOPE


/* @} */

#endif  /* HAVE_LIBLZO */

#endif  /* UTIL_COMPRESS__LZO__HPP */
