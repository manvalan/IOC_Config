#!/usr/bin/env python3
"""
Convert Markdown files to PDF using WeasyPrint and Pandoc
"""

import subprocess
import os
from pathlib import Path

def markdown_to_pdf(md_file, pdf_file):
    """Convert markdown to PDF via HTML intermediate"""
    
    print(f"Converting {md_file} to PDF...")
    
    # Step 1: Convert markdown to HTML using pandoc
    html_file = md_file.replace('.md', '.html')
    
    cmd_html = [
        'pandoc',
        '-f', 'markdown',
        '-t', 'html',
        md_file,
        '-o', html_file,
        '--css=style.css',
        '--metadata', 'title="IOC_Config Documentation"',
        '--metadata', 'author="Michele Bigi"',
        '--self-contained'
    ]
    
    print(f"  Step 1: Generating HTML from Markdown...")
    result = subprocess.run(cmd_html, capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"Error converting to HTML: {result.stderr}")
        return False
    
    print(f"  ✓ HTML generated: {html_file}")
    
    # Step 2: Convert HTML to PDF using weasyprint
    try:
        from weasyprint import HTML
        
        print(f"  Step 2: Converting HTML to PDF...")
        HTML(html_file).write_pdf(pdf_file)
        print(f"  ✓ PDF generated: {pdf_file}")
        
        # Clean up HTML file
        os.remove(html_file)
        print(f"  ✓ Cleaned up temporary HTML file")
        
        return True
        
    except ImportError:
        print(f"Error: WeasyPrint not installed. Run: pip install weasyprint")
        return False
    except Exception as e:
        print(f"Error converting to PDF: {e}")
        return False


def main():
    """Convert all documentation files to PDF"""
    
    os.chdir('/Users/michelebigi/VisualStudioCode/GitHub/IOC_Config')
    
    documents = [
        ('REFERENCE_MANUAL.md', 'REFERENCE_MANUAL.pdf'),
        ('IMPLEMENTATION_GUIDE.md', 'IMPLEMENTATION_GUIDE.pdf'),
        ('ARCHITECTURE.md', 'ARCHITECTURE.pdf'),
    ]
    
    print("=" * 60)
    print("IOC_Config Documentation - PDF Generation")
    print("=" * 60)
    print()
    
    success_count = 0
    fail_count = 0
    
    for md_file, pdf_file in documents:
        if Path(md_file).exists():
            if markdown_to_pdf(md_file, pdf_file):
                success_count += 1
                print()
            else:
                fail_count += 1
                print()
        else:
            print(f"⚠ File not found: {md_file}")
            fail_count += 1
            print()
    
    print("=" * 60)
    print(f"Summary: {success_count} successful, {fail_count} failed")
    print("=" * 60)
    
    # List generated PDFs
    print("\nGenerated PDF files:")
    for md_file, pdf_file in documents:
        if Path(pdf_file).exists():
            size_mb = Path(pdf_file).stat().st_size / (1024 * 1024)
            print(f"  ✓ {pdf_file} ({size_mb:.2f} MB)")
        else:
            print(f"  ✗ {pdf_file} (NOT FOUND)")


if __name__ == '__main__':
    main()
