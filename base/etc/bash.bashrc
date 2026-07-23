# ThaiOS Bash Configuration

THAIOS_COLOR1='\[\e[38;5;196m\]'   # Red
THAIOS_COLOR2='\[\e[38;5;202m\]'   # Orange
THAIOS_COLOR3='\[\e[38;5;32m\]'    # Blue
THAIOS_RESET='\[\e[0m\]'

PS1="${THAIOS_COLOR1}┌[${THAIOS_RESET}\u${THAIOS_COLOR3}@${THAIOS_RESET}\h${THAIOS_COLOR1}]─[${THAIOS_RESET}\w${THAIOS_COLOR1}]\n└[${THAIOS_RESET}\t${THAIOS_COLOR1}]>${THAIOS_RESET} "

alias ls='ls --color=auto'
alias ll='ls -la'
alias la='ls -A'
alias grep='grep --color=auto'
alias thai-update='sudo thai-pkg update'
alias thai-install='sudo thai-pkg install'
alias thai-remove='sudo thai-pkg remove'
alias thai-search='thai-pkg search'
