//////////////////////////////////////////////////////////////////////
// DistributedLock.cpp: 
//////////////////////////////////////////////////////////////////////

#include "DistributedLock.h"
#include "logger.h"
#include "base/common.h"
#include <string>
#include <sys/timeb.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char*   log_Time(void)
{
	struct  tm      *ptm;
	struct  timeb   stTimeb;
	static  char    szTime[19];

	ftime(&stTimeb);
	ptm = localtime(&stTimeb.time);
	sprintf(szTime, "%02d-%02d %02d:%02d:%02d.%03d",
		ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, stTimeb.millitm);
	szTime[18] = 0;
	return szTime;
}

CDistributedLock::CDistributedLock(server::mysqldb::MySQLConnection* c,const std::string& path,uint64_t key,uint16_t partitions) 
{
	m_bLocked = false ;
	m_bDbDead = false;

	con_ = c ;
	if ( c )
	{
		server::mysqldb::Statement st = c->createStatement() ;
		char szSQL[256] ;

		sprintf(szSQL,"insert into %s_%d (lock_key,lock_ts) values (" UINT64FORMAT ",now()) on duplicate key update lock_ts=now()",path.c_str(),uint16_t(key % partitions), key) ;
		
		st.prepare(szSQL) ;
        int errcode = 0;
        std::string errwhat;
        for(int i = 0; i < 3; ++i) {
            try {
                server::mysqldb::ResultSet result = st.execute();
				if (result.getAffectedRows() > 0)
				{
					LogTrace(Info, "Distributed lock get succeed , affectedRows %d tms %s", result.getAffectedRows(), log_Time());
					m_bLocked = true;
				}
				else{
					LogTrace(Warn, "Distributed lock get failed , affectedRows %d tms %s", result.getAffectedRows(), log_Time());
				}
                break;
            } catch (server::mysqldb::Exception &e) {
                c->reconnect();
                errcode = e.code();
                errwhat = e.what();
				LogTrace(Error, "statemt error, %d %s try again !", errcode, errwhat.c_str());
            }
        }
        if(!m_bLocked) {
            LogTrace(Error, "sql execute error, code:%d, reason:%s", errcode, errwhat.c_str());
        }
	}else
	{
		LogTrace(Error, "get distributed lock failed, lock db is gone !");
		m_bDbDead = true;
	}
}


CDistributedLock::~CDistributedLock()
{	
	if ( con_ ) 
	{
		try {
			m_bLocked ? con_->commit() : con_->rollback() ;
		} catch (server::mysqldb::Exception &e) {	
			LogTrace(Error, "commit execute error, code:%d, reason:%s", e.code(), e.what());	
			con_->close() ;
		}
	}
}
