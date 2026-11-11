return {
  {
    "saghen/blink.cmp",
    opts = {
      keymap = {
        preset = "enter",
        ["<Tab>"] = { "select_next", "snippet_forward", "fallback" },
        ["<S-Tab>"] = { "select_prev", "snippet_backward", "fallback" },
        ["<CR>"] = { "accept", "fallback" },
        ["<C-space>"] = { "show", "show_documentation", "hide_documentation" },
        ["<C-e>"] = { "cancel", "fallback" },
      },
      completion = {
        -- Enter remains a newline until a candidate is explicitly selected.
        list = { selection = { preselect = false, auto_insert = false } },
        ghost_text = { enabled = false },
      },
    },
  },
}
