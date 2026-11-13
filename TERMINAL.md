# 终端配置

## 配置与链接

仓库默认位置：`~/.mydotfiles/com.ml4w.dotfiles.stable`。以下文件与目录通过用户目录中的符号链接使用仓库版本：

| 用户路径 | 仓库路径 |
| --- | --- |
| `~/.zshrc` | `.zshrc` |
| `~/.p10k.zsh` | `.p10k.zsh` |
| `~/.zshrc_custom` | `.zshrc_custom` |
| `~/.config/zshrc` | `.config/zshrc` |
| `~/.config/kitty` | `.config/kitty` |

在新机器安装依赖并备份原有同名文件后，建立上述链接。例如（要求目标位置不存在）：

```sh
repo="$HOME/.mydotfiles/com.ml4w.dotfiles.stable"
ln -s "$repo/.zshrc" "$HOME/.zshrc"
ln -s "$repo/.p10k.zsh" "$HOME/.p10k.zsh"
ln -s "$repo/.zshrc_custom" "$HOME/.zshrc_custom"
mkdir -p "$HOME/.config"
ln -s "$repo/.config/zshrc" "$HOME/.config/zshrc"
ln -s "$repo/.config/kitty" "$HOME/.config/kitty"
```

ML4W 的其余桌面配置沿用原有安装方式。zsh 自定义入口为 `.config/zshrc/custom/20-customization`。

## 依赖

- Oh My Zsh：安装到 `~/.oh-my-zsh`。
- Powerlevel10k：`https://github.com/romkatv/powerlevel10k.git`，安装到 `${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/themes/powerlevel10k`。
- 第三方插件安装到 `${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/plugins`：
  - `zsh-autosuggestions`：`https://github.com/zsh-users/zsh-autosuggestions.git`
  - `zsh-syntax-highlighting`：`https://github.com/zsh-users/zsh-syntax-highlighting.git`
  - `you-should-use`：`https://github.com/MichaelAquilina/zsh-you-should-use.git`
  - `nice-exit-code`：`https://github.com/bric3/nice-exit-code.git`
- 程序依赖：`zsh`、`git`、`fzf`（支持 `--zsh`）、`zoxide`、`catimg`、`thefuck`、`pkgfile`、`notify-send`。解压插件所需程序取决于使用的压缩格式。
- Arch 的缺失命令提示需要有效的 pkgfile 数据库：`sudo pkgfile -u`。
- 字体族：`JetBrains Maple Mono`。从 `JetBrainsMapleMono-NF-XX-XX-XX.zip` 安装字体到 `~/.local/share/fonts`，执行 `fc-cache -f`。字体压缩包和二进制程序由本机单独安装。

## 提示符与通知

`.p10k.zsh` 在 Powerlevel10k 经典预设上覆盖配色，使用终端 ANSI 调色板以跟随 Kitty/Matugen。输入符号为 `❯`，正常时为壁纸主题主色，错误时为红色。请在此文件调整个性化配置。

Matugen 的 Kitty 模板与 Waybar 使用同一份壁纸调色板，生成 `~/.config/kitty/colors-matugen.conf` 后通过 SIGUSR1 通知已打开的 Kitty 重载。终端始终使用黑色背景、浅色正文；ANSI color4/12 使用 primary 的亮暗阶，color5/13 使用 tertiary 阶，color6/14 使用 secondary 阶。Powerlevel10k 的目录、Git 状态、上下文和耗时分别使用这些 ANSI 色阶。

Chrome 多色主题由 solar-theme 生成到 `~/.config/chrome-wallpaper-theme/manifest.json`。首次在 `chrome://extensions` 加载此文件夹并保持启用。用户级启动脚本 `~/.local/bin/google-chrome-stable` 每次启动前清理该目录的 `Cached Theme.pak`，使下一次完整启动重建主题。Super+B 和用户级应用菜单已接入；浏览器运行中不即时刷新，`chrome://restart` 和直接调用 `/usr/bin/google-chrome-stable` 会绕过脚本。不要安装 CRX 副本，否则无法跟随源目录同步。

bgnotify 阈值以 `.config/zshrc/custom/20-customization` 中的 `bgnotify_threshold` 为准；当前为 301 秒。

右侧状态使用 `nice-exit-code` 的标签生成中文说明，不显示成功/失败图标。最终退出码为 0 时隐藏整个状态组件；普通错误以黄色显示 `中文含义(退出码)`，例如 `命令不存在(127)`。信号退出显示 `已中断(130) · 中断信号 SIGINT(2)`：退出说明和退出码为黄色，分隔点为终端白色，信号中文含义、完整信号名和信号码为红色。没有信号时省略分隔点和信号部分。插件没有映射的退出码显示 `未定义的退出状态`。这些说明是通用映射，具体程序可能有自己的退出码定义；信号名沿用 p10k 根据退出码的推断。

重载 shell 配置使用 `exec zsh`；字体修改后重新打开 Kitty。
