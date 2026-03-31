#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import xml.etree.ElementTree as ET
import re
from collections import defaultdict

NAMESPACES = {
    'w': 'http://schemas.openxmlformats.org/wordprocessingml/2006/main',
    'w14': 'http://schemas.microsoft.com/office/word/2010/wordml',
    'w15': 'http://schemas.microsoft.com/office/word/2012/wordml',
    'mc': 'http://schemas.openxmlformats.org/markup-compatibility/2006',
    'wp': 'http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing',
    'wp14': 'http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing',
    'wpg': 'http://schemas.microsoft.com/office/word/2010/wordprocessingGroup',
    'wps': 'http://schemas.microsoft.com/office/word/2010/wordprocessingShape',
    'pic': 'http://schemas.openxmlformats.org/drawingml/2006/picture'
}

for prefix, uri in NAMESPACES.items():
    ET.register_namespace(prefix, uri)

def twips_to_pt(twips):
    return twips / 20.0

def extract_rpr_style(rpr_elem, font_info):
    """提取 run 的 CSS 样式，同时收集字体名称到 font_info 集合中"""
    styles = []
    if rpr_elem is None:
        return '', font_info

    # 粗体
    if rpr_elem.find('w:b', NAMESPACES) is not None:
        styles.append('font-weight: bold;')
    if rpr_elem.find('w:bCs', NAMESPACES) is not None:
        styles.append('font-weight: bold;')

    # 斜体
    if rpr_elem.find('w:i', NAMESPACES) is not None:
        styles.append('font-style: italic;')
    if rpr_elem.find('w:iCs', NAMESPACES) is not None:
        styles.append('font-style: italic;')

    # 下划线
    u_elem = rpr_elem.find('w:u', NAMESPACES)
    if u_elem is not None and u_elem.get('{{{}}}val'.format(NAMESPACES['w'])) == 'single':
        styles.append('text-decoration: underline;')

    # 删除线
    if rpr_elem.find('w:strike', NAMESPACES) is not None:
        styles.append('text-decoration: line-through;')

    # 颜色
    color_elem = rpr_elem.find('w:color', NAMESPACES)
    if color_elem is not None:
        color_val = color_elem.get('{{{}}}val'.format(NAMESPACES['w']))
        if color_val and re.match(r'^[0-9A-Fa-f]{6}$', color_val):
            styles.append(f'color: #{color_val};')

    # 字号 (半磅 -> 磅)
    sz_elem = rpr_elem.find('w:sz', NAMESPACES)
    if sz_elem is not None:
        sz_val = sz_elem.get('{{{}}}val'.format(NAMESPACES['w']))
        if sz_val:
            pt = int(sz_val) / 2
            styles.append(f'font-size: {pt}pt;')
    szCs_elem = rpr_elem.find('w:szCs', NAMESPACES)
    if szCs_elem is not None:
        sz_val = szCs_elem.get('{{{}}}val'.format(NAMESPACES['w']))
        if sz_val:
            pt = int(sz_val) / 2
            styles.append(f'font-size: {pt}pt;')

    # 字体 (西文 + 东亚)
    fonts = []
    rfonts = rpr_elem.find('w:rFonts', NAMESPACES)
    if rfonts is not None:
        ascii_font = rfonts.get('{{{}}}ascii'.format(NAMESPACES['w']))
        eastasia_font = rfonts.get('{{{}}}eastAsia'.format(NAMESPACES['w']))
        hAnsi_font = rfonts.get('{{{}}}hAnsi'.format(NAMESPACES['w']))
        cs_font = rfonts.get('{{{}}}cs'.format(NAMESPACES['w']))

        for font in (ascii_font, eastasia_font, hAnsi_font, cs_font):
            if font:
                font_info.add(font)
                fonts.append(f'"{font}"')

    if fonts:
        # 智能后备：Inconsolata 后跟 monospace；Noto Serif CJK 后跟 serif
        if any('Inconsolata' in f for f in fonts):
            fonts.append('monospace')
        if any('Noto Serif' in f for f in fonts):
            fonts.append('"Noto Serif SC", "Source Han Serif", "SimSun", serif')
        styles.append(f'font-family: {", ".join(fonts)};')

    return ' '.join(styles), font_info

def extract_ppr_style(ppr_elem):
    styles = []
    if ppr_elem is None:
        return ''
    jc_elem = ppr_elem.find('w:jc', NAMESPACES)
    if jc_elem is not None:
        val = jc_elem.get('{{{}}}val'.format(NAMESPACES['w']))
        if val == 'start':
            styles.append('text-align: left;')
        elif val == 'center':
            styles.append('text-align: center;')
        elif val == 'right':
            styles.append('text-align: right;')
        elif val == 'both':
            styles.append('text-align: justify;')
        else:
            styles.append('text-align: left;')
    spacing_elem = ppr_elem.find('w:spacing', NAMESPACES)
    if spacing_elem is not None:
        before = spacing_elem.get('{{{}}}before'.format(NAMESPACES['w']))
        after = spacing_elem.get('{{{}}}after'.format(NAMESPACES['w']))
        if before:
            styles.append(f'margin-top: {twips_to_pt(int(before))}pt;')
        if after:
            styles.append(f'margin-bottom: {twips_to_pt(int(after))}pt;')
    return ' '.join(styles)

def process_paragraph(p_elem, font_info):
    ppr_elem = p_elem.find('w:pPr', NAMESPACES)
    p_style = extract_ppr_style(ppr_elem)
    runs_html = []
    has_page_break = False
    for child in p_elem:
        if child.tag == '{{{}}}r'.format(NAMESPACES['w']):
            rpr_elem = child.find('w:rPr', NAMESPACES)
            r_style, font_info = extract_rpr_style(rpr_elem, font_info)
            text_elems = child.findall('w:t', NAMESPACES)
            for t_elem in text_elems:
                text = t_elem.text or ''
                if r_style:
                    runs_html.append(f'<span style="{r_style}">{text}</span>')
                else:
                    runs_html.append(text)
            br_elem = child.find('w:br', NAMESPACES)
            if br_elem is not None and br_elem.get('{{{}}}type'.format(NAMESPACES['w'])) == 'page':
                has_page_break = True
    if not runs_html and not has_page_break:
        return '<div class="paragraph" style="min-height: 1em;"></div>', False, font_info
    if has_page_break and not runs_html:
        return None, True, font_info
    inner_html = ''.join(runs_html)
    return f'<div class="paragraph" style="{p_style}">{inner_html}</div>', False, font_info

def build_pages(body_elem, font_info):
    pages = []
    current_page_content = []
    for child in body_elem:
        if child.tag == '{{{}}}p'.format(NAMESPACES['w']):
            para_html, has_page_break, font_info = process_paragraph(child, font_info)
            if has_page_break:
                if current_page_content:
                    pages.append(''.join(current_page_content))
                    current_page_content = []
            elif para_html is not None:
                current_page_content.append(para_html)
    if current_page_content:
        pages.append(''.join(current_page_content))
    if not pages:
        pages.append('')
    return pages, font_info

def get_section_props(body_elem):
    sectPr = body_elem.find('w:sectPr', NAMESPACES)
    if sectPr is None:
        return None, None
    pgSz = sectPr.find('w:pgSz', NAMESPACES)
    pgMar = sectPr.find('w:pgMar', NAMESPACES)
    width_pt = height_pt = margin_top_pt = margin_bottom_pt = margin_left_pt = margin_right_pt = None
    if pgSz is not None:
        w_twips = pgSz.get('{{{}}}w'.format(NAMESPACES['w']))
        h_twips = pgSz.get('{{{}}}h'.format(NAMESPACES['w']))
        if w_twips:
            width_pt = twips_to_pt(int(w_twips))
        if h_twips:
            height_pt = twips_to_pt(int(h_twips))
    if pgMar is not None:
        top = pgMar.get('{{{}}}top'.format(NAMESPACES['w']))
        bottom = pgMar.get('{{{}}}bottom'.format(NAMESPACES['w']))
        left = pgMar.get('{{{}}}left'.format(NAMESPACES['w']))
        right = pgMar.get('{{{}}}right'.format(NAMESPACES['w']))
        if top:
            margin_top_pt = twips_to_pt(int(top))
        if bottom:
            margin_bottom_pt = twips_to_pt(int(bottom))
        if left:
            margin_left_pt = twips_to_pt(int(left))
        if right:
            margin_right_pt = twips_to_pt(int(right))
    return (width_pt, height_pt), (margin_top_pt, margin_right_pt, margin_bottom_pt, margin_left_pt)

def generate_font_imports(font_info):
    """根据收集的字体名称生成 CSS @import 语句"""
    imports = []
    # 映射常见字体到 Google Fonts 的可用族
    font_map = {
        'Inconsolata': 'Inconsolata:wght@400;700',
        'Noto Serif CJK SC': 'Noto+Serif+SC:wght@400;700;900',
        'Noto Serif CJK SC Black': 'Noto+Serif+SC:wght@400;700;900',
    }
    added = set()
    for font in font_info:
        for key, gf_param in font_map.items():
            if key.lower() in font.lower():
                if key not in added:
                    imports.append(f"@import url('https://fonts.googleapis.com/css2?family={gf_param}&display=swap');")
                    added.add(key)
                break
    return '\n    '.join(imports)

def generate_html(xml_path, output_path='output.html'):
    tree = ET.parse(xml_path)
    root = tree.getroot()
    body = root.find('w:body', NAMESPACES)
    if body is None:
        raise ValueError("未找到 w:body 元素")

    font_info = set()
    pages, font_info = build_pages(body, font_info)

    page_size, page_margins = get_section_props(body)
    if page_size is None or page_size[0] is None:
        page_width_pt, page_height_pt = 595.0, 842.0
    else:
        page_width_pt, page_height_pt = page_size
    if page_margins is None or any(m is None for m in page_margins):
        margin_top_pt = margin_bottom_pt = margin_left_pt = margin_right_pt = 72.0
    else:
        margin_top_pt, margin_right_pt, margin_bottom_pt, margin_left_pt = page_margins

    font_imports = generate_font_imports(font_info)

    html_template = f"""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Word 文档渲染</title>
    <style>
        {font_imports}
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        body {{
            background-color: #e2e2e2;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 30px 20px;
            font-family: 'Segoe UI', 'Roboto', 'Noto Sans', sans-serif;
        }}
        .page {{
            background: white;
            width: {page_width_pt}pt;
            min-height: {page_height_pt}pt;
            margin: 20px auto;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
            padding: {margin_top_pt}pt {margin_right_pt}pt {margin_bottom_pt}pt {margin_left_pt}pt;
            page-break-after: always;
            break-inside: avoid;
        }}
        .paragraph {{
            margin: 0 0 0.5em 0;
            line-height: 1.4;
            word-wrap: break-word;
        }}
        span {{
            white-space: pre-wrap;
        }}
        @media print {{
            body {{
                background: none;
                padding: 0;
                margin: 0;
            }}
            .page {{
                margin: 0;
                box-shadow: none;
                page-break-after: always;
            }}
        }}
    </style>
</head>
<body>
"""
    for idx, page_content in enumerate(pages):
        html_template += f'<div class="page">\n{page_content}\n</div>\n'

    html_template += """</body>
</html>"""

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html_template)
    print(f"HTML 已生成: {output_path}")
    if font_info:
        print(f"检测到的字体: {', '.join(font_info)}")
        print("已自动添加 Google Fonts 导入，请确保网络连接可用。")

if __name__ == '__main__':
    import sys
    xml_file = sys.argv[1] if len(sys.argv) > 1 else 'document.xml'
    generate_html(xml_file)