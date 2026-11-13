# Powerlevel10k classic preset. Run `p10k configure` to personalize it.
source "${ZSH_CUSTOM:-$HOME/.oh-my-zsh/custom}/themes/powerlevel10k/config/p10k-classic.zsh"

# Follow Kitty's Matugen palette through ANSI colors rather than fixed RGB.
# Transparent segments inherit the terminal's background and opacity.
() {
  emulate -L zsh
  local name color
  for name in ${(k)parameters}; do
    [[ $name == POWERLEVEL9K_*FOREGROUND ||
       $name == POWERLEVEL9K_*VISUAL_IDENTIFIER_COLOR ]] || continue
    case $name in
      *ERROR*|*CRITICAL*|*CONFLICTED*) color=1 ;;
      *WARNING*|*MODIFIED*|*COMMAND_EXECUTION_TIME*) color=3 ;;
      *OK*|*CLEAN*|*BACKGROUND_JOBS*) color=10 ;;
      *) color=6 ;;
    esac
    typeset -g "$name=$color"
  done
}
typeset -g POWERLEVEL9K_BACKGROUND=
typeset -g POWERLEVEL9K_DIR_FOREGROUND=4
typeset -g POWERLEVEL9K_DIR_ANCHOR_FOREGROUND=12
typeset -g POWERLEVEL9K_DIR_SHORTENED_FOREGROUND=8
typeset -g POWERLEVEL9K_VCS_UNTRACKED_FOREGROUND=4
typeset -g POWERLEVEL9K_VCS_VISUAL_IDENTIFIER_COLOR=5
typeset -g POWERLEVEL9K_VCS_LOADING_VISUAL_IDENTIFIER_COLOR=8
typeset -g POWERLEVEL9K_CONTEXT_ROOT_FOREGROUND=1
typeset -g POWERLEVEL9K_CONTEXT_FOREGROUND=6
typeset -g POWERLEVEL9K_CONTEXT_{REMOTE,REMOTE_SUDO}_FOREGROUND=14
typeset -g POWERLEVEL9K_COMMAND_EXECUTION_TIME_FOREGROUND=13

# Show complete signal names and numbers, e.g. SIGINT(2).
typeset -g POWERLEVEL9K_STATUS_VERBOSE_SIGNAME=true

# Show non-zero exit statuses on the right; hide successful commands.
(( ${POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS[(Ie)status]} )) ||
  POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS=(status ${POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS[@]})
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE}=false
typeset -g POWERLEVEL9K_STATUS_{ERROR,ERROR_SIGNAL,ERROR_PIPE}=true
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE}_FOREGROUND=10
typeset -g POWERLEVEL9K_STATUS_{ERROR,ERROR_SIGNAL,ERROR_PIPE}_FOREGROUND=3
typeset -g POWERLEVEL9K_STATUS_SHOW_PIPESTATUS=false
# Show only the exit code and signal text, without check or cross icons.
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE,ERROR,ERROR_SIGNAL,ERROR_PIPE}_VISUAL_IDENTIFIER_EXPANSION=''
# Expand nice-exit-code labels while retaining the original numeric status.
function dotfiles_exit_formatter() {
  emulate -L zsh
  local code=$1 converted label description signal_text= signal_name signal_description
  local ZSH_PROMPT_EXIT_SIGNAL_PREFIX= ZSH_PROMPT_EXIT_SIGNAL_SUFFIX=
  if (( code == 0 )); then
    typeset -g dotfiles_exit_text=
    return 0
  elif (( $+functions[nice_exit_code] )); then
    converted=$(nice_exit_code "$code")
    label=${converted#*:}
    case $label in
      FATAL) description='致命错误' ;;
      WARN) description='执行失败' ;;
      BUILTINMISUSE) description='内建命令使用错误' ;;
      CCANNOTINVOKE) description='无法执行命令' ;;
      CNOTFOUND) description='命令不存在' ;;
      HUP) description='终端挂断' ;;
      INT) description='已中断' ;;
      QUIT) description='退出信号终止' ;;
      ILL) description='非法指令' ;;
      ABRT) description='异常终止' ;;
      FPE) description='算术异常' ;;
      KILL) description='已强制终止' ;;
      SEGV) description='段错误' ;;
      PIPE) description='管道断裂' ;;
      TERM) description='已终止' ;;
      # These raw exit codes are not proof of a stop signal.
      STOP) description='插件标注：停止' ;;
      TSTP) description='插件标注：终端暂停' ;;
      TTIN) description='插件标注：后台读取终端' ;;
      TTOU) description='插件标注：后台写入终端' ;;
      *) description='未定义的退出状态' ;;
    esac
  else
    description='退出说明不可用'
  fi
  # Use p10k's full signal name, including signals not mapped by the plugin.
  if (( code > 128 )) && [[ $P9K_CONTENT == SIG* ]]; then
    signal_name=${P9K_CONTENT%%\(*}
    case $signal_name in
      SIGHUP) signal_description='挂断信号' ;;
      SIGINT) signal_description='中断信号' ;;
      SIGQUIT) signal_description='退出信号' ;;
      SIGILL) signal_description='非法指令信号' ;;
      SIGTRAP) signal_description='跟踪陷阱信号' ;;
      SIGABRT|SIGIOT) signal_description='异常终止信号' ;;
      SIGBUS) signal_description='总线错误信号' ;;
      SIGFPE) signal_description='算术异常信号' ;;
      SIGKILL) signal_description='强制终止信号' ;;
      SIGUSR1) signal_description='用户自定义信号一' ;;
      SIGUSR2) signal_description='用户自定义信号二' ;;
      SIGSEGV) signal_description='无效内存访问信号' ;;
      SIGPIPE) signal_description='管道断裂信号' ;;
      SIGALRM) signal_description='定时器到期信号' ;;
      SIGTERM) signal_description='终止请求信号' ;;
      SIGCHLD|SIGCLD) signal_description='子进程状态变化信号' ;;
      SIGCONT) signal_description='继续运行信号' ;;
      SIGSTOP) signal_description='强制暂停信号' ;;
      SIGTSTP) signal_description='终端暂停信号' ;;
      SIGTTIN) signal_description='后台读取终端信号' ;;
      SIGTTOU) signal_description='后台写入终端信号' ;;
      SIGURG) signal_description='紧急数据信号' ;;
      SIGXCPU) signal_description='处理器时间超限信号' ;;
      SIGXFSZ) signal_description='文件大小超限信号' ;;
      SIGVTALRM) signal_description='虚拟定时器到期信号' ;;
      SIGPROF) signal_description='性能分析定时信号' ;;
      SIGWINCH) signal_description='窗口大小变化信号' ;;
      SIGIO|SIGPOLL) signal_description='输入输出就绪信号' ;;
      SIGPWR) signal_description='电源故障信号' ;;
      SIGSYS) signal_description='非法系统调用信号' ;;
      SIGRT*) signal_description='实时信号' ;;
      *) signal_description='其他信号' ;;
    esac
    signal_text=" %F{7}·%f %F{1}${signal_description} ${P9K_CONTENT}%f"
  fi
  typeset -g dotfiles_exit_text="${description}(${code})${signal_text}"
  return 0
}
functions -M dotfiles_exit_formatter 1 1
# Use p10k's saved status, since $? can change during prompt rendering.
typeset -g POWERLEVEL9K_STATUS_CONTENT_EXPANSION='${$((dotfiles_exit_formatter(_p9k__status)))+${dotfiles_exit_text}}'

# Color the input marker explicitly instead of using the unstyled line terminator.
(( ${POWERLEVEL9K_LEFT_PROMPT_ELEMENTS[(Ie)prompt_char]} )) ||
  POWERLEVEL9K_LEFT_PROMPT_ELEMENTS+=(prompt_char)
typeset -g POWERLEVEL9K_PROMPT_CHAR_OK_{VIINS,VICMD,VIVIS,VIOWR}_FOREGROUND=4
typeset -g POWERLEVEL9K_PROMPT_CHAR_ERROR_{VIINS,VICMD,VIVIS,VIOWR}_FOREGROUND=1
typeset -g POWERLEVEL9K_PROMPT_CHAR_{OK,ERROR}_VIINS_CONTENT_EXPANSION='❯'

# Keep the preset's Git formatting and counters, replacing its fixed colors.
functions[_p10k_system_git_formatter_base]=$functions[my_git_formatter]
function my_git_formatter() {
  emulate -L zsh
  _p10k_system_git_formatter_base "$@"
  my_git_format=${my_git_format//\%76F/\%5F}
  my_git_format=${my_git_format//\%178F/\%3F}
  my_git_format=${my_git_format//\%39F/\%4F}
  my_git_format=${my_git_format//\%196F/\%1F}
  my_git_format=${my_git_format//\%244F/\%8F}
}

(( ! $+functions[p10k] )) || p10k reload
