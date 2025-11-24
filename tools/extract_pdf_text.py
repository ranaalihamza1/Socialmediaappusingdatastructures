import sys
from pypdf import PdfReader


def extract_text_from_pdf(path):
    reader = PdfReader(path)
    parts = []
    for page in reader.pages:
        # extract_text may return None for some pages
        t = page.extract_text()
        if t:
            parts.append(t)
    return "\n\n".join(parts)


def main():
    if len(sys.argv) < 2:
        print("Usage: python extract_pdf_text.py <pdf-path>", file=sys.stderr)
        sys.exit(2)
    pdf_path = sys.argv[1]
    try:
        txt = extract_text_from_pdf(pdf_path)
        if not txt.strip():
            print("[No extractable text found — PDF may be scanned images or protected]")
        else:
            print(txt)
    except Exception as e:
        print(f"ERROR extracting PDF: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
