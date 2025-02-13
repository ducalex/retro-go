import os

def scan_folder_for_strings(folder_path):
    # List to store found strings in this file
    file_strings = []

    # Walk through the directory and subdirectories
    for dirpath, _, filenames in os.walk(folder_path):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            if file_path.endswith(".c"):  # scanning all .c files
                with open(file_path, 'r', encoding='utf-8') as file:
                    content = file.readlines()

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
                            start = end + 2  # Move past the last found string
                        else:
                            # If no closing, break out of loop to avoid infinite loop
                            break

    return file_strings

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
                start = line.find('[RG_LANG_EN] = "', start)
                if start == -1:
                    break  # No more occurrences in this line

                # Find the closing "
                start_quote = start + len('[RG_LANG_EN] = "')
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
    
    return found_strings

# Scan the project's folders for strings to translate _("string")
found_strings_in_files = scan_folder_for_strings(os.getcwd())

# removing any dupicates :
found_strings_in_files = list(dict.fromkeys(found_strings_in_files))

# Scan the file 'retro-go/localization.c'
translated = scan_file_for_msg_strings('components/retro-go/translations.h')

file = open("missing_translation.txt", "w")
for string in found_strings_in_files:
    if string not in translated:
        print("missing translation", '"'+string+'"')
        file.write('{\n\t[RG_LANG_EN] = "'+string+'",\n\t[RG_LANG_FR] = \"\",\n},\n')
        
        # file output :
        #{
        #   [RG_LANG_EN]  = "missing string",
        #   [RG_LANG_FR]  = "",
        #},

file.close()