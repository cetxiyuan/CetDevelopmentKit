#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""将4个独立 .md 文档分别转换为 HTML"""

import sys, io, re, os

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

DOCS_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(DOCS_DIR, 'html')
os.makedirs(OUTPUT_DIR, exist_ok=True)

TARGET_FILES = [
    'CetDevelopmentKit_架构设计.md',
    'CetDevelopmentKit_API参考.md',
    'CetDevelopmentKit_构建与运行指南.md',
    'CetDevelopmentKit_维护者指南.md',
]

def md_to_html(text):
    """轻量级 Markdown -> HTML 转换器"""
    lines = text.split('\n')
    out = []
    in_code_block = False
    code_lang = ''
    code_lines = []
    in_table = False
    table_lines = []
    in_paragraph = False
    para_lines = []
    in_list = False
    list_lines = []

    def flush_paragraph():
        nonlocal in_paragraph, para_lines
        if in_paragraph and para_lines:
            p = '<p>' + '\n'.join(para_lines) + '</p>'
            out.append(p)
            para_lines = []
        in_paragraph = False

    def flush_list():
        nonlocal in_list, list_lines
        if in_list and list_lines:
            out.append('<ul>')
            for li in list_lines:
                out.append(f'<li>{li}</li>')
            out.append('</ul>')
            list_lines = []
        in_list = False

    def flush_table():
        nonlocal in_table, table_lines
        if in_table and len(table_lines) >= 2:
            out.append('<table>')
            headers = table_lines[0]
            out.append('<thead><tr>')
            for h in headers:
                out.append(f'<th>{h.strip()}</th>')
            out.append('</tr></thead>')
            out.append('<tbody>')
            # Skip separator row (index 1) and process data rows
            body_start = 1
            # Check if row 1 is a separator (contains only |----| patterns)
            if len(table_lines) > 1:
                is_sep = all(re.match(r'^[\s\-:]+$', c.strip()) for c in table_lines[1])
                if is_sep:
                    body_start = 2
            for row in table_lines[body_start:]:
                out.append('<tr>')
                for cell in row:
                    out.append(f'<td>{cell.strip()}</td>')
                out.append('</tr>')
            out.append('</tbody></table>')
            table_lines = []
        in_table = False

    def process_inline(text):
        text = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', text)
        text = re.sub(r'`([^`]+)`', r'<code>\1</code>', text)
        text = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'<a href="\2">\1</a>', text)
        return text

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Code block
        if stripped.startswith('```'):
            flush_paragraph()
            flush_list()
            flush_table()
            if in_code_block:
                code_text = '\n'.join(code_lines)
                escaped = code_text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
                lang_class = f' class="language-{code_lang}"' if code_lang else ''
                copy_btn = '<button class="code-copy-btn" onclick="copyCode(this)" title="\u590d\u5236\u4ee3\u7801">\U0001f4cb \u590d\u5236</button>'
                out.append(f'<div class="code-block-wrapper">{copy_btn}<pre><code{lang_class}>{escaped}</code></pre></div>')
                code_lines = []
                code_lang = ''
                in_code_block = False
            else:
                code_lang = stripped[3:].strip()
                in_code_block = True
            i += 1
            continue

        if in_code_block:
            code_lines.append(line)
            i += 1
            continue

        # Horizontal rule
        if stripped == '---':
            flush_paragraph()
            flush_list()
            flush_table()
            out.append('<hr>')
            i += 1
            continue

        # Table detection - line starts and ends with |
        if stripped.startswith('|') and stripped.endswith('|'):
            flush_paragraph()
            flush_list()
            if not in_table:
                table_lines = []
                in_table = True
            cells = [c.strip() for c in stripped[1:-1].split('|')]
            table_lines.append(cells)
            i += 1
            continue

        if in_table:
            flush_table()

        # Heading
        heading_match = re.match(r'^(#{1,6})\s+(.+)', stripped)
        if heading_match:
            flush_paragraph()
            flush_list()
            level = len(heading_match.group(1))
            raw_text = heading_match.group(2)
            raw_text = raw_text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
            raw_text = re.sub(r'`([^`]+)`', r'<code>\1</code>', raw_text)
            out.append(f'<h{level}>{raw_text}</h{level}>')
            i += 1
            continue

        # Blockquote
        if stripped.startswith('>'):
            flush_paragraph()
            flush_list()
            content = process_inline(stripped[1:].strip())
            out.append(f'<blockquote>{content}</blockquote>')
            i += 1
            continue

        # List item
        list_match = re.match(r'^(\s*)[-*+]\s+(.+)', line)
        if list_match:
            flush_paragraph()
            if not in_list:
                list_lines = []
                in_list = True
            content = process_inline(list_match.group(2))
            list_lines.append(content)
            i += 1
            continue

        # Checkbox list item
        cb_match = re.match(r'^(\s*)[-*+]\s+\[([ xX])\]\s+(.+)', line)
        if cb_match:
            flush_paragraph()
            checked = 'checked' if cb_match.group(2).lower() == 'x' else ''
            if not in_list:
                list_lines = []
                in_list = True
            list_lines.append(f'<input type="checkbox" disabled {checked}> {process_inline(cb_match.group(3))}')
            i += 1
            continue

        if in_list and not stripped:
            flush_list()
            i += 1
            continue

        # Empty line
        if not stripped:
            flush_paragraph()
            i += 1
            continue

        # Paragraph text
        if not in_paragraph:
            in_paragraph = True
            para_lines = []
        text = process_inline(stripped)
        # Protect HTML tags with placeholders before escaping
        placeholders = {}
        counter = [0]
        def store_ph(m):
            key = f'\x00PH{counter[0]}\x00'
            placeholders[key] = m.group(0)
            counter[0] += 1
            return key
        text = re.sub(r'<(?:strong|/strong|code|/code|a\s[^>]*|/a|blockquote|/blockquote|input[^>]*|br\s*/?|hr\s*/?)>', store_ph, text)
        # Escape HTML
        text = text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
        # Restore protected HTML tags
        for key, value in placeholders.items():
            text = text.replace(key, value)
        para_lines.append(text)
        i += 1

    flush_paragraph()
    flush_list()
    flush_table()
    if in_code_block:
        code_text = '\n'.join(code_lines)
        escaped = code_text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')
        lang_class = f' class="language-{code_lang}"' if code_lang else ''
        copy_btn = '<button class="code-copy-btn" onclick="copyCode(this)" title="\u590d\u5236\u4ee3\u7801">\U0001f4cb \u590d\u5236</button>'
        out.append(f'<div class="code-block-wrapper">{copy_btn}<pre><code{lang_class}>{escaped}</code></pre></div>')

    return '\n'.join(out)


HTML_TEMPLATE = '''<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>{title}</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/atom-one-dark.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/bash.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/python.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/javascript.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/json.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/markdown.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/shell.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/ini.min.js"></script>
<style>
* {{
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}}

body {{
    background: #fafafa;
    color: #1e2a3e;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "Noto Sans SC", "PingFang SC", "Microsoft YaHei", sans-serif;
    line-height: 1.7;
    font-size: 15px;
}}

#doc-wrapper {{
    max-width: 1000px;
    margin: 0 auto;
    padding: 20px 24px 40px;
}}

#doc-header {{
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 16px 24px;
    background: #ffffff;
    border-radius: 8px;
    margin-bottom: 24px;
    box-shadow: 0 1px 3px rgba(0,0,0,0.08);
    position: sticky;
    top: 0;
    z-index: 100;
}}

#doc-header h1 {{
    font-size: 20px;
    color: #1e2a3e;
}}

#copy-all-btn {{
    background: #2563eb;
    color: #ffffff;
    border: none;
    padding: 8px 20px;
    border-radius: 6px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 500;
    transition: background 0.2s;
}}

#copy-all-btn:hover {{
    background: #1d4ed8;
}}

#doc-content {{
    background: #ffffff;
    border-radius: 8px;
    padding: 32px 40px;
    box-shadow: 0 1px 3px rgba(0,0,0,0.08);
}}

#doc-content h1 {{
    font-size: 28px;
    margin-top: 40px;
    margin-bottom: 16px;
    padding-bottom: 8px;
    border-bottom: 2px solid #e2e8f0;
    color: #1e2a3e;
}}

#doc-content h1:first-child {{
    margin-top: 0;
}}

#doc-content h2 {{
    font-size: 22px;
    margin-top: 32px;
    margin-bottom: 12px;
    color: #2d3748;
}}

#doc-content h3 {{
    font-size: 18px;
    margin-top: 24px;
    margin-bottom: 10px;
    color: #3b3f46;
}}

#doc-content h4 {{
    font-size: 16px;
    margin-top: 20px;
    margin-bottom: 8px;
    color: #4a5568;
}}

#doc-content p {{
    margin-bottom: 12px;
}}

#doc-content ul, #doc-content ol {{
    margin-bottom: 16px;
    padding-left: 24px;
}}

#doc-content li {{
    margin-bottom: 4px;
}}

#doc-content a {{
    color: #2563eb;
    text-decoration: none;
}}

#doc-content a:hover {{
    text-decoration: underline;
}}

/* Tables */
#doc-content table {{
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 20px;
    font-size: 14px;
}}

#doc-content th, #doc-content td {{
    border: 1px solid #e2e8f0;
    padding: 8px 12px;
    text-align: left;
}}

#doc-content th {{
    background: #f6f8fa;
    font-weight: 600;
    color: #2d3748;
}}

#doc-content tr:nth-child(even) td {{
    background: #fafbfc;
}}

/* Code blocks */
.code-block-wrapper {{
    position: relative;
    margin-bottom: 20px;
}}

.code-block-wrapper pre {{
    background: #282c34;
    border-radius: 8px;
    padding: 20px 16px 16px;
    overflow-x: auto;
    margin: 0;
}}

#doc-content pre code {{
    color: #abb2bf;
    font-family: "Cascadia Code", "Fira Code", "JetBrains Mono", "Consolas", "Monaco", monospace;
    font-size: 13.5px;
    line-height: 1.6;
    tab-size: 4;
    background: none;
    padding: 0;
}}

/* Inline code - must override hljs dark theme */
#doc-content code:not(pre code) {{
    background: #f0f4f8 !important;
    color: #c7254e !important;
    padding: 2px 6px;
    border-radius: 4px;
    font-family: "Cascadia Code", "Fira Code", "JetBrains Mono", "Consolas", "Monaco", monospace;
    font-size: 0.9em;
}}

/* Code copy button */
.code-copy-btn {{
    position: absolute;
    top: 8px;
    right: 12px;
    background: rgba(255,255,255,0.12);
    color: #abb2bf;
    border: 1px solid rgba(255,255,255,0.15);
    padding: 4px 10px;
    border-radius: 4px;
    cursor: pointer;
    font-size: 12px;
    transition: all 0.2s;
    z-index: 10;
}}

.code-copy-btn:hover {{
    background: rgba(255,255,255,0.22);
    color: #ffffff;
}}

/* Horizontal rule */
#doc-content hr {{
    border: none;
    border-top: 1px solid #e2e8f0;
    margin: 24px 0;
}}

/* Blockquote */
#doc-content blockquote {{
    border-left: 4px solid #2563eb;
    padding: 8px 16px;
    margin: 16px 0;
    background: #f0f4ff;
    border-radius: 0 4px 4px 0;
    color: #4a5568;
}}

/* Toast notification */
#toast {{
    position: fixed;
    bottom: 32px;
    left: 50%;
    transform: translateX(-50%);
    background: #1e2a3e;
    color: #ffffff;
    padding: 10px 24px;
    border-radius: 8px;
    font-size: 14px;
    opacity: 0;
    transition: opacity 0.3s;
    pointer-events: none;
    z-index: 999;
}}

#toast.show {{
    opacity: 1;
}}

/* Footer */
#doc-footer {{
    text-align: center;
    padding: 24px 0 8px;
    color: #94a3b8;
    font-size: 13px;
}}

#doc-content pre code.language-mermaid {{
    color: #98c379 !important;
}}

#doc-content td strong {{
    color: #1e2a3e;
}}

/* Mobile responsive */
@media (max-width: 768px) {{
    #doc-content {{
        padding: 20px 16px;
    }}
    #doc-header {{
        flex-direction: column;
        gap: 8px;
        text-align: center;
    }}
    #doc-content table {{
        font-size: 13px;
    }}
    #doc-content th, #doc-content td {{
        padding: 6px 8px;
    }}
}}
</style>
</head>
<body>
<div id="doc-wrapper">
    <div id="doc-header">
        <h1>{title}</h1>
        <button id="copy-all-btn" onclick="copyFullDocument()">\U0001f4c4 \u590d\u5236\u5168\u6587</button>
    </div>
    <div id="doc-content">
{body}
    </div>
    <div id="doc-footer">\U0001f4c4 \u6587\u6863\u8f6c\u6362 \u00b7 \u7b46\u540d CetXiyuan</div>
</div>
<div id="toast"></div>

<script>
document.addEventListener('DOMContentLoaded', function() {{
    document.querySelectorAll('#doc-content pre code').forEach(function(block) {{
        if (!block.classList.contains('language-mermaid')) {{
            hljs.highlightElement(block);
        }}
    }});
}});

function copyCode(btn) {{
    const codeBlock = btn.parentElement.querySelector('code');
    const text = codeBlock.textContent;
    navigator.clipboard.writeText(text).then(function() {{
        showToast('\u2705 \u5df2\u590d\u5236\u4ee3\u7801');
    }});
}}

function copyFullDocument() {{
    const content = document.getElementById('doc-content');
    const text = content.innerText;
    navigator.clipboard.writeText(text).then(function() {{
        showToast('\u2705 \u5df2\u590d\u5236\u5168\u6587');
    }});
}}

function showToast(message) {{
    const toast = document.getElementById('toast');
    toast.textContent = message;
    toast.classList.add('show');
    setTimeout(function() {{
        toast.classList.remove('show');
    }}, 2000);
}}
</script>
</body>
</html>'''


for md_filename in TARGET_FILES:
    md_path = os.path.join(DOCS_DIR, md_filename)
    if not os.path.exists(md_path):
        print(f'SKIP (not found): {md_path}')
        continue

    with open(md_path, 'r', encoding='utf-8') as f:
        md_content = f.read()

    # Extract title from first heading
    title_match = re.match(r'^#\s+(.+)', md_content)
    page_title = title_match.group(1).strip() if title_match else 'CetDevelopmentKit'

    # Convert
    html_body = md_to_html(md_content)
    full_html = HTML_TEMPLATE.format(title=page_title, body=html_body)

    # Output filename
    html_name = md_filename.replace('.md', '.html')
    output_path = os.path.join(OUTPUT_DIR, html_name)

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(full_html)

    print(f'OK: {html_name} ({len(full_html)} bytes)')

print('\nDone! All 4 HTML files generated.')
