# CS-332: Lab-1

## Purpose:
To read a given file and count the number of times given keywords appear in the file.

## How to Compile:
'gcc te.c -o te'

## How to Execute:
'./te <Path> [flags]'
    flags may be:
        1. -S (prints size of each file)
        2. -s <size> (prints only files greater than or equal to size)
        3. -f <string pattern> (prints only files whose file or directory name contains the substring)


## Author(s)
Michael Moran

## Credits:
1. Reference for members of stat structure: https://man7.org/linux/man-pages/man2/lstat.2.html
2. This program utilized the code in figure 4.22 of the textbook as the base program, and editted it to fit HW2 requirements. 
    (This code was also posted to canvas as figure4.22.pdf)
3. Reference for string functions https://en.cppreference.com/w/c/string/byte
4. 