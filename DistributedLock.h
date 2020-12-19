#ifndef DistributedLock_H_
#define DistributedLock_H_

//////////////////////////////////////////////////////////////////////
// DistributedLock.h: 
//////////////////////////////////////////////////////////////////////
#include "base/MySQLDriver.h"
class CDistributedLock
{
public:
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	CDistributedLock(server::mysqldb::MySQLConnection* c, const std::string& path, uint64_t key, uint16_t partitions = 16);
	~CDistributedLock();

	operator bool()
	{
		return m_bLocked;
	}

	bool getLocked()
	{
		return m_bLocked;
	}

	bool getDbDeaded()
	{
		return m_bDbDead;
	}

private:
	server::mysqldb::MySQLConnection* con_;
	bool		m_bLocked; //获取分布式锁是否成功
	bool		m_bDbDead;  //数据库是否是否宕机
};
#endif
