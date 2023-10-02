#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mypipe.h"

struct mypipe_st
{
	int head;
	int tail;
	char data[PIPISIZE];
	int datasize; 			// 表示当前管道数据个数
	int count_rd;			// 读取数据者计数
	int count_wr;			// 写数据者计数
	pthread_mutex_t mut;	// 互斥锁，保护对管道的读和写，两者都应该是阻塞的，使用通知法
	pthread_cond_t cond;	// 通知法使用条件变量
};



mypipe_t *mypipe_init(void)//对管道的初始化
{
	struct mypipe_st *me;
	me = malloc(sizeof(*me));
	if(me == NULL)
		return NULL;
	me->head = 0;
	me->tail = 0;
	me->datasize = 0;
	me->count_rd = 0;
	me->count_wr = 0;
	pthread_mutex_init(&me->mut, NULL);//初始化互斥锁和条件变量
	pthread_cond_init(&me->cond, NULL);

	return me;

}

static int mypipe_readbyte_unlock(struct mypipe_st *me, char *datareader)//读取一个字节到datareader中
{
	if(me->datasize <= 0)
		return -1;

	*datareader = me->data[me->head];

	me->head = next(me->head);
	me->datasize --;

	return 0;
}

int mypipe_register(mypipe_t *ptr, int opmap)//对加入到管道中的使用者进行注册，确定他们是想要读内容还是写内容，以便进行操作
{
	/*判断opmap*/


	pthread_mutex_lock(&me->mut);
	if(opmap & MYPIPE_READ)
		me->count_rd++;
	if(opmap & MYPIPE_WRITE)
		me->count_wr++;

	pthread_cond_broadcast(&me->cond);//有人加入，通知在等待的人

	// 判断管道有效
	while(me->count_rd <= 0 || me->count_wr <= 0)
		pthread_cond_wait(&me->cond, &me->mutex);//管道中没人写或者读，等待有人来写或者读的命令提示

	pthread_mutex_unlock(&me->mut);
	return 0;
}

int mypipe_unregister(mypipe_t *ptr, int opmap)//有使用者退出，更新状态
{
	/*判断opmap*/


	pthread_mutex_lock(&me->mut);
	if(opmap & MYPIPE_READ)
		me->count_rd--;
	if(opmap & MYPIPE_WRITE)
		me->count_wr--;

	pthread_cond_broadcast(&me->cond);//有人退出，同样通知

	// // 判断管道有效
	// while(me->count_rd <= 0 || me->count_wr <= 0)
	// 	pthread_cond_wait(&me->cond, &me->mutex);

	pthread_mutex_unlock(&me->mut);
	return 0;

}



//仿照系统io--read
int mypipe_read(mypipe_t *ptr, void *buf, size_t count)
{
	struct mypipe_st *me = ptr;
	size_t i;

	pthread_mutex_lock(&me->mut);

	// 阻塞读
	while(me->datasize <= 0 && me->count_wr > 0) // 管道空着，没有数据可读，且有写数据的人
		pthread_cond_wait(&me->cond, &me->mut); // 解锁等待，等待管道中有数据

	if(me->datasize <= 0 && me->count_wr <= 0)
	{
		pthread_mutex_unlock(&me->mut);
		return 0;//没有数据且也没有人写数据了，就应该退出了
	}

	for(i = 0; i < count; i++)
	{
		if(mypipe_readbyte_unlock(me, buf+i) != 0)
			break;
	}

	pthread_cond_broadcast(&me->cond); // 惊群通知
	pthread_mutex_unlock(&me->mut);

	return i;
}


int mypipe_write(mypipe_t *ptr, const void *buf, size_t count)
{
	struct mypipe_st *me = ptr;

	pthread_mutex_lock(&me->mut);

	// 管道慢且有读者，才等待

	pthread_mutex_unlock(&me->mut);

	return n;
}

int mypipe_destroy(mypipe_t *ptr)
{
	struct mypipe_st *me = ptr;
	free(me);
	pthread_mutex_destroy(&me->mut);
	pthread_cond_destroy(&me->cond);

	return 0;
}
