---
title: "Linux uid euid gid egid"
date: 2025-01-10 21:19:00 +0800
categories: [Linux, APUE]
tags: [Linux, APUE]
---
# Preface
这个博客用来记录学习 Linux 对于文件进程的 UID（真实用户ID），EUID（有效用户ID），GID（组ID），EGID（有效组ID），以及（set-user-id、set-group-user-id）的学习记录，花费了我不少时间去理解。
## 用户 ID 和设置 ID(各种 ID 的作用)
对于一个进程来说，一般来说有一下关联的进程
<table>
  <thead>
    <tr>
      <td>名称</td>
      <td>作用</td>
    </tr>
  </thead>
  <tbody>
  <tr>
    <td>实际用户 ID</td>
    <td rowspan="2">实际上是谁。这个一般在我们登陆的时候就由系统指定了，一般来说是不会改变（可以由 root 用户通过 setuid 函数更改）</td>
  </tr>
  <tr>
    <td>实际组 ID</td>
  </tr>
  <tr>
    <td>有效用户 ID</td>
    <td rowspan="3">用于文件权限的访问检查（在绝大多数情况下， euid=ruid，egid=rgid）</td>
  </tr>
  <tr>
    <td>有效组 ID</td>
  </tr>
  <tr>
    <td>附属组 ID </td>
  </tr>
  <tr>
    <td>已保存的设置用户 ID</td>
    <td rowspan="2">exec 程序读取（有关进程的启动部分了）</td>
  </tr>
  <tr>
    <td>已保存的设置组 ID</td>
  </tr>
  </tbody>
</table>

### UID and GID
正如表格中所说的一样，当我们登陆的时候，我们的 uid(ruid), gid(rgid) 一般就被内核设置好了
   ```shell
   echo $UID, $GID;
   ```
   ```c
    #include <stdio.h>
    #include <unistd.h>

    int main(int argc, char const *argv[])
      {
      printf("real user id is %d, real group id is %d\n", getuid(), getgid());
      // real user id is 501, real group id is 20
      return 0;
    }
   ```
### EUID and EGID
有效 ID 和有效组 ID：用户检查用户是否有权限访问某个文件或者目录
#### 权限
首先简单了解一下文件权限主要有三种 **r(4),w(2),x(1)** 三种权限，`r` 表示的是读，`w` 表示的是写， `x` 表示的是执行权限
```shell
touch a.txt
ls -l a.txt
// result: -rw-r--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
```
在上面，用 `touch` 命令创建了一个空文件，`ls -l` 列出文件信息，信息以 `-rw-r--r--` 开始，显示了如何通过权限判断能否访问这个文件，第一个字符是 `-` 表示的是普通文件（`d` 表示的是目录，还是很多其他的）。 `rwx` 权限是三个一组：
1. 第一组表示的文件的所有者(u)对于文件的权限，`rw-`，表示有 **读和写** 权限。
2. 第二组表示的 **文件所有者同一个组的(g)** 用户对于该文件的文件权限，`r--` 表示同组成员只有 **读** 的权限。
3. 第三组表示的其他用户(o)（不是文件所有者，也不是同组成员）对于文件的访问权限，`r--` 表示其他成员只有 **读** 的权限。
shell 中有一个常用的命令 `chmod` 来更改文件的权限，(u 表示拥有者，g 表示同组，o 表示其他)
```shell
ls -l a.txt # -rw-r--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
chmod u+x a.txt
ls -l a.txt # -rwxr--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
chmod g+w a.txt
ls -l a.txt # -rwxrw-r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
# 上面两个命令和合并为 chmod u+x,g+w a.txt
```
当然，可以直接使用数字表示的每组的权限，如上面所写的一样，r(4), w(2), x(1), 直接使用这种方式把 `a.txt` 的权限弄到初始化时候权限。
```shell
ls -l a.txt # -rwxrw-r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
chmod 644 a.txt
ls -l a.txt # -rw-r--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
```

#### 文件，目录
对于文件的读，写以及执行权限都很好理解，不再多言。
对于目录的三种权限位
1. 读：表示能够获取目录下所有文件名字的列表。
2. 写：创建、删除文件或者目录
3. 执行：表示在进入目录，搜索特定的文件。
4. 
```shell
mkdir btest
ls -l
# result
# total 0
# -rwsr--r--  1 vzgoll  staff   0 Jan 11 12:14 a.txt
# drwxr-xr-x  2 vzgoll  staff  64 Jan 11 12:41 btest
```
现在有一个 `btest` 的测试目录，作为所有者，对于目录有全部的权限
```shell
cd btest
touch b.txt
# 
chmod u-w btest
touch c.txt # touch: c.txt: Permission denied
rm b.txt # rm: b.txt: Permission denied
mkdir dd # mkdir: dd: Permission denied
```


刚开始的时候，能进入目录创建文件，去掉 `w` 写权限之后，就无法创建和删除文件（目录）
```shell
cd ..
chmod u+w btest
ls -l btest
# total 0
# -rw-r--r--  1 vzgoll  staff  0 Jan 11 12:46 b.txt
chmod u-r btest
ls -l btest 
# total 0
# ls: btest: Permission denied
```
在我们去掉 `r` 读权限之后，无法读取目录了，提示我们没有权限读取这个目录



此时，我们失去了 `r` 权限的拥有，对文件名 complete 补全也没了（本人 zsh），但是不影响我们读取文件内容
```shell
cat btest/b.txt # 正常
chmod u-x btest
cat btest/b.txt # cat: btest/b.txt: Permission denied
```
你可以看到在，失去对目录 `x` 执行权限之后，就无法访问目录下的文件了。

C code，O_SEARCH 表示的就是搜索，更加的直观，在去掉 `x` 权限之后，**open directory btest error: Permission denied**
** (`O_SEARCH` is equal to `O_EXEC | O_DIRECTORY`)
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char const *argv[])
{
	int fd;
	if((fd = open("./btest", O_SEARCH)) < 0){
		perror("open directory btest error");
		exit(EXIT_FAILURE);
	}
	printf("open directory btest success fd is: %d\n", fd);
	close(fd);
	return 0;
}
```

至次，我理清了对于文件的三种权限的理解。
#### tips
1. 删除文件，需要的是我们对于文件所在目录的 `wx` 写和执行权限，对于文件本身的权限无关。
```shell
sudo su
mdkir ctest
exit # 退出超级用户
# 
rmdir ctest # ok 这个时候 ctest 的目录实在 test 下，属于我
# 
sudo su
mkdir ctest
cd ctest
mkdir ctestInner
exit
# 
cd ctest
rmdir ctestInner # rmdir: ctestInner: Permission denied 
# 因为这个时候 cTestInner 目录是 cTest，所有者是 root
```
2. 对于目录有 `wx` 才能在目录里面创建文件。
3. 访问某个文件，对于文件的所有前缀目录都要有 `x` 权限，如上所说的搜索权限

### save-set-user-id 和 save-set-group-id
在 `save-set-user-id` 和 `save-set-group-id` 之前，还有 `set-user-id` 和 `set-group-id` 的理解。

当一个进程执行的时候，通常情况下，`euid=ruid` and `egid=rgid`，但是有些程序是 root 所拥有的情况下，又需要给普通用户使用，比如 `passwd` 和 `at` 命令，用户要能修改自己的密码，又不能修改别人的密码，这样的情况下，就需要使用 `set-user-id`，

在程序启动的时候，当 `set-user-id` 位被设置了的时候，进程的 `euid` 为文件的所有者的 **uid**，对于 `passwd` 而言，正常用户启动的时候，这个程序进程也有了 root 权限。
`set-group-id` 也是类似的。

当 `set-user-id` 被设置的时候， `save-set-user-id` ，`euid` 需要被替换为文件所有者的 `uid`, 那么 `save-set-user-id` 保存的就是原来的 `euid`. `save-set-group-id` 是同一个道理。

#### privilege
```shell
ls -l $(which passwd) # -rwsr-xr-x. 1 root root 27832 Jun 10  2014 /usr/bin/passwd (在一个 Linux 服务器上，mac 上并没有设置 s，我也不清楚为啥)
```
可以看到，`set-user-id` 占用了原来 `x` 执行位，这就多了一个设置（不过 `set-user-id` 本来也是用来配合 `x` 使用。

在设置 `set-user-id` 的时候，如果
1. 当 `x` 被设置的时候，显示为 `s` 小写
2. 当 `x` 没被设置的时候，显示为 `S` 大写(不过也想不通，不执行设置 `set-user-id` 的意义是啥)

```shell
ls -l a.txt # -rwsr--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
chmod u-x a.txt
ls -l a.txt # -rwSr--r--  1 vzgoll  staff  0 Jan 11 12:14 a.txt
```

### 附属组的概念
正如开发的时候时候，一个开发者有自己主要开发功能的时候（主要的组），还有其他设置协助别人开发，加入别人的组

```shell
groups
```
