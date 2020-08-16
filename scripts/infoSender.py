import sys

def main():

    f = open('/home/daniil/Documents/unknowwm/out.txt', 'w')
    if (len(sys.argv) > 1):
        data = sys.argv[1]
        print(data)
        f.write(data)
    else:
        f.write("error")
        print("error")
    f.close()

main()