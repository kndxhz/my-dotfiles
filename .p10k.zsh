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
typeset -g POWERLEVEL9K_DIR_ANCHOR_FOREGROUND=4
typeset -g POWERLEVEL9K_DIR_SHORTENED_FOREGROUND=8
typeset -g POWERLEVEL9K_VCS_UNTRACKED_FOREGROUND=4
typeset -g POWERLEVEL9K_VCS_VISUAL_IDENTIFIER_COLOR=10
typeset -g POWERLEVEL9K_VCS_LOADING_VISUAL_IDENTIFIER_COLOR=8
typeset -g POWERLEVEL9K_CONTEXT_ROOT_FOREGROUND=1
typeset -g POWERLEVEL9K_CONTEXT_FOREGROUND=4
typeset -g POWERLEVEL9K_CONTEXT_{REMOTE,REMOTE_SUDO}_FOREGROUND=4

# Show complete signal names and numbers, e.g. SIGINT(2).
typeset -g POWERLEVEL9K_STATUS_VERBOSE_SIGNAME=true

# Always show the final exit code on the right, including successful commands.
(( ${POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS[(Ie)status]} )) ||
  POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS=(status ${POWERLEVEL9K_RIGHT_PROMPT_ELEMENTS[@]})
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE,ERROR,ERROR_SIGNAL,ERROR_PIPE}=true
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE}_FOREGROUND=10
typeset -g POWERLEVEL9K_STATUS_{ERROR,ERROR_SIGNAL,ERROR_PIPE}_FOREGROUND=3
typeset -g POWERLEVEL9K_STATUS_SHOW_PIPESTATUS=false
# Show only the exit code and signal text, without check or cross icons.
typeset -g POWERLEVEL9K_STATUS_{OK,OK_PIPE,ERROR,ERROR_SIGNAL,ERROR_PIPE}_VISUAL_IDENTIFIER_EXPANSION=''
# P10k's saved status is used because $? can change while rendering the prompt.
# Keep signal descriptions when they differ from the numeric exit code.
typeset -g POWERLEVEL9K_STATUS_CONTENT_EXPANSION='${_p9k__status}${${P9K_CONTENT:#${_p9k__status}}:+ · ${P9K_CONTENT}}'

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
  my_git_format=${my_git_format//\%76F/\%10F}
  my_git_format=${my_git_format//\%178F/\%3F}
  my_git_format=${my_git_format//\%39F/\%4F}
  my_git_format=${my_git_format//\%196F/\%1F}
  my_git_format=${my_git_format//\%244F/\%8F}
}

(( ! $+functions[p10k] )) || p10k reload
