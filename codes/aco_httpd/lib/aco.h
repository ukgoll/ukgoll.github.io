// Copyright 2018 Sen Han <00hnes@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ACO_H
#define ACO_H

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

#ifdef ACO_USE_VALGRIND
    #include <valgrind/valgrind.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ACO_VERSION_MAJOR 1
#define ACO_VERSION_MINOR 2
#define ACO_VERSION_PATCH 4

#ifdef __i386__
    #define ACO_REG_IDX_RETADDR 0
    #define ACO_REG_IDX_SP 1
    #define ACO_REG_IDX_BP 2
    #define ACO_REG_IDX_FPU 6
#elif __x86_64__
    #define ACO_REG_IDX_RETADDR 4
    #define ACO_REG_IDX_SP 5
    #define ACO_REG_IDX_BP 7
    #define ACO_REG_IDX_FPU 8
#elif __aarch64__
    #define ACO_REG_IDX_RETADDR 13
    #define ACO_REG_IDX_SP 14
    #define ACO_REG_IDX_BP 12
    #define ACO_REG_IDX_FPU 15
#else
    #error "platform no support yet"
#endif
/**
 * ACO_REG_IDX_RETADDR 手动定义保存 x30 (返回地址 LR)
 * ACO_REG_IDX_SP 手动定义保存 sp (栈指针)
 * ACO_REG_IDX_BP 手动定义保存 x29 (帧指针 FP)
 */

typedef struct {
    void*  ptr;
    size_t sz;
    size_t valid_sz;
    // max copy size in bytes
    size_t max_cpsz;
    // copy from share stack to this save stack
    size_t ct_save;
    // copy from this save stack to share stack 
    size_t ct_restore;
} aco_save_stack_t;

struct aco_s;
typedef struct aco_s aco_t;

typedef struct {
    void*  ptr;            
    size_t sz;
    void*  align_highptr;
    void*  align_retptr;
    size_t align_validsz;
    size_t align_limit;
    aco_t* owner;

    char guard_page_enabled;
    void* real_ptr;
    size_t real_sz;

#ifdef ACO_USE_VALGRIND
    unsigned long valgrind_stk_id;
#endif
} aco_share_stack_t;

typedef void (*aco_cofuncp_t)(void);

struct aco_s{
    // cpu registers' state
#ifdef __i386__
    #ifdef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
        void*  reg[6];
    #else
        void*  reg[8];
    #endif
#elif __x86_64__
    #ifdef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
        void*  reg[8];
    #else
        void*  reg[9];
    #endif
#elif __aarch64__
    #ifdef ACO_CONFIG_SHARE_FPU_MXCSR_ENV
        void*  reg[15];
    #else
        void*  reg[16];
    #endif
#else
    #error "platform no support yet"
#endif
    aco_t* main_co;
    void*  arg;
    char   is_end;

    aco_cofuncp_t fp;
    
    aco_save_stack_t  save_stack;
    aco_share_stack_t* share_stack;
};

#define aco_likely(x) (__builtin_expect(!!(x), 1))

#define aco_unlikely(x) (__builtin_expect(!!(x), 0))

#define aco_assert(EX)  ((aco_likely(EX))?((void)0):(abort()))

#define aco_assertptr(ptr)  ((aco_likely((ptr) != NULL))?((void)0):(abort()))

#define aco_assertalloc_bool(b)  do {  \
    if(aco_unlikely(!(b))){    \
        fprintf(stderr, "Aborting: failed to allocate memory: %s:%d:%s\n", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__);    \
        abort();    \
    }   \
} while(0)

#define aco_assertalloc_ptr(ptr)  do {  \
    if(aco_unlikely((ptr) == NULL)){    \
        fprintf(stderr, "Aborting: failed to allocate memory: %s:%d:%s\n", \
            __FILE__, __LINE__, __PRETTY_FUNCTION__);    \
        abort();    \
    }   \
} while(0)

#if defined(aco_attr_no_asan)
    #error "aco_attr_no_asan already defined"
#endif
#if defined(ACO_USE_ASAN)
    #if defined(__has_feature)
        #if __has_feature(__address_sanitizer__)
            #define aco_attr_no_asan \
                __attribute__((__no_sanitize_address__))
        #endif
    #endif
    #if defined(__SANITIZE_ADDRESS__) && !defined(aco_attr_no_asan)
        #define aco_attr_no_asan \
            __attribute__((__no_sanitize_address__))
    #endif
#endif
#ifndef aco_attr_no_asan
    #define aco_attr_no_asan
#endif

extern void aco_runtime_test(void);

// 在当前线程里面初始化 coroutine
extern void aco_thread_init(aco_cofuncp_t last_word_co_fp);

// 下面三个函数都会通过 __asm__ 调用汇编函数
extern void* acosw(aco_t* from_co, aco_t* to_co) __asm__("acosw"); // asm

extern void aco_save_fpucw_mxcsr(void* p) __asm__("aco_save_fpucw_mxcsr");  // asm

// guard 保护调用
extern void aco_funcp_protector_asm(void) __asm__("aco_funcp_protector_asm"); // asm

extern void aco_funcp_protector(void);

extern aco_share_stack_t* aco_share_stack_new(size_t sz);

aco_share_stack_t* aco_share_stack_new2(size_t sz, char guard_page_enabled);

extern void aco_share_stack_destroy(aco_share_stack_t* sstk);

extern aco_t* aco_create(
        aco_t* main_co,
        aco_share_stack_t* share_stack, 
        size_t save_stack_sz, 
        aco_cofuncp_t fp, void* arg
    );


// https://www.ibm.com/docs/da/i/7.3.0?topic=specifiers-thread-storage-class-specifier 每个线程都有一个这个变量，相互不影响
// aco's Global Thread Local Storage variable `co`, 用来存储当前的协程，`协程` 这个概念是用户自己实现的，在同一个时间点内还是只有一个程序在运行
// 用 aco_gtls_co 指向当前线程正在运行的 `协程` 如果除了 main coroutine 没有 coroutine 在运行，这个就是指向 NULL。
extern __thread aco_t* aco_gtls_co;

// 在 `main coroutine` 调用，开启一个新的 `协程`
aco_attr_no_asan
extern void aco_resume(aco_t* resume_co);


// 这里调用了 汇编代码 // TODO
//extern void aco_yield1(aco_t* yield_co);
#define aco_yield1(yield_co) do {             \
    aco_assertptr((yield_co));                    \
    aco_assertptr((yield_co)->main_co);           \
    acosw((yield_co), (yield_co)->main_co);   \
} while(0)

#define aco_yield() do {        \
    aco_yield1(aco_gtls_co);    \
} while(0)

#define aco_get_arg() (aco_gtls_co->arg)

#define aco_get_co() ({(void)0; aco_gtls_co;})

#define aco_co() ({(void)0; aco_gtls_co;})

// 销毁 coroutine
extern void aco_destroy(aco_t* co);


// 判断一个 coroutine 是不是 `main coroutine`，`main coroutine` 的 `main_co` 字段指向 NULL
// `非 main coroutine` 的 `main_co` 执行 `main_coroutine`
#define aco_is_main_co(co) ({((co)->main_co) == NULL;})

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

#ifdef __cplusplus
}
#endif

#endif
