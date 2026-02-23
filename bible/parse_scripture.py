import zipfile
import re
import json

def parse_acts():
    epub_path = 'ignatius-bible-nt.epub'
    html_file = 'TheIgnatiusC-leNewTestament_split_075.html'
    
    with zipfile.ZipFile(epub_path, 'r') as zip_ref:
        with zip_ref.open(html_file) as f:
            content = f.read().decode('utf-8')

    # Clean HTML tags
    text_content = re.sub(r'<[^>]+>', ' ', content)
    # Clean up whitespace and non-ascii artifacts
    text_content = text_content.replace('\n', ' ').replace('\r', ' ')
    text_content = re.sub(r'\s+', ' ', text_content)
    
    # Simple regex for "VerseNumber Text"
    matches = re.finditer(r'(\d+)\s+([A-Z][^0-9]+)', text_content)
    
    bible_data = []
    for m in matches:
        v_num = int(m.group(1))
        v_text = m.group(2).strip()
        # Clean text for C string compatibility
        v_text = v_text.replace('"', "'")
        v_text = re.sub(r'[^\x00-\x7F]+', ' ', v_text) # Remove non-ascii
        
        if len(v_text) > 15:
            bible_data.append({
                "chapter": 1,
                "verse": v_num,
                "text": v_text[:150].strip()
            })
        
    with open('src/scripture.h', 'w') as h:
        h.write("#ifndef SCRIPTURE_H\n")
        h.write("#define SCRIPTURE_H\n\n")
        h.write("typedef struct { int chapter; int verse; const char* text; } Verse;\n\n")
        h.write("static const Verse actsDatabase[] = {\n")
        for v in bible_data:
            h.write(f'    {{ {v["chapter"]}, {v["verse"]}, "{v["text"]}" }},\n')
        h.write("    { 0, 0, \"\" } \n")
        h.write("};\n\n")
        h.write("#endif\n")

if __name__ == "__main__":
    parse_acts()
