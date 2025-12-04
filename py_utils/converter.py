import struct
import sys

def convert_file(input_filename, output_filename=None):
    """
    Convert file from "Z/X/Y STAT" text format to binary XYZSTAT format.
    
    Args:
        input_filename: Path to input text file
        output_filename: Path to output binary file (default: input_filename + '.bin')
    """
    if output_filename is None:
        output_filename = input_filename + '.bin'
    
    try:
        with open(input_filename, 'r') as infile, open(output_filename, 'wb') as outfile:
            for line_num, line in enumerate(infile, 1):
                line = line.strip()
                if not line:  # Skip empty lines
                    continue
                
                try:
                    parts = line.split()
                    if len(parts) != 2:
                        raise ValueError(f"Expected 2 parts, got {len(parts)}")
                    
                    zxy_parts = parts[0].split('/')
                    if len(zxy_parts) != 3:
                        raise ValueError(f"Expected Z/X/Y format, got {parts[0]}")
                    
                    z, x, y = map(int, zxy_parts)
                    stat = int(parts[1])
                    
                    outfile.write(struct.pack('<QQQQ', x, y, z, stat))
                    
                except (ValueError, IndexError) as e:
                    print(f"Warning: Line {line_num} malformed ('{line}'): {e}", 
                          file=sys.stderr)
                    continue
        
        print(f"Successfully converted {input_filename} to {output_filename}")
        
    except FileNotFoundError:
        print(f"Error: File '{input_filename}' not found", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python convert.py <input_file> [output_file]")
        print("Example: python convert.py data.txt data.bin")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    convert_file(input_file, output_file)