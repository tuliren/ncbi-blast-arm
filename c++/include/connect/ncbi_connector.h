#ifndef CONNECT___NCBI_CONNECTOR__H
#define CONNECT___NCBI_CONNECTOR__H

/* $Id: ncbi_connector.h 652387 2022-07-07 14:06:26Z lavr $
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
 * Author:  Denis Vakatov
 *
 * File Description:
 *   Specifications to implement a connector("CONNECTOR") to be used to open
 *   and handle connection("CONN", see also in "ncbi_connection.[ch]") to an
 *   abstract I/O service. This is generally not for the public use.
 *   It is to be used in the modules that implement a particular connector.
 *
 */

#include <connect/ncbi_core.h>


/** @addtogroup Connectors
 *
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif


struct SConnectorTag;
typedef struct SConnectorTag* CONNECTOR;  /**< connector handle */


/** DEF_CONN_TIMEOUT as STimeout */
extern NCBI_XCONNECT_EXPORT const STimeout g_NcbiDefConnTimeout;


/* Function type definitions for the connector method table.
 * The arguments & behavior of "FConnector***" functions are mostly just the
 * same as those for their counterparts "CONN_***" in ncbi_connection.h.  The
 * first argument of these functions accepts the real connector handle rather
 * than an upper-level connection handle("CONN").
 * In every call that takes STimeout as an argument, the argument can be either
 * NULL (for infinite timeout, kInfiniteTimeout) or a valid non-NULL pointer
 * that points to a finite timeout structure.  Note that kDefaultTimeout gets
 * resolved at the level of the connection, and is never passed through.
 */


/** Get the name of the connector (may NOT be NULL)
 */
typedef const char* (*FConnectorGetType)
(CONNECTOR       connector
 );


/** Get the human readable connector's description (may be NULL on error)
 */
typedef       char* (*FConnectorDescr)
(CONNECTOR       connector
 );


/** Open connection.  Used to setup all related data structures, but not
 * necessarily has to actually open the data channel.
 * @note  Regardless of the returned status, the connection is considered open
 *        (so this call doesn't get re-issued) after this call returns.
 */
typedef EIO_Status (*FConnectorOpen)
(CONNECTOR       connector,
 const STimeout* timeout
 );


/** Wait until either read or write (depending on the "event" value) becomes
 * available, or until "timeout" expires, or until error occurs.
 * @note  The passed "event" is guaranteed to be either eIO_Read or eIO_Write.
 * @note  FConnectorWait() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorWait)
(CONNECTOR       connector,
 EIO_Event       event,
 const STimeout* timeout
 );


/** Write to connector.
 * The passed "n_written" is always non-NULL, and "*n_written" is always zero.
 * Upon return, the number of bytes actually written must get reflected in
 * "*n_written", and it may never be greater than "size".
 * @warning  This call may not return eIO_Success if no data at all have been
 *           written (unless "size" was passed as 0).
 * @note  FConnectorWrite() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorWrite)
(CONNECTOR       connector,
 const void*     buf,
 size_t          size,
 size_t*         n_written,
 const STimeout* timeout
 );


/** Flush yet unwritten output data, if any.
 * @note  FConnectorFlush() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorFlush)
(CONNECTOR       connector,
 const STimeout* timeout
 );


/** Read from connector.
 * The passed "n_read" is always non-NULL, and "*n_read" is always zero.  Upon
 * return, the number of bytes actually read must get reflected in "*n_read",
 * and it may never be greater than "size".
 * @warning  This call may not return eIO_Success if no data at all have been
             read (unless "size" was passed 0).
 * @note  This call should use the eIO_Closed return code solely to indicate
 *        true EOF in data;  and never for other read errors (such as transport
 *        or medium issues of any sort, like being unable to open a file or an
 *        underlying connection, if any;  or being unable to negotiate).
 * @note  FConnectorRead() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorRead)
(CONNECTOR       connector,
 void*           buf,
 size_t          size,
 size_t*         n_read,
 const STimeout* timeout
 );


/** Obtain last I/O completion code from the transport level (connector).
 * @note  "direction" is guaranteed to be either eIO_Read or eIO_Write.
 * @note  This call should return eIO_Success in case of nonexistent or
 *        yet-incomplete low level transport, if any.
 * @note  FConnectorStatus() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorStatus)
(CONNECTOR       connector,
 EIO_Event       direction
 );
          

/** Close data link (if any) and cleanup related data structures.
 * @note  FConnectorFlush() gets called before FConnectorClose() automatically.
 * @note  It may return eIO_Closed to indicate an unusual close condition.
 * @note  The same connector may now be either re-opened / reused or recycled.
 * @note  FConnectorClose() is guaranteed to be called after FConnectorOpen(),
 *        and only if the latter succeeded (returned eIO_Success).
 */
typedef EIO_Status (*FConnectorClose)
(CONNECTOR       connector,
 const STimeout* timeout
 );


/** Standard set of connector methods to handle a connection (corresponding
 * connectors are also in here), part of the connection handle ("CONN").
 * @sa
 *  CONN
 */
typedef struct {
    FConnectorGetType get_type;  CONNECTOR c_get_type;
    FConnectorDescr   descr;     CONNECTOR c_descr;
    FConnectorOpen    open;      CONNECTOR c_open;
    FConnectorWait    wait;      CONNECTOR c_wait;
    FConnectorWrite   write;     CONNECTOR c_write;
    FConnectorFlush   flush;     CONNECTOR c_flush;
    FConnectorRead    read;      CONNECTOR c_read;
    FConnectorStatus  status;    CONNECTOR c_status;
    FConnectorClose   close;     CONNECTOR c_close;
    const STimeout*   default_timeout;  /**< default timeout pointer     */
    STimeout          default_tmo;      /**< storage for default_timeout */
    CONNECTOR         list;
} SMetaConnector;


#define CONN_SET_METHOD(meta, method, function, connector) \
    do {                                                   \
        meta->method     = function;                       \
        meta->c_##method = connector;                      \
    } while (0)


#define CONN_SET_DEFAULT_TIMEOUT(meta, timeout)            \
    do {                                                   \
        if (timeout == kDefaultTimeout) {                  \
            meta->default_timeout = &g_NcbiDefConnTimeout; \
        } else if (timeout) {                              \
            meta->default_tmo     = *timeout;              \
            meta->default_timeout = &meta->default_tmo;    \
        } else                                             \
            meta->default_timeout = kInfiniteTimeout/*0*/; \
    } while (0)


/** Insert a connector in the beginning of the connection's list of connectors.
 * Calls connector's FSetupVTable, which must be defined.
 */
extern NCBI_XCONNECT_EXPORT EIO_Status METACONN_Insert
(SMetaConnector* meta,
 CONNECTOR       connector
 );


/** Delete given "connector" all its descendants (all connectors if "connector"
 * is NULL) from the connections's list of connectors.  FDestroy (if defined)
 * gets called for each removed connector.
 */
extern NCBI_XCONNECT_EXPORT EIO_Status METACONN_Remove
(SMetaConnector* meta,
 CONNECTOR       connector
 );


/** Upcall on request to setup virtual function table (called from connection).
 * NB:  May not detect any failures (follow up in Open to fail if necessary).
 */
typedef void (*FSetupVTable)
(CONNECTOR       connector
 );


/** Destroy connector and its data handle.  This is NOT a close request!
 * Should not to be used on open connectors (that is, for those
 * FConnectorClose must be called prior to this call).
 */
typedef void (*FDestroy)
(CONNECTOR       connector
 );


/** Connector specification.
 */
typedef struct SConnectorTag {
    SMetaConnector* meta;     /**< back link to original meta   */
    FSetupVTable    setup;    /**< init meta, may not be NULL   */
    FDestroy        destroy;  /**< destroys handle, can be NULL */
    void*           handle;   /**< data handle of the connector */
    CONNECTOR       next;     /**< linked list                  */
} SConnector;


#ifdef __cplusplus
}  /* extern "C" */
#endif


/* @} */

#endif /* CONNECT___NCBI_CONNECTOR__H */
