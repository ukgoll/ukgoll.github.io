---
title: "Linux group 使用"
date: 2025-02-07 21:00:00 +0800
categories: [Linux]
tags: [Linux]
---
# Preface
最近在开发 App 的时候，测试热更新的时候，遇到一个问题，关于权限的问题。我琢磨了一下，可以使用 **Linux group** 这个概念来解决：

在 App 测试热更新的时候，需要启动更改 nginx 配置文件，安装配置 nginx 的时候，一般都是 root 用户作为管理员来操作的，作为公司新人的我，肯定是没有 root 权限的。问题出现了。
## 具体解决
我的解决思路：
1. 将 root 用户需要管理的这个服务单独分离，分到一个组，是热更服务，直接叫做 **hotupdate** 就好了。
2. 将这部分的服务目录，更改为用户组 **hotupdate** （原来基本会是 root）。
   ```shell
   chgrp hotupdate config_file
   ```
3. 将对应的配置文件,组类的权限添加 **w** 权限。
   ```shell
   chmod g+w config_file
   ```
4. 创建一个普通用户，将这个普通用户添加到 **hotupdate** 组
   ```shell
   useradd normaluser
   usermod -aG hotupdate normaluser
   ```
   或者
  ```shell
   useradd -G hotupdate normaluser
   ```

在我这个问题为前提下，需要重启 nginx
```shell
ls -l $(which systemctl)
# -rwxr-xr-x 1 root root 967512 Mar  2  2023 /usr/bin/systemctl
```
如上所示，systemctl 对于其他用户也能启动，所以 **normaluser** 在拥有编辑 nginx config file 的权限之后，也能重启 nginx。

## 本机测试
前提：在虚拟机里面安装 ubuntu live-server 版本，设置 NAT 网络模式，使用端口转发来访问 nginx，安装好 nginx。
我转发到了本地的 1128 端口

在 nginx 启动之后，有个默认的 nginx html 页面
![nginx-default](/assets/custome/nginx-defalut.png "nginx-default")


这个默认的配置位于 `/etc/nginx/sites-available` 目录下的 `default` 文件。
```shell
chgrp default hotupdate;
chmod g+w default;
ls -l
# total 4
# -rw-rw-r-- 1 root hotupdate 2454 Feb 11 07:41 default
```
在修改之后，修改 `root /var/www/html` 为自己的文件之后，对于我而言是 `/home/normaluser/nginx-html-test`，然后重启 nginx，`sudo systemctl restart nginx`。就可以看到新的 nginx 页面。
![nginx-modify](/assets/custome/nginx-modify.png "nginx-modify")

好，思路可行，验证通过。
