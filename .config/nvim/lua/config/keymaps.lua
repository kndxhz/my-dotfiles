-- Keymaps are automatically loaded on the VeryLazy event
-- Default keymaps that are always set: https://github.com/LazyVim/LazyVim/blob/main/lua/lazyvim/config/keymaps.lua
-- Add any additional keymaps here

-- Explicit system clipboard shortcuts, also available through which-key.
vim.keymap.set({ "n", "x" }, "<leader>y", '"+y', { desc = "Copy to System Clipboard" })
vim.keymap.set("n", "<leader>Y", '"+yy', { desc = "Copy Line to System Clipboard" })
vim.keymap.set("n", "<leader>p", '"+p', { desc = "Paste from System Clipboard" })
vim.keymap.set("x", "<leader>p", '"+P', { desc = "Paste from System Clipboard" })
vim.keymap.set("x", "<C-c>", '"+y', { desc = "Copy to System Clipboard" })
vim.keymap.set("x", "<C-S-c>", '"+y', { desc = "Copy to System Clipboard" })

-- Insert register contents literally, without applying automatic indentation.
vim.keymap.set("i", "<C-v>", "<C-r><C-p>+", { desc = "Paste from System Clipboard" })
vim.keymap.set("i", "<S-Insert>", "<C-r><C-p>+", { desc = "Paste from System Clipboard" })

-- Visual P preserves the register, so repeated pastes keep the copied text.
vim.keymap.set("x", "p", "P", { desc = "Paste Without Replacing Clipboard" })
