CC       := gcc
CFLAGS   := -g -Wall
LDFLAGS  := -lpthread
INCLUDE  := -I.

# === Общие исходники для примеров ===
EXAMPLE_SRCS := queue-example.c
THREADS_SRCS := queue-threads.c

# === Бинарники ===
ALL := queue-example queue-threads-spin queue-threads-mutex queue-threads-cv queue-threads-semaphor

.PHONY: all clean run-example run-spin run-mutex

all: $(ALL)

# Пример без потоков, использует базовую очередь (если нужна)
queue-example: queue.h queue.c $(EXAMPLE_SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) queue.c $(EXAMPLE_SRCS) $(LDFLAGS) -o $@

# Паттерн: queue-threads-<impl> линкуется с queue_<impl>.c
#   ожидаются файлы: queue_spin.c, queue_mutex.c
queue-threads-%: queue.h queue_%.c $(THREADS_SRCS)
	$(CC) $(CFLAGS) $(INCLUDE) queue_$*.c $(THREADS_SRCS) $(LDFLAGS) -o $@

# Быстрые раннеры
run-example: queue-example
	./$<

run-spin: queue-threads-spin
	./$<

run-mutex: queue-threads-mutex
	./$<

run-cv: queue-threads-cv
	./$<

run-semaphor: queue-threads-semaphor
	./$<


clean:
	rm -f $(ALL) *.o
