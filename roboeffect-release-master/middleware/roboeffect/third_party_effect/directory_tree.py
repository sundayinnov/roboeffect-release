import pathlib
import sys
import subprocess
import platform
import os

def generate_tree_lines(path_obj, prefix="", ignore_list=None):
    """
    递归生成目录树结构的文本行列表
    """
    if ignore_list is None:
        ignore_list = []

    lines = []
    
    # 获取当前目录下所有文件和文件夹
    try:
        # 过滤掉在 ignore_list 中的文件
        # 使用 sorted 确保顺序一致
        items = sorted([
            p for p in path_obj.iterdir() 
            if p.name not in ignore_list
        ])
    except PermissionError:
        return [f"{prefix}    [权限被拒绝]"]

    count = len(items)
    
    for index, item in enumerate(items):
        is_last = (index == count - 1)
        connector = "└─ " if is_last else "├─ "
        
        # 构建当前行
        line = f"{prefix}{connector}{item.name}"
        lines.append(line)
        
        # 递归处理子目录
        if item.is_dir():
            extension = "    " if is_last else "│   "
            # 将子目录的结果追加到列表
            lines.extend(generate_tree_lines(item, prefix + extension, ignore_list))
            
    return lines

def copy_to_clipboard(text):
    """
    尝试将文本复制到剪贴板，兼容多种环境
    """
    # 方法 1: 尝试使用 tkinter (标准库，跨平台且对 Unicode 支持较好)
    try:
        import tkinter as tk
        r = tk.Tk()
        r.withdraw() # 隐藏主窗口
        r.clipboard_clear()
        r.clipboard_append(text)
        r.update() # 保持内容在剪贴板
        r.destroy()
        return True
    except (ImportError, Exception):
        pass

    # 方法 2: 尝试调用系统命令
    system_name = platform.system()
    try:
        if system_name == "Windows":
            # Windows clip 命令 (尝试 UTF-16LE 编码以支持中文)
            subprocess.run("clip", input=text.encode("utf-16"), check=True)
            return True
        elif system_name == "Darwin": 
            # macOS pbcopy
            subprocess.run("pbcopy", input=text.encode("utf-8"), check=True)
            return True
        elif system_name == "Linux":
            # Linux xclip (如果安装了的话)
            subprocess.run(["xclip", "-selection", "clipboard"], input=text.encode("utf-8"), check=True)
            return True
    except (FileNotFoundError, subprocess.SubprocessError):
        pass
        
    return False

def main():
    current_dir = pathlib.Path.cwd()
    
    # 获取脚本自身文件名，用于过滤，避免显示脚本自己
    try:
        script_name = pathlib.Path(__file__).name
    except NameError:
        script_name = "directory_tree_copy.py"
        
    # 定义忽略列表
    # 你可以在这里添加不想显示的文件或文件夹，比如 .git, __pycache__ 等
    ignore_list = [script_name, ".git", "__pycache__", ".DS_Store", ".idea", ".vscode"]

    print("正在生成目录结构...", end="", flush=True)

    # 1. 生成结构列表
    tree_lines = ["."] # 添加根节点
    tree_lines.extend(generate_tree_lines(current_dir, ignore_list=ignore_list))
    
    # 2. 合并为字符串
    final_text = "\n".join(tree_lines)
    
    # 3. 复制到剪贴板
    if copy_to_clipboard(final_text):
        print("\n\n✅ 成功！目录结构已复制到剪贴板。")
        print("你可以直接去目标位置按 Ctrl+V (或 Cmd+V) 粘贴了。")
    else:
        print("\n\n⚠️ 自动复制失败（未找到 tkinter 或系统剪贴板命令）。")
        print("请手动复制以下内容：\n")
        print(final_text)

if __name__ == "__main__":
    main()