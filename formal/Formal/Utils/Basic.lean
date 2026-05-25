import Std
def hello := "world"

def isEven : Nat -> Bool := fun n => n % 2 == 0
#eval isEven 2

universe u 
def F (α : Type u) : Type u := Prod α α
#check (1 + 1)
def add (x y : Nat) : Nat := x + y

add 5 3
