
import os
import time

f_badapple="C:\\Test\\BadApple\\1\\1080p.badapple"

def consloe_size(x:int,y:int):
    order='mode con cols=%d lines=%d'%(x,y)
    os.system(order)
    return

def rewind():
    print('\x1b[256F',end='')

def clear():
    rewind()
    print('\x1b[0J',end='')

s=open(f_badapple,'r').read().split('\n\n')
width,height,n,clk=s[0].split()
width=int(width)
height=int(height)
n=int(n)
clk=int(clk)

consloe_size(width,height)
clear()
print('BEGIN...')
time.sleep(1)

for i in s[1:]:
    rewind()
    print(i,end='')
    time.sleep(clk/1200)