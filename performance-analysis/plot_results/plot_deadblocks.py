import os

#path = input("Enter the directory name: ")
path = "deadblocks_OG_OFF_hawk_pref_2048_16"
phrase = "Deadblock"
print(phrase)

List = os.listdir(path)
#print(List)

def get_number_between_dash_and_B(line):
    # Extract the substring between "-" and "B"
    start_index = line.rfind('-') + 1
    end_index = line.rfind('B', )
    #print(line[start_index:end_index])
    return int(line[start_index:end_index])

sorted_data = sorted(List, key=get_number_between_dash_and_B)

def get_first_three_digits(line):
    # Extract the first 3 digits from the string
    return int(line[:3])

sorted_list = sorted(sorted_data, key=get_first_three_digits)

# print(sorted_list)
ignored_files = []

cwd = os.getcwd()
for file in sorted_list:
    os.chdir(cwd + "/" + path)
    line_numbers = []
    try:
        # print(file)
        if (file in ignored_files):
            print("")
        else:
            with open(file, 'r') as f:
                for line_number, line in enumerate(f, start=1):
                    # Check if the phrase is present in the line
                    if (phrase in line):
                        dead = line.split()[6]
                        print(dead)

    # Handle exceptions if the file doesn't exist or there's an error in reading it
    except FileNotFoundError:
        print("File not found.")
    except IOError:
        print("Error while reading the file.")
