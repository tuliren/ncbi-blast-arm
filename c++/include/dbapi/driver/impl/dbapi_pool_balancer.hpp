#ifndef DBAPI_DRIVER___DBAPI_POOL_BALANCER__HPP
#define DBAPI_DRIVER___DBAPI_POOL_BALANCER__HPP

/*  $Id: dbapi_pool_balancer.hpp 628113 2021-03-24 20:36:42Z ucko $
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
 * Author:  Aaron Ucko
 *
 */

/// @file dbapi_pool_balancer.hpp
/// Help distribute connections within a pool across servers.

#include <dbapi/driver/impl/dbapi_driver_utils.hpp>

/** @addtogroup DBAPI
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class IDBServiceInfo : public CObject
{
public:
    typedef IDBServiceMapper::TOptions             TOptions;
    typedef SSimpleLock<IDBServiceInfo>            TLock;
    typedef SSimpleUnlock<IDBServiceInfo>          TUnlock;
    typedef CGuard<IDBServiceInfo>                 TGuard;
    typedef CGuard<IDBServiceInfo, TUnlock, TLock> TAntiGuard;
    
    virtual const string& GetServiceName(void) const = 0;
    virtual const TOptions& GetOptions(void) = 0;
    virtual void Lock(void) = 0;
    virtual void Unlock(void) = 0;
};


class CDBPoolBalancer : public CPoolBalancer
{
public:
    CDBPoolBalancer(IDBServiceInfo& service_info,
                    const string& pool_name,
                    I_DriverContext* driver_ctx = nullptr);

    TSvrRef GetServer(CDB_Connection** conn, const CDBConnParams* params);

protected:
    IBalanceable* x_TryPool(const void* params) override;
    unsigned int  x_GetCount(const void* params, const string& name) override;
    unsigned int  x_GetPoolMax(const void* params) override;
    void          x_Discard(const void* params, IBalanceable* conn) override;

private:
    void x_ReinitFromCounts(void);
    
    CRef<IDBServiceInfo> m_ServiceInfo;
    string               m_PoolName;
    I_DriverContext*     m_DriverCtx;
};


inline
TSvrRef CDBPoolBalancer::GetServer(CDB_Connection** conn,
                                   const CDBConnParams* params)
{
    IBalanceable* balanceable = nullptr;
    auto svr_ref = x_GetServer(params, &balanceable);
    if (conn != nullptr) {
        *conn = static_cast<CDB_Connection*>(balanceable);
    }
    return svr_ref;
}


END_NCBI_SCOPE

/* @} */

#endif  /* DBAPI_DRIVER___DBAPI_POOL_BALANCER__HPP */
