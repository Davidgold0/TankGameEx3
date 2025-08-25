#!/bin/bash

# Script to copy .so files from Algorithm and GameManager folders to their respective other folders

echo -e "\033[32mStarting to copy .so files...\033[0m"

# Copy .so files from Algorithm folder to Algorithms_other
if [ -d "Algorithm" ]; then
    algorithm_files=$(find Algorithm -name "*.so" 2>/dev/null)
    if [ -n "$algorithm_files" ]; then
        file_count=$(echo "$algorithm_files" | wc -l)
        echo -e "\033[33mFound $file_count .so file(s) in Algorithm folder\033[0m"
        
        # Create Algorithms_other directory if it doesn't exist
        if [ ! -d "Algorithms_other" ]; then
            mkdir -p Algorithms_other
            echo -e "\033[36mCreated Algorithms_other directory\033[0m"
        fi
        
        echo "$algorithm_files" | while read -r file; do
            filename=$(basename "$file")
            cp "$file" "Algorithms_other/$filename"
            echo -e "\033[32mCopied $filename to Algorithms_other/\033[0m"
        done
    else
        echo -e "\033[33mNo .so files found in Algorithm folder\033[0m"
    fi
else
    echo -e "\033[31mAlgorithm folder not found\033[0m"
fi

# Copy .so files from GameManager folder to GameManagers_other
if [ -d "GameManager" ]; then
    game_manager_files=$(find GameManager -name "*.so" 2>/dev/null)
    if [ -n "$game_manager_files" ]; then
        file_count=$(echo "$game_manager_files" | wc -l)
        echo -e "\033[33mFound $file_count .so file(s) in GameManager folder\033[0m"
        
        # Create GameManagers_other directory if it doesn't exist
        if [ ! -d "GameManagers_other" ]; then
            mkdir -p GameManagers_other
            echo -e "\033[36mCreated GameManagers_other directory\033[0m"
        fi
        
        echo "$game_manager_files" | while read -r file; do
            filename=$(basename "$file")
            cp "$file" "GameManagers_other/$filename"
            echo -e "\033[32mCopied $filename to GameManagers_other/\033[0m"
        done
    else
        echo -e "\033[33mNo .so files found in GameManager folder\033[0m"
    fi
else
    echo -e "\033[31mGameManager folder not found\033[0m"
fi

echo -e "\033[32mScript execution completed!\033[0m"
