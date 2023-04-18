import os
import re
import random
from time import sleep

def main():
    while True:
        content = []
        # Open main.c to edit its content
        with open("main.c", "r") as f:
            content = f.read()
            # Find "#define MAX_SIZE N", where N is an integer
            # Then, replace N with a random number
            n = random.randint(1, 10)
            content = re.sub(r"#define N \d+", "#define N " + str(n), content)
            m = random.randint(1, 20)
            content = re.sub(r"#define M \d+", "#define M " + str(m), content)
            p = random.randint(int(n*m*0.6), int(n*m*1.5))
            content = re.sub(r"#define P \d+", "#define P " + str(p), content)
            e = random.randint(120, 250)
            content = re.sub(r"#define E \d+", "#define E " + str(e), content)
            # Write the new content to main.c
        with open("main.c", "w") as f:
            f.write(content)
        # Compile the program
        os.system("gcc -w main.c -o main")
        print("N = " + str(n) + ", M = " + str(m) + ", P = " + str(p) + ", E = " + str(e))
        sleep(1)
        # Run the program
        os.system("./main")


if __name__ == "__main__":
    main()