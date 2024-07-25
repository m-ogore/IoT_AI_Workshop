#int number = o; this is what we do in C

'''
    in C
    void changeNumber(){
        number = 6;
        printf("%d", number);
    }

'''

def change_value(number, number2, number3):
    
    

    if number == 0:
        print("the number has to be greater than 0")
   
    number2 = number * 10

    
    #for i in range(1, 10, 2):
    for i in range(10):
        number += 1
        number2 -= 1
        number3 = i
        print(f'{number}, {number2}, {number3}')
    
number = 1
number2 = 10
number3 = 0
change_value(number, number2, number3)

    