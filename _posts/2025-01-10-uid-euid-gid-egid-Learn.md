---
title: "Linux uid euid gid egid"
date: 2025-01-10 21:19:00 +0800
categories: [Linux]
tags: [Linux]
---
# Preface
这个博客用来记录学习 Linux 对于文件进程的 UID（真实用户ID），EUID（有效用户ID），GID（组ID），EGID（有效组ID），以及（set-user-id、set-group-user-id）的学习记录，花费了我不少时间去理解。
# Docker Ubuntu image
这种关于多用户的功能学习，用 Linux 最好，使用 docker 拉一个简版的 `ubuntu` 的镜像是非常好用的
docker 安装比较简单。
## 直接开始拉 image
```shell
docker pull ubuntu;
```
1. 运行一个 Ubuntu container
```shell
docker container run --name idstatement -it ubuntu bash
```
2. 为了让他一直在运行，使用 `CTRL+P+Q` deattach container(**可以使用 `docker attach idstatement` 重新连接到这个容器**)
3. 由于登陆是默认的 `root` 登陆，超级权限，为了学习验证，创建一个普通用户 `nuser`
```shell
useradd nuser
```
4. 再开一个终端，使用普通用户登陆这个 container
```shell
docker exec -it -u nuser idstatement bash 
```
经过上面的过程，我们就有了一个普通用户 `nuser`，和 `root` 用户的
