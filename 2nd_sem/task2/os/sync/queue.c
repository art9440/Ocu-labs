#define _GNU_SOURCE
#include <pthread.h>
#include <assert.h>

#include "queue.h"

void *qmonitor(void *arg) {
	queue_t *q = (queue_t *)arg;

	printf("qmonitor: [%d %d %d]\n", getpid(), getppid(), gettid());

	while (1) {
		queue_print_stats(q);
		sleep(1);
	}

	return NULL;
}

queue_t* queue_init(int max_count) {
	int err;

	queue_t *q = malloc(sizeof(queue_t));
	if (!q) {
		printf("Cannot allocate memory for a queue\n");
		abort();
	}

	q->first = NULL;
	q->last = NULL;
	q->max_count = max_count;
	q->count = 0;

	q->add_attempts = q->get_attempts = 0;
	q->add_count = q->get_count = 0;

	err = pthread_create(&q->qmonitor_tid, NULL, qmonitor, q);
	if (err) {
		printf("queue_init: pthread_create() failed: %s\n", strerror(err));
		abort();
	}

	return q;
}

void queue_destroy(queue_t *q) {
	if (!q) return;

	int rc = pthread_cancel(q->qmonitor_tid);
	(void)rc;
	pthread_join(q->qmonitor_tid, NULL);

	qnode_t *n = q->first;
    while (n) {
        qnode_t *next = n->next;
        free(n);
        n = next;
    }

	free(q);
}

int queue_add(queue_t *q, int val) {
	q->add_attempts++;

	assert(q->count <= q->max_count);

	if (q->count == q->max_count)
		return 0;

	qnode_t *new = malloc(sizeof(qnode_t));
	if (!new) {
		printf("Cannot allocate memory for new node\n");
		abort();
	}

	new->val = val;
	new->next = NULL;

	if (!q->first)
	//ставим голову/хвост на new.
        // Если между проверкой и записью читатель что-то добавил/убрал — это всё ещё гонка,
        // но эта ветка не разыменовывает старые указатели.
		q->first = q->last = new;  
	else { 
		 // Был хотя бы один элемент. Стараемся минимально трогать старые узлы.
        if (q->last) {
            // Пытаемся аккуратно «пришить» new к хвосту.
            // Это единственное место, где мы трогаем старый хвост.
            q->last->next = new;   // остаётся потенциальный UAF при тяжёлой гонке
			//в какой момент может не упасть
            q->last = new;         // замена для: без q->last = q->last->next; =>
									//убираем одно разыменовывание старого хвоста
		}else {
			// Неконсистентное состояние (first != NULL, last == NULL):
            // лечим как «очередь из одного узла» — пришьём к first.
            if (q->first && q->first->next == NULL) {
                q->first->next = new;
                q->last = new;
            } else {
                // Совсем «сломано» — кладём new как новый единственный.
                // чтобы не падать.
                q->first = q->last = new;
            }
		}
	}

	q->count++;
	q->add_count++;

	return 1;
}

int queue_get(queue_t *q, int *val) {
	q->get_attempts++;

	// Ранняя проверка на пустоту (чтоб не разыменовать NULL из-за гонки)
    if (q->count <= 0 || q->first == NULL)
        return 0;

	assert(q->count >= 0);

	if (q->count == 0)
		return 0;

	qnode_t *tmp = q->first;
	if (!tmp)            // дополнительная страховка
        return 0;


	*val = tmp->val;
	q->first = tmp->next; //Замена для : q->first->next; 
						  //чтобы не обращаться к возможному, уже измененному q->first

	 // если стало пусто — ОБНУЛЯЕМ last, так как до этого мы сделали tmp->next и если он NULL
	 //то значит в очереди нет элементов
    if (q->first == NULL)
        q->last = NULL;

	free(tmp);

	// Оборона от отрицательных значений из-за гонок
    if (q->count > 0) q->count--;
    if (q->get_count < q->add_count) q->get_count++;

	return 1;
}

void queue_print_stats(queue_t *q) {
	printf("queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld %ld %ld)\n",
		q->count,
		q->add_attempts, q->get_attempts, q->add_attempts - q->get_attempts,
		q->add_count, q->get_count, q->add_count -q->get_count);
}



