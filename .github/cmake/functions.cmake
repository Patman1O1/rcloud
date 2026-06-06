#-----------------------------------------------------------------------------------------------------------------------
# to_pascal_case()
#
# Description: Converts a string to PascalCase.
#
# Parameters:
#   INPUT_STRING - The string that will be converted to PascalCase.
#   OUTPUT_STRING - The variable where the converted string will be stored.
#
#-----------------------------------------------------------------------------------------------------------------------
function(to_pascal_case INPUT_STRING RESULT_STRING)
    # Ignore empty strings
    if("${INPUT_STRING}" EQUAL "")
        set(${RESULT_STRING} "" PARENT_SCOPE)
        return()
    endif()

    # Replace common delimiters with spaces
    string(REGEX REPLACE "[-_.]" " " temp "${INPUT_STRING}")

    # Insert space before uppercase letters in camelCase (but not at start)
    # This regex finds lowercase followed by uppercase and inserts space between
    string(REGEX REPLACE "([a-z])([A-Z])" "\\1 \\2" temp "${temp}")

    # Insert space between consecutive uppercase and lowercase (for acronyms)
    # e.g., "XMLParser" -> "XML Parser"
    string(REGEX REPLACE "([A-Z]+)([A-Z][a-z])" "\\1 \\2" temp "${temp}")

    # Convert to lowercase first for consistent processing
    string(TOLOWER "${temp}" temp)

    # Split into words
    string(REGEX REPLACE " +" ";" word_list "${temp}")

    set(result "")
    foreach(word ${word_list})
        # Skip empty words
        if(word)
            # Capitalize first letter of each word
            string(SUBSTRING "${word}" 0 1 first_char)
            string(TOUPPER "${first_char}" first_char)

            string(LENGTH "${word}" word_len)
            if(word_len GREATER 1)
                string(SUBSTRING "${word}" 1 -1 rest_chars)
                set(word "${first_char}${rest_chars}")
            else()
                set(word "${first_char}")
            endif()

            string(APPEND result "${word}")
        endif()
    endforeach()

    set(${RESULT_STRING} "${result}" PARENT_SCOPE)
endfunction()

#-----------------------------------------------------------------------------------------------------------------------
# to_snake_case()
#
# Description: Converts a string to snake_case.
#
# Parameters:
#   INPUT_STRING - The string that will be converted to snake_case.
#   RESULT_STRING - The variable where the converted string will be stored.
#
#-----------------------------------------------------------------------------------------------------------------------
function(to_snake_case INPUT_STRING RESULT_STRING)
    # Ignore empty strings
    if("${INPUT_STRING}" EQUAL "")
        set(${RESULT_STRING} "" PARENT_SCOPE)
        return()
    endif()

    # Replace common delimiters with underscores
    string(REGEX REPLACE "[-. ]" "_" temp "${INPUT_STRING}")

    # Insert underscore before uppercase letters in camelCase/PascalCase
    # Match lowercase followed by uppercase: aB -> a_B
    string(REGEX REPLACE "([a-z])([A-Z])" "\\1_\\2" temp "${temp}")

    # Insert underscore between consecutive uppercase and lowercase (for acronyms)
    # Match uppercase+uppercase followed by lowercase: ABc -> AB_c
    string(REGEX REPLACE "([A-Z]+)([A-Z][a-z])" "\\1_\\2" temp "${temp}")

    # Insert underscore between digit and letter
    string(REGEX REPLACE "([0-9])([A-Za-z])" "\\1_\\2" temp "${temp}")
    string(REGEX REPLACE "([A-Za-z])([0-9])" "\\1_\\2" temp "${temp}")

    # Convert to lowercase
    string(TOLOWER "${temp}" temp)

    # Replace multiple consecutive underscores with single underscore
    string(REGEX REPLACE "_+" "_" temp "${temp}")

    # Remove leading/trailing underscores
    string(REGEX REPLACE "^_|_$" "" temp "${temp}")

    set(${RESULT_STRING} "${temp}" PARENT_SCOPE)
endfunction()

#-----------------------------------------------------------------------------------------------------------------------
# to_kebab_case()
#
# Description: Converts a string to kebab-case.
#
# Parameters:
#   INPUT_STRING - The string that will be converted to kebab-case.
#   RESULT_STRING - The variable where the converted string will be stored.
#
#-----------------------------------------------------------------------------------------------------------------------
function(to_kebab_case INPUT_STRING RESULT_STRING)
    # Ignore empty strings
    if("${INPUT_STRING}" EQUAL "")
        set(${RESULT_STRING} "" PARENT_SCOPE)
        return()
    endif()

    # Replace common delimiters with underscores
    string(REGEX REPLACE "[_. ]" "-" temp "${INPUT_STRING}")

    # Insert underscore before uppercase letters in camelCase/PascalCase
    # Match lowercase followed by uppercase: aB -> a-B
    string(REGEX REPLACE "([a-z])([A-Z])" "\\1-\\2" temp "${temp}")

    # Insert underscore between consecutive uppercase and lowercase (for acronyms)
    # Match uppercase+uppercase followed by lowercase: ABc -> AB-c
    string(REGEX REPLACE "([A-Z]+)([A-Z][a-z])" "\\1-\\2" temp "${temp}")

    # Insert underscore between digit and letter
    string(REGEX REPLACE "([0-9])([A-Za-z])" "\\1-\\2" temp "${temp}")
    string(REGEX REPLACE "([A-Za-z])([0-9])" "\\1-\\2" temp "${temp}")

    # Convert to lowercase
    string(TOLOWER "${temp}" temp)

    # Replace multiple consecutive underscores with single underscore
    string(REGEX REPLACE "-+" "-" temp "${temp}")

    # Remove leading/trailing underscores
    string(REGEX REPLACE "^-|-$" "" temp "${temp}")

    set(${RESULT_STRING} "${temp}" PARENT_SCOPE)
endfunction()

