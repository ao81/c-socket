### .tmux.conf
```bash
# デフォルトのプリフィックス(Ctrl+b)を解除
unbind C-b

# プリフィックスを Ctrl+a に設定
set -g prefix C-a

# Ctrl+a を2回連続で押すと、アプリケーション側に Ctrl+a を送る設定
bind C-a send-prefix

# マウス操作を有効にする
set -g mouse on
```
