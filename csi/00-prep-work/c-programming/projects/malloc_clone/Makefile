CC := clang
CFLAGS := -std=c23 -Wall -Wextra -Werror -pedantic -g

BINS := stage1_implicit stage2_explicit stage3_pageheap stage4_central stage5_threadcache

.PHONY: all clean check_compile

all: $(BINS)

stage1_implicit: stage1_implicit_coalescer.c
	$(CC) $(CFLAGS) -o $@ $<

stage2_explicit: stage2_explicit_freelist.c
	$(CC) $(CFLAGS) -o $@ $<

stage3_pageheap: stage3_tcmalloc_pageheap.c
	$(CC) $(CFLAGS) -o $@ $<

stage4_central: stage4_tcmalloc_central_freelist.c
	$(CC) $(CFLAGS) -o $@ $<

stage5_threadcache: stage5_tcmalloc_threadcache.c
	$(CC) $(CFLAGS) -pthread -o $@ $<

check_compile: $(BINS)
	@echo "[OK] All 5 stage starter skeletons compiled cleanly under $(CC) $(CFLAGS)"

clean:
	rm -f $(BINS)
	rm -rf *.dSYM
