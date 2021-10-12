from random import randint

def parse_float_from_str(string):
    final=""
    for i in string:
        try:
            int(i)
            final+=i
        except:
            if i==".":
                final+=i

    try:
        return float(final)
    except:
        return 0


def parse_letter_in_string(string,func_to_use=float):
    """Returns an array of what the information will be for each of the 32 modules"""

    final = []

    string = string.split(" ")

    for i in range(len(string)-1):
        i+=1

        try:
            final.append(
                func_to_use(string[i])
            )
        except:
            pass

    return final


def generate_random_voltages(x="v: "):

    for i in range(32):
        x+=str(randint(0,4))+"."+str(randint(0,2))+str(randint(0,4))+" "
    return x

def signi_digs(number,significant_digits=3):
    number=str(number)


    twoparts = number.split(".")
    if len(twoparts)==1:
        return number+"."+"0"*significant_digits

    if len(twoparts[1])>significant_digits:
        return twoparts[0]+"."+twoparts[1][:significant_digits]

    return twoparts[0]+"." +twoparts[1] + (significant_digits-len(twoparts[1]))*"0"

if __name__ == '__main__':
    print(signi_digs(322.123,4))
