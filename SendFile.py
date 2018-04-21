#!/usr/bin/env python 

from subprocess import call
import time
from datetime import datetime
def main():
    while True:
        t = str(datetime.now())
        print t
        f = open('data.txt','w+') 
        f.write(t+'\n')
        f.close()

        call(['scp','data.txt', 'zhanwang:/home/zhanwang/Temp'])
        time.sleep(0.01)
if __name__ == '__main__':
    main()
