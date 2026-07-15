#!/usr/bin/env python3
"""Convert CetDevelopmentKit docs from Markdown to HTML."""

import sys
import io
import re
import os
import markdown

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

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
        <button id="copy-all-btn" onclick="copyFullDocument()">📄 复制全文</button>
    </div>
    <div id="doc-content">
{content}
    </div>
    <div id="doc-footer">📄 文档转换 · 筆名 CetXiyuan</div>
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
        showToast('✅ 已复制代码');
    }});
}}

function copyFullDocument() {{
    const content = document.getElementById('doc-content');
    const text = content.innerText;
    navigator.clipboard.writeText(text).then(function() {{
        showToast('✅ 已复制全文');
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


def convert_md_to_html(md_text, title):
    """Convert markdown text to HTML content."""
    # Parse markdown to HTML
    md = markdown.Markdown(extensions=['extra', 'codehilite', 'fenced_code', 'tables'])
    html = md.convert(md_text)

    # Post-process: wrap code blocks with copy button
    # markdown library uses <pre><code>...</code></pre>
    # We need to wrap them in code-block-wrapper div with copy button

    def wrap_code_block(match):
        code_content = match.group(0)
        # Check if it's a mermaid diagram (no language class or just plaintext)
        if '<code>' in code_content and code_content.count('<code>') == 1:
            # Check if there's a language class on the code tag
            code_tag_match = re.search(r'<code(.*?)>', code_content)
            if code_tag_match:
                attrs = code_tag_match.group(1)
                # Replace code class with highlight.js compatible class
                if 'class="' in attrs:
                    new_attrs = re.sub(r'class="language-(\w+)"', r'class="language-\1"', attrs)
                    code_content = code_content.replace(attrs, new_attrs, 1)

        return f'<div class="code-block-wrapper"><button class="code-copy-btn" onclick="copyCode(this)" title="复制代码">📋 复制</button>{code_content}</div>'

    # Markdown's codehilite extension outputs <div class="codehilite"><pre>...
    # or sometimes <pre><code class="language-xxx">...
    # Let's just wrap all <pre> blocks

    html = re.sub(r'<pre><code.*?</code></pre>', wrap_code_block, html, flags=re.DOTALL)
    # Also handle codehilite div wrapping
    html = re.sub(r'<div class="codehilite"><pre>.*?</pre></div>', wrap_code_block, html, flags=re.DOTALL)

    return html


def main():
    docs_dir = os.path.dirname(os.path.abspath(__file__))
    html_dir = os.path.join(docs_dir, 'html')
    os.makedirs(html_dir, exist_ok=True)

    # Map of md files to their titles (extracted from first h1 or filename)
    md_files = [
        'CetDevelopmentKit_架构设计.md',
        'CetDevelopmentKit_API参考.md',
        'CetDevelopmentKit_构建与运行指南.md',
        'CetDevelopmentKit_维护者指南.md',
    ]

    for md_file in md_files:
        md_path = os.path.join(docs_dir, md_file)
        if not os.path.exists(md_path):
            print(f"SKIP: {md_file} not found")
            continue

        with open(md_path, 'r', encoding='utf-8') as f:
            md_text = f.read()

        # Extract title from first h1 heading
        title_match = re.search(r'^#\s+(.+)$', md_text, re.MULTILINE)
        if title_match:
            title = title_match.group(1).strip()
        else:
            title = os.path.splitext(md_file)[0]

        html_content = convert_md_to_html(md_text, title)
        html_full = HTML_TEMPLATE.format(title=title, content=html_content)

        html_file = os.path.splitext(md_file)[0] + '.html'
        html_path = os.path.join(html_dir, html_file)

        with open(html_path, 'w', encoding='utf-8') as f:
            f.write(html_full)

        print(f"OK: {md_file} -> {html_file}")

if __name__ == '__main__':
    main()
