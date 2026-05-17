# Called by the build system after xgettext writes POT_FILE.new.
# Only replaces the existing pot file when the translatable strings have
# changed, ignoring the POT-Creation-Date header, so every build does not
# produce a version-control diff with only a date change.

if(EXISTS "${POT_FILE}")
    execute_process(
        COMMAND grep -v "POT-Creation-Date" "${POT_FILE}.new"
        OUTPUT_VARIABLE new_content
    )
    execute_process(
        COMMAND grep -v "POT-Creation-Date" "${POT_FILE}"
        OUTPUT_VARIABLE old_content
    )

    if(new_content STREQUAL old_content)
        file(REMOVE "${POT_FILE}.new")
    else()
        file(RENAME "${POT_FILE}.new" "${POT_FILE}")
    endif()
else()
    file(RENAME "${POT_FILE}.new" "${POT_FILE}")
endif()
