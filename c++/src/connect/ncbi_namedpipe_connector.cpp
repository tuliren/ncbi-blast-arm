/* $Id: ncbi_namedpipe_connector.cpp 652073 2022-06-30 14:11:07Z lavr $
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
 * Author:  Denis Vakatov, Anton Lavrentiev, Vladimir Ivanov
 *
 * File Description:
 *   Implement CONNECTOR for a named pipe interprocess communication
 *   (based on the NCBI CNamedPipe).
 *
 *   See in "connectr.h" for the detailed specification of the underlying
 *   connector("CONNECTOR", "SConnectorTag") methods and structures.
 *
 */

#include <ncbi_pch.hpp>
#include "ncbi_ansi_ext.h"
#include <connect/ncbi_namedpipe_connector.hpp>


USING_NCBI_SCOPE;


/***********************************************************************
 *  INTERNAL -- Auxiliary types and static functions
 ***********************************************************************/

// All internal data necessary to perform the (re)connect and i/o
typedef struct {
    CNamedPipeClient* pipe;      // pipe handle; non-NULL
    string            pipename;  // pipe name
    size_t            pipesize;  // pipe size
} SNamedPipeConnector;


/***********************************************************************
 *  INTERNAL -- "s_VT_*" functions for the "virt. table" of connector methods
 ***********************************************************************/


extern "C" {


static const char* s_VT_GetType
(CONNECTOR /*connector*/)
{
    return "NAMEDPIPE";
}


static char* s_VT_Descr
(CONNECTOR connector)
{
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    return strdup(xxx->pipename.c_str());
}


static EIO_Status s_VT_Open
(CONNECTOR       connector,
 const STimeout* timeout)
{
    _ASSERT(timeout != kDefaultTimeout);
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    return xxx->pipe->Open(xxx->pipename, timeout, xxx->pipesize);
}


static EIO_Status s_VT_Wait
(CONNECTOR       connector,
 EIO_Event       event,
 const STimeout* timeout)
{
    _ASSERT(timeout != kDefaultTimeout);
    _ASSERT(event == eIO_Read  ||  event == eIO_Write);
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    return xxx->pipe->Wait(event, timeout);
}


static EIO_Status s_VT_Write
(CONNECTOR       connector,
 const void*     buf,
 size_t          size,
 size_t*         n_written,
 const STimeout* timeout)
{
    _ASSERT(timeout != kDefaultTimeout);
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    _VERIFY(xxx->pipe->SetTimeout(eIO_Write, timeout) == eIO_Success);
    return xxx->pipe->Write(buf, size, n_written);
}


static EIO_Status s_VT_Read
(CONNECTOR       connector,
 void*           buf,
 size_t          size,
 size_t*         n_read,
 const STimeout* timeout)
{
    _ASSERT(timeout != kDefaultTimeout);
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    _VERIFY(xxx->pipe->SetTimeout(eIO_Read, timeout) == eIO_Success);
    return xxx->pipe->Read(buf, size, n_read);
}


static EIO_Status s_VT_Status
(CONNECTOR connector,
 EIO_Event dir)
{
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    return xxx->pipe->Status(dir);
}


static EIO_Status s_VT_Close
(CONNECTOR       connector,
 const STimeout* /*timeout*/)
{
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    return xxx->pipe->Close();
}


static void s_Setup
(CONNECTOR connector)
{
    SMetaConnector* meta = connector->meta;

    // Initialize virtual table
    CONN_SET_METHOD(meta, get_type, s_VT_GetType, connector);
    CONN_SET_METHOD(meta, descr,    s_VT_Descr,   connector);
    CONN_SET_METHOD(meta, open,     s_VT_Open,    connector);
    CONN_SET_METHOD(meta, wait,     s_VT_Wait,    connector);
    CONN_SET_METHOD(meta, write,    s_VT_Write,   connector);
    CONN_SET_METHOD(meta, flush,    0,            0);
    CONN_SET_METHOD(meta, read,     s_VT_Read,    connector);
    CONN_SET_METHOD(meta, status,   s_VT_Status,  connector);
    CONN_SET_METHOD(meta, close,    s_VT_Close,   connector);
    meta->default_timeout = kInfiniteTimeout;
}


static void s_Destroy
(CONNECTOR connector)
{
    SNamedPipeConnector* xxx = (SNamedPipeConnector*) connector->handle;
    connector->handle = 0;

    _ASSERT(xxx->pipe);
    delete xxx->pipe;
    xxx->pipe = 0;
    delete xxx;
    free(connector);
}


} /* extern "C" */


BEGIN_NCBI_SCOPE


/***********************************************************************
 *  EXTERNAL -- the connector's "constructors"
 ***********************************************************************/

extern CONNECTOR NAMEDPIPE_CreateConnector
(const string& pipename,
 size_t        pipesize) 
{
    auto ccc = make_c_unique((SConnector*) malloc(sizeof(SConnector)));
    if (!ccc)
        return 0;

    // Initialize internal data structures
    unique_ptr<SNamedPipeConnector> xxx(new SNamedPipeConnector);
    xxx->pipe     = new CNamedPipeClient;
    xxx->pipename = pipename;
    xxx->pipesize = pipesize;

    // Initialize connector data
    ccc->handle  = xxx.release();
    ccc->next    = 0;
    ccc->meta    = 0;
    ccc->setup   = s_Setup;
    ccc->destroy = s_Destroy;

    return ccc.release();
}


END_NCBI_SCOPE
