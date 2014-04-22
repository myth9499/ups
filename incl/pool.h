/**线程池相关结构体
 *  *作者：李磊
 *   *2013.5.7
 *    */
typedef struct worker
{
	void *(*process )(void *arg);
	void *arg;
	struct worker *next;
} CThread_worker;

typedef struct
{
	pthread_mutex_t queue_lock;
	pthread_cond_t  queue_ready;

	CThread_worker *queue_head;

	int shutdown;
	pthread_t *threadid;

	int max_thread_num;
	int cur_queue_size;
	int cur_workcnt;
} CThread_pool;
