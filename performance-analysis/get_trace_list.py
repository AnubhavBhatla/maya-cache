def read_file(filename):
    try:
        with open(filename, 'r') as file:
            lines = file.readlines()
            return lines
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        return []

def main():
    filename = "trace_list.txt"
    lines = read_file(filename)
    print((lines))
    with open('trace_links.txt', 'w') as file:
        if lines:
            print("Lines in the file:")
            for line in lines:
                file.write("https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/" + line.strip() + '.champsimtrace.xz' + '\n')  # Strip newline characters before printing
if __name__ == "__main__":
    main()
