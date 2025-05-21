---
title: "Libaco coroutine"
date: 2025-05-15 09:00:00 +0800
categories: [C, Async]
tags: [C, Async]
---

# Preface
学习 `libaco` 的源码，了解一个 coroutine 是如何调度实现的。学习如何使用 `libaco` 编译异步代码。

[libaco 源代码地址](https://github.com/hnes/libaco)，原来是想学习 [腾讯的 libco](https://github.com/Tencent/libco) 的，但是由于是 c++，目前我还在使用 C 语言学习 Linux 编程，所以开始学习了解 `libaco` C 语言编写的 coroutine。

# libaco 
libaco 的源代码比较简单，三个文件 `aco.h`, `aco.c` 和 `acosw.S`，主要的核心实现是 `asosw.S` 汇编代码，手动切换当前执行 stack。


[libaco 官网](https://libaco.org/docs/description) 中介绍了 libaco 的设计里面和基本使用，每个线程都有一个 `main coroutine` 和 多个 `no-main coroutine`。

libaco 的代码库里面支持 `x86` 32 平台 和 `x86_64` 64 位平台，没有看懂支持 arm 架构的，不过在 pr 里面有，可以补充，测试一下。

我不会汇编代码，所以通过 AI 学习理解 `libaco` 的 `x86_64` 汇编代码，首先我们需要了解的是，libaco coroutine 在同一个时间点还是只是一段程序在运行，还是串行的代码，是通过汇编代码手动来切换，更加的快捷。切换的主要的函数就是 `acosw` 函数，在 `aco_resume`(`main coroutine` 使用切换到其他线程) 和 `aco_yield`  `no-main coroutine` 使用，切换到 `main coroutine` 核心都是使用 `acosw` 函数切换。

所有的都是从 x86_64 64系统学习。

## 结构 context 介绍
在学习 libaco 之前，`save stack` 和 `share stack` 的概念要理解一下，对于切换过程的理解是比较重要的。[libaco 的官网](https://libaco.org/docs/description/)的图文搭配解释的非常的清楚，所有的 coroutine 分为 `main coroutine` 和 `no-main coroutine`，`main coroutine` 和当前线程使用同样的 `stack`，其余的 `no-main coroutine` 都是按照情况使用 `share stack`，和 `main coroutine` 不共用（这里我这么记录的原因是：**官网上所说的和配图都是所有的 `no-main coroutine` 都是共用一段 `share stack`，但是按照源码来看，每个 `no-main coroutine` 创建的时候都需要创建一个 `struct aco_share_stack_t`，不能为空，当创建好一个 `share stack` 之后，比如创建 3 个 `no-main coroutine` 都是使用这个 `share stack`，那么就是公用的，就附和官网所说的 `共用`，这种情况下每次切换 coroutine 的时候，这三个 coroutine 都要保存 share-stack 自己使用的痕迹；如果是一个 `no-main coroutine` 使用一个 `share stack` 那么就是 `standalone coroutine`，每次调用的时候或者 挂起的时候都不要保存使用痕迹到自己的 `aco_save_stack_t`**

1. `save stack` 这是用于 `no-main coroutine` 和 其他 `no-main coroutine` 共享 `share_stack` 的时候，启动 resume 或者挂起 yield 的时候，保存在 `share_stack` 中使用的内存到自己身上。
2. `share stack` 如上所述，`no-main coroutine` 和 `no-main coroutine` 可以共享 `share_stack` 能显著的降低内存损耗。

```c

struct aco_s{
    // cpu registers' state
    // 定义 reg 数字来保存 cpu register 的状态
    #ifdef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
        void*  reg[8];
    #else
        void*  reg[9];
    #endif
    aco_t* main_co; // `main coroutine` 指向 NULL，`no main coroutine` 指向 `main coroutine`
    void*  arg; // coroutine 的参数
    char   is_end; // 是否结束

    aco_cofuncp_t fp; // coroutine 对应调用的函数
    aco_save_stack_t  save_stack;
    aco_share_stack_t* share_stack;
};


typedef struct {
    void*  ptr;
    size_t sz;
    size_t valid_sz; // coroutine 实际使用的 share_stack 的大小(如果是在共享的情况下)
    // max copy size in bytes，会伸缩的
    size_t max_cpsz;
    // copy from share stack to this save stack
    size_t ct_save;
    // copy from this save stack to share stack 
    size_t ct_restore;
} aco_save_stack_t;


typedef struct {
    void*  ptr; // 在启用 guard page 之后的 mmap 分配的地址减去 guard page size, 否则就是 sz 的大小的的内存地址
    size_t sz; // 在 启用 guard page 之后，会重新计算 传入的 sz，否则就是传入的 sz
    void*  align_highptr; // 最高的地址，ptr 地址加上 sz 的大小减去该平台 两个指针的大小  (uintptr_t)(p->sz - (sizeof(void*) << 1) + (uintptr_t)p->ptr)
    void*  align_retptr;  // 保存 return 检测函数的指针，等于 align_highptr - sizeof(void *);
    size_t align_validsz; // 
    size_t align_limit; // 能用的大小
    aco_t* owner; // 当前的 share_stack 被谁使用了，在 `aco_resume` 切换的时候才会指定

    char guard_page_enabled;
    void* real_ptr; // 在启用 guard page 之后的分配的开始地址
    size_t real_sz; // 在启用 guard page 之后的分配的真实 size
} aco_share_stack_t;
```

## aco_create
这个函数还是比较好理解，和汇编打交道的时候，再记录 `p->reg[ACO_REG_IDX_RETADDR]` 等等。
```c
typedef void (*aco_cofuncp_t)(void);

aco_t* aco_create(
        aco_t* main_co, aco_share_stack_t* share_stack,
        size_t save_stack_sz, aco_cofuncp_t fp, void* arg
    ){

    aco_t* p = (aco_t*)malloc(sizeof(aco_t));
    assertalloc_ptr(p);
    // 常规操作
    memset(p, 0, sizeof(aco_t));
    // 创建 no-main coroutine
    if(main_co != NULL){ // non-main co
        assertptr(share_stack);
        p->share_stack = share_stack;
        // 这里会和 `acosw` 汇编函数打上交道
        p->reg[ACO_REG_IDX_RETADDR] = (void*)fp;
        // rsp register
        p->reg[ACO_REG_IDX_SP] = p->share_stack->align_retptr;
        // 
        #ifndef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
            p->reg[ACO_REG_IDX_FPU] = aco_gtls_fpucw_mxcsr[0];
        #endif
        // no-main coroutine 的 main_co 指向 main coroutine 很合理。
        p->main_co = main_co;
        p->arg = arg;
        p->fp = fp;
        if(save_stack_sz == 0){
            save_stack_sz = 64;
        }
        p->save_stack.ptr = malloc(save_stack_sz);
        assertalloc_ptr(p->save_stack.ptr);
        p->save_stack.sz = save_stack_sz;
        p->save_stack.valid_sz = 0;
        return p;
    } else { // main co
        p->main_co = NULL;
        p->arg = arg;
        p->fp = fp;
        p->share_stack = NULL;
        p->save_stack.ptr = NULL;
        return p;
    }
    assert(0);
}
```
`main coroutine` 和 `no-main coroutine` 都是通过此函数调用创建，`main coroutine` 的创建是固定的 `aco_create(NULL, NULL, 0, NULL, NULL)`，正如上面说的，`main coroutine` 和当前线程共享，所以不要传递 `share_stack` 等参数，对于 `save_stack_sz` 根据情况分配，传递 0 的话默认给你分 `2M`，后面会记录到，有内存对齐优化的。`co_fp` 则是 coroutine 需要调用的函数，arg 则是参数，这个函数和 `pthread_create` 的创建概念还是比较类似的。

## aco_share_stack_new2
这个就是函数是给 `no-main coroutine` `创建 share stack` 使用，如上所记录的，**多个 `no-main coroutine` 可以共享同一个 share stack，在这种情况下，每次 yield 和 resume 都要保存执行状态到 `save_stack`**

> The end of the input argument area shall be aligned on a 16 (32, if __m256 is passed on stack) byte boundary. In other words, the value (%rsp + 8) is always a multiple of 16 (32) when control is transferred to the function entry point. The stack pointer, %rsp, always points to the end of the latest allocated stack frame. 

所以如下代码就是为了 16 位对齐的，16 = 2 ** 4，所以有了如下操作
```c
// aco_share_stack_new2
u_p = (u_p >> 4) << 4; // 这个是为了 16 字节对齐, x86_64 abi 要求 rsp 入口需要是 16 位对齐的
p->align_highptr = (void*)u_p; // 这个 share_stack 最高位置
p->align_retptr  = (void*)(u_p - sizeof(void*));

// aco_create
// #define ACO_REG_IDX_RETADDR 4
// #define ACO_REG_IDX_SP 5
p->reg[ACO_REG_IDX_RETADDR] = (void*)fp;
p->reg[ACO_REG_IDX_SP] = p->share_stack->align_retptr;

// acosw.S
mov     rax,QWORD PTR [rsi+0x20] // retaddr
mov     rcx,QWORD PTR [rsi+0x28] // rsp  
// ...
mov     rsp,rcx
jmp     rax

/*
rsp = [rdi+0x28] = rdi->reg[5] = p->share_stack->align_retptr
而在  p->share_stack->align_retptr 和 p->share_stack->align_highptr 之间差一个 sizeof(void *)， 在 64 位平台上刚好是 8 bit，所以
rsp + 8 = p->share_stack->align_highptr // 在前面已经 u_p = (u_p >> 4) << 4 手动让他 16 位对齐了
*/
```



```c
aco_share_stack_t* aco_share_stack_new2(size_t sz, char guard_page_enabled){
	  // 默认 2 M
    if(sz == 0){
        sz = 1024 * 1024 * 2;
    }
	  // 最小 4 kb
    if(sz < 4096){
        sz = 4096;
    }
    assert(sz > 0);
    size_t u_pgsz = 0;
    if(guard_page_enabled != 0){ // 在启用保护页机制的情况下，需要计算 sz 为下面 guardsize 添加空间，如果 sz 是 u_pgsz 的整数倍，则添加一倍，如果不是，先去除余数，在添加两倍
        // although gcc's Built-in Functions to Perform Arithmetic with
        // Overflow Checking is better, but it would require gcc >= 5.0
        long pgsz = sysconf(_SC_PAGESIZE);  // Size of a page in bytes.  Must not be less than 1.
        // pgsz must be > 0 && a power of two
		    // 判断 pgsz 是不是 2 的倍数
        assert(pgsz > 0 && (((pgsz - 1) & pgsz) == 0));
        u_pgsz = (size_t)((unsigned long)pgsz);
        // it should be always true in real life
		    // long 类型的话不会被截断，所以说永远为真
        assert(u_pgsz == (unsigned long)pgsz && ((u_pgsz << 1) >> 1) == u_pgsz);
        // sz 是自己分配想要的，u_pgsz 是系统合理的分配
        if(sz <= u_pgsz){
			      // 如果比系统的 PAGESIZE 还小的话，设置为系统 PAGESIZE 的两倍。
            sz = u_pgsz << 1;
        } else {
            // 按照文档的介绍，After some computation of alignment and reserve, this function will ensure the final valid length of the share stack in return:
            size_t new_sz;
            if((sz & (u_pgsz - 1)) != 0){  // 这个判断是看 sz 是不是 u_pgsz 的整数倍，不是就走 if 里面的逻辑，手动向下对齐
                new_sz = (sz & (~(u_pgsz - 1))); // 手动对齐到倍数未，比如 `u_pgsz = 4096(2 ** 12)` 也就是 `0b0001 0000 0000 0000`, (~(u_pgsz - 1)) 就是 `0b1110 0000 0000 0000`，对于 12 位的全部清 0，保证了 `new_sz` 是 `u_pgsz` 的整数倍。
                assert(new_sz >= u_pgsz);
                aco_size_t_safe_add_assert(new_sz, (u_pgsz << 1)); // 这个也是成立的，即使 sysconf 返回 -1 ，也通过强制转换成正数了。
                new_sz = new_sz + (u_pgsz << 1); // 给 new_sz 添加 `u_pgsz` 的两倍，new_sz 在前面已经通过了转换，现在肯定是 `u_pgsz` 的倍数。
                assert(sz / u_pgsz + 2 == new_sz / u_pgsz);
            } else {
                aco_size_t_safe_add_assert(sz, u_pgsz);
                new_sz = sz + u_pgsz; // 添加一倍的 `u_pgsz`
                assert(sz / u_pgsz + 1 == new_sz / u_pgsz);
            }
            // 在启用 guard page 的情况下，sz 被赋值到重新计算的 new_sz，用于下面的 mmap 设置 guard page
            sz = new_sz;
            assert((sz / u_pgsz > 1) && ((sz & (u_pgsz - 1)) == 0));
        }
    }
    // malloc 分配的变量是在堆区
    aco_share_stack_t* p = (aco_share_stack_t*)malloc(sizeof(aco_share_stack_t));
    assertalloc_ptr(p); // 判断不是空
    memset(p, 0, sizeof(aco_share_stack_t));
    // 变量命名有点鬼，读起来麻烦，`p` 表示的是一个 share stack 
    if(guard_page_enabled != 0){ // 启用了保护页机制
        p->real_ptr = mmap(
            NULL, sz, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0
        );
        assertalloc_bool(p->real_ptr != MAP_FAILED);
        p->guard_page_enabled = 1;
        /**
          The legal values for prot are the same as those for mmap (Figure 14.25). Be aware that
          implementations may require the address argument to be an integral multiple of the
          system’s page size.
        */
        // 按照 mproect 函数，add 需要为 virtual system page 的倍数，还要在 mmap 的时候 sz 就是了
        assert(0 == mprotect(p->real_ptr, u_pgsz, PROT_READ));
        // mprotest 用了 `u_pgsz` 作为保护页了，所以可以使用的地址要在 `real_ptr` 上加一个 `u_pgsz`
        p->ptr = (void*)(((uintptr_t)p->real_ptr) + u_pgsz);
        p->real_sz = sz; //
        assert(sz >= (u_pgsz << 1));
        p->sz = sz - u_pgsz;
    } else {
        //p->guard_page_enabled = 0;
        p->sz = sz;
        p->ptr = malloc(sz);
        assertalloc_ptr(p->ptr);
    }

    p->owner = NULL; // share_stack 是在 resume 的时候才指定 `owner`
#ifdef ACO_USE_VALGRIND
    p->valgrind_stk_id = VALGRIND_STACK_REGISTER(
        p->ptr, (void*)((uintptr_t)p->ptr + p->sz)
    );
#endif
    // uintptr_t 数据类型是专用用来操作 指针地址的
    uintptr_t u_p = (uintptr_t)(p->sz - (sizeof(void*) << 1) + (uintptr_t)p->ptr); // 这是是用了两个指针的大小从当前的 `stack` 顶部减去两个指针的大小
    u_p = (u_p >> 4) << 4; // 这个是为了 16 字节对齐, x86_64 abi 要求 rsp 入口需要是 16 位对齐的
    p->align_highptr = (void*)u_p; // 这个 share_stack 最高位置
    p->align_retptr  = (void*)(u_p - sizeof(void*)); // 留出来一个指针的大小，用来存 return 保护函数。
    *((void**)(p->align_retptr)) = (void*)(aco_funcp_protector_asm); // 指向在 `no-main coroutine` 中调用 `return` 执行的保护函数
    assert(p->sz > (16 + (sizeof(void*) << 1) + sizeof(void*)));
    p->align_limit = p->sz - 16 - (sizeof(void*) << 1); // 符合上面的操作，也是表示 `share stack` 最终能用的大小
    return p;
}

```

## aco_resume
这个是 `main coroutine` 用来启动或恢复 `no-main coroutine` 使用，也只有他能用，`no-main coroutine` 无法直接控制其他 `no-main coroutine`

```c
aco_attr_no_asan
void aco_resume(aco_t* resume_co){
	// 判断需要让出的 coroutine 是不是合法的 `no-main coroutine` `aco_resume` 只能是 `main coroutine` 调用
    assert(resume_co != NULL && resume_co->main_co != NULL
        && resume_co->is_end == 0
    );
    if(resume_co->share_stack->owner != resume_co){ // 当前 coroutine 的 share_stack 是和别人共享的时候
        if(resume_co->share_stack->owner != NULL){ // 已经被共享的 coroutine 使用过了
            aco_t* owner_co = resume_co->share_stack->owner; // 前面一个和 `resume_co` 共享 `share_stack` 的 coroutine
            assert(owner_co->share_stack == resume_co->share_stack); // 判断是不是共享的
            // 这个是应该一直等于，为啥要大于，TODO
            assert(
                (
                    (uintptr_t)(owner_co->share_stack->align_retptr)
                    >=
                    (uintptr_t)(owner_co->reg[ACO_REG_IDX_SP])
                )
                &&
                (
                    (uintptr_t)(owner_co->share_stack->align_highptr)
                    -
                    (uintptr_t)(owner_co->share_stack->align_limit)
                    <=
                    (uintptr_t)(owner_co->reg[ACO_REG_IDX_SP])
                )
            );
            // 保存前面那个 coroutine 的状态到他自己的 save_stack
            /**
            在 aco_create 的函数里面有如下的代码
              p->reg[ACO_REG_IDX_SP] = p->share_stack->align_retptr;
              如果只是看 C 语言的部分代码的话，那么 owner_co->save_stack.valid_sz 永远是 0，但是实际上
              还要结合 汇编部分的代码来看，acosw 中 `mov     QWORD PTR [rdi+0x28], rcx  // rsp` 会把当前 `stack` 存储到 p->reg[ACO_REG_IDX_SP]， ACO_REG_IDX_SP 在 x86_64 中是 5, 5 * sizeof(void *) = 40，也就是 0x28，
              在 x86_64，coroutine 结构体的 aco_s 结构体，开始就是 reg 定义，rdi 是函数的第一个参数。
              #ifdef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
                  void*  reg[8];
              #else
                  void*  reg[9];
              #endif
              从一个 `no-main coroutine` 切换到另外一个 `no-main coroutine` 的时候，一定要先切换到 `main coroutine`，
              那么就一定会调用  acosw(no_main_co, main_co); 那么这个时候 p->reg[ACO_REG_IDX_SP] 就指向 [rsp]，也就是 rsp
              指针中存储的内容，那么共享 share_stack 的情况下，owner_co->save_stack.valid_sz 的长度就是这个 `no-main coroutine` 在这个 share_stack 所用的内容，用来保存
            */
            owner_co->save_stack.valid_sz =
                (uintptr_t)(owner_co->share_stack->align_retptr)
                -
                (uintptr_t)(owner_co->reg[ACO_REG_IDX_SP]);
            // 当 save_stack.sz 也就是 `save_stack` 可以容纳的大小超过了实际使用的情况下，默认是 （64 bytes）
            if(owner_co->save_stack.sz < owner_co->save_stack.valid_sz){
                free(owner_co->save_stack.ptr); // 太小了，先释放，重新分配
                owner_co->save_stack.ptr = NULL;
                while(1){
                    owner_co->save_stack.sz = owner_co->save_stack.sz << 1;
                    assert(owner_co->save_stack.sz > 0);
                    if(owner_co->save_stack.sz >= owner_co->save_stack.valid_sz){
                        break;
                    }
                }
                // 重新分配
                owner_co->save_stack.ptr = malloc(owner_co->save_stack.sz);
                assertalloc_ptr(owner_co->save_stack.ptr);
            }
            // TODO: optimize the performance penalty of memcpy function call
            //   for very short memory span
            if(owner_co->save_stack.valid_sz > 0) {
                // 这个宏可以不用过多关注，在一定程度上实现了 memcpy 的优化版本，当作 memcpy 看就好了。
                aco_amd64_optimized_memcpy_drop_in(
                    owner_co->save_stack.ptr,
                    owner_co->reg[ACO_REG_IDX_SP],
                    owner_co->save_stack.valid_sz
                );
                // 每次保存到 save_stack 就记录一次
                owner_co->save_stack.ct_save++;
            }
            // 动态伸缩 max_cpsz 
            if(owner_co->save_stack.valid_sz > owner_co->save_stack.max_cpsz){
                owner_co->save_stack.max_cpsz = owner_co->save_stack.valid_sz;
            }
            // 切换之前把这个共享的 share_stack->owner 清空。
            owner_co->share_stack->owner = NULL;
            owner_co->share_stack->align_validsz = 0;
        }
        // 这个回归到需要切换到的 coroutine 的逻辑了
        assert(resume_co->share_stack->owner == NULL);
        assert(
            resume_co->save_stack.valid_sz
            <=
            resume_co->share_stack->align_limit - sizeof(void*)
        );
        // TODO: optimize the performance penalty of memcpy function call
        //   for very short memory span
        // 如果这个 coroutine 原来就有保存的状态，那么就把保存的状态 copy 到 share_stack  
        if(resume_co->save_stack.valid_sz > 0) {
            aco_amd64_optimized_memcpy_drop_in(
                (void*)(
                    (uintptr_t)(resume_co->share_stack->align_retptr)
                    -
                    resume_co->save_stack.valid_sz
                ),
                resume_co->save_stack.ptr,
                resume_co->save_stack.valid_sz
            );
            resume_co->save_stack.ct_restore++;
        }
        // 动态伸缩 max_cpsz 
        if(resume_co->save_stack.valid_sz > resume_co->save_stack.max_cpsz){
            resume_co->save_stack.max_cpsz = resume_co->save_stack.valid_sz;
        }
        resume_co->share_stack->align_validsz = resume_co->save_stack.valid_sz + sizeof(void*);
        // 改变这个 share_stack 的拥有者
        resume_co->share_stack->owner = resume_co;
    }
    // aco_gtls_co 指向的是当前线程运行的 coroutine 实例
    aco_gtls_co = resume_co;
    acosw(resume_co->main_co, resume_co); // 这个在 汇编 acosw 切换到了 resume_co， 要运行完之后，这个函数结束，然后将
    aco_gtls_co = resume_co->main_co;
}
```


## aco_yield
这个是 `no-main coroutine` 用来让出程序控制权的，比较易读，通过宏定义来实现的，主要就是调用汇编函数，`aco_gtls_co` 是定义的一个线程定义独有的变量，刚开始的时候指向 NULL（**没有任何的 `no-main coroutine` 运行**），如果有 `no-main coroutine` 在执行，那么指向这个 `no-main coroutine`，运行结束之后指向 `main coroutine`

```c
#define aco_yield1(yield_co) do {             \
    aco_assertptr((yield_co));                    \
    aco_assertptr((yield_co)->main_co);           \
    acosw((yield_co), (yield_co)->main_co);   \
} while(0)

#define aco_yield() do {        \
    aco_yield1(aco_gtls_co);    \
} while(0)
```

## 其他比较简单的 C 函数
### aco_get_co
这个就比较简单了，获取当前运行的 coroutine
```c
#define aco_get_co() ({(void)0; aco_gtls_co;})
```

### aco_get_arg
```c
#define aco_get_arg() (aco_gtls_co->arg)
```

### aco_exit
`no-main coroutine` 的退出不使用 `return`，而是使用 `aco_exit`；是将当前运行的 coroutine 设置标识位置 `is_end` 为1，然后 `aco_yield` 让出当前控制权。
```c
#define aco_exit1(co) do {     \
    (co)->is_end = 1;           \
    aco_assert((co)->share_stack->owner == (co)); \
    (co)->share_stack->owner = NULL; \
    (co)->share_stack->align_validsz = 0; \
    aco_yield1((co));            \
    aco_assert(0);                  \
} while(0)

// coroutine 的退出调用 `aco_exit`，调用 return 会触发保护机制
#define aco_exit() do {       \
    aco_exit1(aco_gtls_co); \
} while(0)
```

### aco_destroy
退出 coroutine 之后就是要销毁这个 coroutine
```c
void aco_destroy(aco_t* co){
    assertptr(co);
    if(aco_is_main_co(co)){
        free(co);
    } else {
        if(co->share_stack->owner == co){
            co->share_stack->owner = NULL;
            co->share_stack->align_validsz = 0;
        }
        // malloc 的都要手动 free 养成好习惯
        free(co->save_stack.ptr);
        co->save_stack.ptr = NULL;
        free(co);
    }
}
```
### aco_share_stack_destroy
stack 的也是动态创建，那么必定伴随的销毁
```c
void aco_share_stack_destroy(aco_share_stack_t* sstk){
    assert(sstk != NULL && sstk->ptr != NULL);
#ifdef ACO_USE_VALGRIND
    VALGRIND_STACK_DEREGISTER(sstk->valgrind_stk_id);
#endif
    if(sstk->guard_page_enabled){
        assert(0 == munmap(sstk->real_ptr, sstk->real_sz));
        sstk->real_ptr = NULL;
        sstk->ptr = NULL;
    } else {
        free(sstk->ptr);
        sstk->ptr = NULL;
    }
    free(sstk);
}
```

## x86_64 64位架构汇编函数
libaco 的核心就是汇编函数的处理
[这里介绍了 x86_64 64的常用寄存器](https://web.stanford.edu/class/archive/cs/cs107/cs107.1206/guide/x86-64.html)

为了更好的理解这里汇编函数，建立还是要补习一下 `x86_64` 的汇编的基本知识，比如 [assembly-language64-ubuntu](https://www.cl72.org/120introAssem/assembly-language64-ubuntu.pdf)，写的很浅显易懂，还要了解一下 `rsp` 寄存器和 `rbp` 寄存器

### rsp and rbp 寄存器
rsp 寄存器永远都是用来指向当前程序 stack 的顶部，rbp 寄存器一般用来指向当前 stack frame 的开始位置，但是 gcc 编译的过程可以禁止使用 rbp 寄存器作为 `stack frame` 的基址指针，添加编译参数 `-fomit-frame-pointer`。

stack_frame.c 一个最简单的 c 程序。
```c
void func() {
    char buffer[64];
}

int main() {
    func();
    return 0;
}
```
使用 
`gcc -masm=intel -S stack_frame.c -o stack_frame.S`
编译出来的，可以看到是使用 `rbp` 寄存器，然后 `rbp` 寄存器指向 `rsp` 寄存器，也就是 `rbp` 自己。
```c
func:
.LFB0:
	.cfi_startproc
	endbr64
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	mov	rbp, rsp
	.cfi_def_cfa_register 6
	sub	rsp, 80
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR -8[rbp], rax
	xor	eax, eax
	nop
	mov	rax, QWORD PTR -8[rbp]
	sub	rax, QWORD PTR fs:40
	je	.L2
	call	__stack_chk_fail@PLT
.L2:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```
这个命令，添加了禁止 `stack frame pointer`
`gcc -fomit-frame-pointer -masm=intel -S stack_frame.c -o stack_frame.S`
编译出来，可以明显的看到，他是手动操作 `rsp` 寄存器添加或者减少指定的 `offset`。
```c
func:
.LFB0:
	.cfi_startproc
	endbr64
	sub	rsp, 88
	.cfi_def_cfa_offset 96
	mov	rax, QWORD PTR fs:40
	mov	QWORD PTR 72[rsp], rax
	xor	eax, eax
	nop
	mov	rax, QWORD PTR 72[rsp]
	sub	rax, QWORD PTR fs:40
	je	.L2
	call	__stack_chk_fail@PLT
.L2:
	add	rsp, 88
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE0:
	.size	func, .-func
	.globl	main
	.type	main, @function
```

### Stack frame
根据 `X86_64 abi` 中所述

> In addition to registers, each function has a frame on the run-time stack. 

大部分的函数的调用都会形成 `stack frame`，也就是 `栈桢`，如下的内容文字描述部分很多都是从 [assembly-language64-ubuntu](https://www.cl72.org/120introAssem/assembly-language64-ubuntu.pdf) 中的 12.8.3 节中借鉴来的。

The possible items in the call frame include: （下面这个是无顺序的）
1. Return address (required).
2. Preserved registers (if any).
3. Passed arguments (if any).
4. Stack dynamic local variables (if any).

满足下面所有条件的函数调用不会形成 `stack frame`
1. [`leaf function`](https://gcc.gnu.org/onlinedocs/gccint/Leaf-Functions.html)
2. 只是通过寄存器传递变量；在 `x86_64` 系统中，有六个寄存器可以用来传递参数，`rdi, rsi, rdx, rcx, r8, r9` 分别是从第一个到第六个参数，如果超过了 6 个参数，那么就是 **以参数传递的相反方向将参数压栈**。
3. callee 没有修改 caller 任何需要保存的寄存器；正常的调用，有 `r12-r12, rbx, rbp, rsp`（在 x86_64 abi 中还有 x87 cw register，以及 mxcsr register）
4. 没有创建 `stack` 上的局部变量


#### caller and callee
caller 和 callee 这两个概念比较好理解，比如在 foo 函数里面调用 bar 函数，就是 foo 函数就是 caller， bar 函数就是 callee。

### 核心汇编函数
主要是实现了三个汇编函数，主要的函数还是 `acosw`，用于 coroutine 的切换，对于没有学过汇编的我而言，汇编代码的意思配合 AI 还是能看懂。但是配合 C 语言里面的定义，以及 `rsp` 寄存器和 `rip` 寄存器的作用和设置，以及程序运行函数的压栈，这一下就变得复杂起来了

他所有的核心就是通过 `rsp` 来欺骗 cpu，对于不同的 `coroutine` 更换不同的 `stack`。每次在汇编代码里面切换的时候，将 `rsp` 指向 `coroutine` 的 `share stack`，用于函数执行过程的 `stack` 压栈。对于 `main coroutine` 的 `share stack` 是 NULL，因为在 resume 的时候如果是 `main coroutine`，那么就是不会处理 `share_stack` 的逻辑。在第一次将 `main coroutine` 切换到 `no-main coroutine` 的时候会将需要保存的 register（`rsp` 是最重要的） 保存到 `reg` 数组里面，在切换回来的时候，再将 `reg` 数字 `mov` 到寄存器里面。对于 `no-main coroutine` 也是同样的处理，对于他而言在 `aco_share_stack_new2` 中，有如下代码，将 `reg` 数组中的 `reg[4]` 设置为 coroutine 的调用函数，`reg[5]` 设置为调用 return 的保护函数
```c
#define ACO_REG_IDX_RETADDR 4
#define ACO_REG_IDX_SP 5
p->reg[ACO_REG_IDX_RETADDR] = (void*)fp;
// rsp register
p->reg[ACO_REG_IDX_SP] = p->share_stack->align_retptr;
```

这段代码就是配合上面代码使用的，将 `rsp` 寄存器设置为 `p->reg[ACO_REG_IDX_SP]` 对于 `no-main coroutine` 就是 `p->share_stack->align_retptr`，对于 `main coroutine` 就是 `NULL`，不过 `main coroutine` 就是在主线程之上，他返回，那么程序也就是结束了，不过该有的还是有，这里就需要主要的是 `jmp` 和 `call` 指令的区别了 TODO，**在 jmp 之前，将 `return` 保护函数推送到 `rsp`，这样，如果 coroutine 执行调用 return ，就会来执行这个函数。**
```c
mov     rsp, rcx
jmp     rax
```


#### acosw

首先我们需要了解，为什么要保存是如下的寄存器

> This subsection discusses usage of each register. Registers %rbp, %rbx and %r12 through %r15 “belong” to the calling function and the called function is required to preserve their values. In other words, a called function must preserve these registers’ values for its caller. Remaining registers “belong” to the called function.5 If a calling function wants to preserve such a register value across a function call, it must save the value in its local stack frame. 

> The control bits of the MXCSR register are callee-saved (preserved across calls), while the status bits are caller-saved (not preserved).


1. `rdi` callee 的第一个参数，acosw 只接收两个参数，**用来接收 from_co，当前的 coroutine**。
2. `rsi` callee 的第二个参数，acosw 只接收两个参数，**用来接收 to_co，将要运行的 coroutine**。
3. `rdx` callee 的第三个参数，acosw 只接收两个参数，用来自己保存参数了。
4. `rcx` callee 的第四个参数，acosw 只接收两个参数，用来自己保存参数了。
5. 由上面的文档可以知道 `r10-r15` 属于 caller
6. `rip` 指令指针
7. `rsp` **堆栈指针，由 caller 拥有，这个是切换的核心处理 register。**
8. `rbx` 	Local variable, caller 拥有
9. `rbp` Local variable, caller 拥有

```c
.globl acosw
.type  acosw, @function
.intel_syntax noprefix
/*
    0x00                  -->                  0xff
    r12 r13 r14 r15 rip rsp rbx rbp fpucw16 mxcsr32
    0   8   10  18  20  28  30  38  40      44
*/
acosw:
    // rdi: from_co
    // rsi: to_co

    mov     rdx, QWORD PTR [rsp]      // 获取返回地址
    lea     rcx, [rsp+8]              // 获取当前栈顶指针

    // 保存 from_co 的寄存器状态
    mov     QWORD PTR [rdi+0x00], r12
    mov     QWORD PTR [rdi+0x08], r13
    mov     QWORD PTR [rdi+0x10], r14
    mov     QWORD PTR [rdi+0x18], r15
    mov     QWORD PTR [rdi+0x20], rdx  // return address
    mov     QWORD PTR [rdi+0x28], rcx  // rsp
    mov     QWORD PTR [rdi+0x30], rbx
    mov     QWORD PTR [rdi+0x38], rbp

    fnstcw  WORD  PTR [rdi+0x40]       // FPU 控制字
    stmxcsr DWORD PTR [rdi+0x44]       // SSE 控制字

    // 恢复 to_co 的寄存器状态
    mov     r12, QWORD PTR [rsi+0x00]
    mov     r13, QWORD PTR [rsi+0x08]
    mov     r14, QWORD PTR [rsi+0x10]
    mov     r15, QWORD PTR [rsi+0x18]
    mov     rax, QWORD PTR [rsi+0x20]  // return address
    mov     rcx, QWORD PTR [rsi+0x28]  // rsp
    mov     rbx, QWORD PTR [rsi+0x30]
    mov     rbp, QWORD PTR [rsi+0x38]

    fldcw   WORD  PTR [rsi+0x40]
    ldmxcsr DWORD PTR [rsi+0x44]

    mov     rsp, rcx
    jmp     rax
```
## 实践
如上就是关于 libaco 的原理和实现过程，但是每一次从 `no-main couroutine` `aco_yield` 到 `main coroutine`，如果这个 `no-main coroutine` 还有代码要执行呢，为何没有像 `python twisted` 那样自动切换到需要继续执行的 `coroutine`，这个就是要自己结合业务代码实际的去实现 `schedule` 调度了。

对于我而言，我学习这个是为了更好的了解 socket 服务器的编写，根据网上的信息和对 coroutine 的理解，使用 coroutine 来编写 socket 服务器并不会提升多少性能，比较同一个时间段还是只能一个 coroutine 在运行，但是他能以异步的模式来编写同步的代码，这样写起来就不会那么的麻烦，能提高程序的可维护性和可读性。
