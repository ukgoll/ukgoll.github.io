include ../utils.mk
umt: umask_test.c
	clang umask_test.c -o umt
rrc: random_rv.c
	clang -o $@ $<
hsv: heap_stack_variable.c
	clang -o $@ $<
aligna: address_align.c
	clang -o $@ $<
fbu: fork_buff.c
	clang -o $@ $<
fkw: fork_wait.c
	clang -o $@ $<
pct: parent_child_tell.c
	clang -o $@ $<
rl: resource_limit.c ../utils.o
	clang -o $@ $^
pgc: proc_grp.c ../utils.o
	clang -o $@ $^
pow: pipe_oversize_write.c
	clang -o $@ $<
