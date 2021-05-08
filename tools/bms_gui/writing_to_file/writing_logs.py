

from datetime import datetime

def generate_file_name():

    now = datetime.now()
    dt_string = now.strftime("%d_%m_%Y-%H-%M-%S")
    return dt_string

def write_log(text, path_name = ""):

    if path_name=="":
        path_name = "logs/" + generate_file_name()

    text=generate_file_name()+" " + text #adding time stamp
    file = open(path_name, "a+")
    text = text.replace(" ",",")
    file.write(text)

    file.close()
if __name__ == '__main__':
    #debugging
    for i in range(100):
        write_log(str(i)+"\n")