import os

def scan_folder_for_strings(folder_path):
    found_strings = {}

    # Walk through the directory and subdirectories
    for dirpath, _, filenames in os.walk(folder_path):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            if file_path.endswith(".c"):  # Assuming you want to scan Python files
                with open(file_path, 'r', encoding='utf-8') as file:
                    content = file.readlines()

                # List to store found strings in this file
                file_strings = []

                # Search for the _(" pattern in each line
                for line in content:
                    start = 0
                    while start < len(line):
                        # Find the occurrence of _(" in the line
                        start = line.find('_("', start)
                        if start == -1:
                            break  # No more occurrences in this line
                        
                        # Find the closing ")
                        end = line.find('")', start + 3)  # search after _(
                        if end != -1:
                            # Extract the string between _(" and ")
                            extracted_string = line[start + 3:end]
                            file_strings.append(extracted_string)
                            print(extracted_string)
                            start = end + 2  # Move past the last found string
                        else:
                            # If no closing, break out of loop to avoid infinite loop
                            break

                if file_strings:
                    found_strings[file_path] = file_strings

    return found_strings

def scan_file_for_msg_strings(file_path):
    found_strings = []

    try:
        # Open the file and read it line by line
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.readlines()

        # Search for the .msg = "pattern" in each line
        for line in content:
            start = 0
            while start < len(line):
                # Find the occurrence of .msg =
                start = line.find('.msg = "', start)
                if start == -1:
                    break  # No more occurrences in this line

                # Find the closing "
                start_quote = start + len('.msg = "')
                end_quote = line.find('"', start_quote)
                if end_quote != -1:
                    # Extract the string between " and "
                    extracted_string = line[start_quote:end_quote]
                    found_strings.append(extracted_string)
                    start = end_quote + 1  # Move past the last found string
                else:
                    # If no closing ", break out of the loop
                    break
    except FileNotFoundError:
        print(f"The file '{file_path}' does not exist.")
    
    found_string_dict = {}
    found_string_dict[file_path] = found_strings
    return found_string_dict


# Function to display the results
def print_found_strings(found_strings):
    if not found_strings:
        print("No strings found matching the pattern.")
    else:
        for file_path, strings in found_strings.items():
            print(f"\nIn file: {file_path}")
            for s in strings:
                print(f"  Found string: {s}")

# Specify the folder path
folder_to_scan = os.getcwd()

# Scan the folder and print results
found_strings_in_files = scan_folder_for_strings(folder_to_scan)
print_found_strings(found_strings_in_files)

larousse = found_strings_in_files.values()

# Scan the file 'retro-go/localization.c' and print the results
found_strings = scan_file_for_msg_strings('components/retro-go/rg_localization.c')
print_found_strings(found_strings)

translated = found_strings.values()
for word in translated:
    print(word)

for file in larousse:
    for word in file:
        for translation in translated:
                if word not in translation:
                    print("missing translation", word)