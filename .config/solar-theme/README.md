# 独立日出日落主题

位置：安徽滁州天长城区近似坐标 32.684971, 119.009323；Asia/Shanghai。
数据来源与署名：Sunrise-Sunset.org，https://sunrise-sunset.org/。
每天联网取得当天日出日落并缓存；每分钟判断一次。无当天数据且联网失败时保持当前主题，15 分钟后重试。

## 范围
当前用户 UWSM 会话和 systemd 用户服务的 QT_QPA_PLATFORMTHEME=qt6ct。
同步 GTK 3/4 的深浅偏好及 colors.css、GSettings color-scheme、Qt6ct 配色。
Qt6：白天 airy.conf，夜间 darker.conf；保留原有 Breeze 样式及其他设置。
Qt5ct 未安装，当前不包含 Qt5 的独立主题集成。
不保证已经打开的每个应用会立即刷新；可重新打开仍使用旧配色的应用。
Waybar、终端、通知中心、登录界面等自身样式未纳入本次自动切换。

## 配置与管理
- 配置：~/.config/solar-theme/config.json
- 状态：python3 ~/.local/bin/solar-theme status
- 立即自动校准：systemctl --user start solar-theme.service
- 停用：systemctl --user disable --now solar-theme.timer
- 手动浅色/深色：python3 ~/.local/bin/solar-theme light 或 dark（运行中的定时器会自动纠正）
- 日志：journalctl --user -u solar-theme.service
- 原始 GTK/Qt 文件备份：~/.local/state/solar-theme/backup/

GTK 原有深色配色被保留；浅色配色通过独立 Matugen 配置从原强调色生成一次，存放于此目录。
旧 ~/.config/ml4w/listeners.sh 中仅注销 gtk-theme-switcher 注册项，以避免两套主题切换逻辑竞争。
要恢复旧监听器，可停用此定时器后取消该注册项的注释。

持久环境设置位于 ~/.config/uwsm/env 和 ~/.config/environment.d/90-solar-theme.conf。
本方案不使用 GTK_THEME 强制覆盖应用主题。
