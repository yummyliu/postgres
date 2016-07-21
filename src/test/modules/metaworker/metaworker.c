#include "postgres.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/proc.h"
#include "fmgr.h"
#include "libpq-fe.h"
/* Essential for shared libs! */
PG_MODULE_MAGIC;

/* Entry point of library loading */
void _PG_init(void);
void  metaworker_main(Datum) pg_attribute_noreturn();

/* Signal handling */
static bool got_sigterm = false;

/* pg connect */
PGconn	   *conn;
static void
exit_nicely(PGconn *conn)
{
	PQfinish(conn);
	exit(1);
}
static void
metaworker_sigterm(SIGNAL_ARGS)
{
    int         save_errno = errno;
    got_sigterm = true;
    if (MyProc)
        SetLatch(&MyProc->procLatch);
    errno = save_errno;
    exit_nicely(conn);
}



static void
check_conn(PGconn *conn, const char *dbName)
{
	/* check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK)
	{
		fprintf(stderr, "Connection to database \"%s\" failed: %s",
				dbName, PQerrorMessage(conn));
	    /* close the connection to the database and cleanup */
	}
}
static void
connectdb(char* dbname){
/* connect to database test */
	const char conninfo[20];
    sprintf(conninfo,"dbname = %s",dbname);
    elog(LOG,"start connect\n");
    do{
        conn = PQconnectdb(conninfo);
    }while(PQstatus(conn) != CONNECTION_OK);
    elog(LOG,"connect ok\n");
}

static int 
setsocket(int port){
    /* listen port:8611 for client sql statement*/
    int listenfd;
	struct sockaddr_in servaddr;
	int n;
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s(errno: %d) port: %d\n",
                strerror(errno),
                errno,
                servaddr.sin_port);
		exit(0);
	}else{
        printf("bind on %d \n",
                servaddr.sin_port);
    }
	if( listen(listenfd, 10) == -1){
		printf("listen socket error: %s(errno: %d)\n",
                strerror(errno),
                errno);
		exit(0);
	}
    return listenfd;
}

void
metaworker_main(Datum main_arg)
{
    pqsignal(SIGTERM, metaworker_sigterm);
    /* ready to receive signals */
    BackgroundWorkerUnblockSignals();

    int listenfd = setsocket(8611); 
    connectdb("postgres");

    while (!got_sigterm)
    {
        int     rc;
        /* Wait 10s */
        rc = WaitLatch(&MyProc->procLatch,
                WL_LATCH_SET | WL_TIMEOUT | WL_POSTMASTER_DEATH,
                10000L);
        ResetLatch(&MyProc->procLatch);    
        int connfd;
		if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
			elog(LOG,"accept socket error: %s(errno: %d)",strerror(errno),errno);
            exit(0);
        }
	    char buff[4096];
		int n = recv(connfd, buff, 4096, 0);
		buff[n] = '\0';
		elog(LOG, "get msg: %s\n",buff);

	    PGresult   *res;
        res = PQexec(conn,buff);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            elog(LOG,"failed: %s\n",buff);
        }
    }
    /*
     * add :
     * kill all subproc
     */
	/* close the connection to the database and cleanup */
	exit_nicely(conn);
}

/*
 * Entrypoint of this module.
 *
 * We register more than one worker process here, to demonstrate how that can
 * be done.
 */
void
_PG_init(void)
{
    BackgroundWorker    worker;

    /* Register the worker processes */
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS |
		BGWORKER_BACKEND_DATABASE_CONNECTION;
    worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
    worker.bgw_main = metaworker_main;
    worker.bgw_notify_pid = 0;
    snprintf(worker.bgw_name, BGW_MAXLEN, "metaworker");
    worker.bgw_restart_time = BGW_NEVER_RESTART;
    worker.bgw_main_arg = Int32GetDatum(NULL);
    RegisterBackgroundWorker(&worker);
}
