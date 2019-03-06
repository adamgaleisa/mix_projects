#!/usr/bin/python3
## Problem 1: Hash Iterator
### Problem description
#[***hash***](https://en.wikipedia.org/wiki/Hash_function) iterator. 
#This takes two inputs, **a salt** and **an integer X**, and generates a 
#**10-character string** over the course of a large number of iterations.
#how to run
# echo "machine-learning,4" >/tmp/input_file.txt 
#./hash_iterator.py </tmp/input_file.txt

import hashlib
import base64
import sys

result = list("----------")
hash_input = sys.stdin.readline().split(",")
h_index = int(hash_input[1]) # number of zeros to check for
start_str = "0" * h_index    # string of 0s to check for
for i in range(1,0x7fffffff):
    hash_result = hashlib.md5((hash_input[0] + str(i)).encode()).hexdigest()  #encode == byte array,hexdigest == stringfy ouput
    index = int(hash_result[h_index],16) # convert from base16 to integer
    if (hash_result.startswith(start_str) and index >= 0 and index <= 9 and result[index] == "-"):
        result[index] = hash_result[i % 32]
        if "-" not in result: break

print("".join(result))
